/*
 * This file is part of ZeroVSH Patcher.

 * ZeroVSH Patcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * ZeroVSH Patcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZeroVSH Patcher. If not, see <http://www.gnu.org/licenses/ .
 */

//Headers
#include <pspsdk.h>
#include <psputilsforkernel.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>

// from CFW SDK
#include "psploadcore.h"
#include "systemctrl.h"
#include "systemctrl_se.h"

#include "hook.h"
#include "logger.h"
#include "blacklist.h"
#include "resolver.h"

PSP_MODULE_INFO("ZeroVSH_Patcher_Module", PSP_MODULE_KERNEL, 0, 1);
PSP_MAIN_THREAD_ATTR(0);

SceUID vshKernelLoadModuleVSH(const char *path, int flags, SceKernelLMOption *option);

STMOD_HANDLER previous = NULL;

int k1, model;

SceUID memid;
char *usermem;

char *g_blacklist_mod[] = {
		"sceHVNetfront_Module"
};

//OK
void *zeroCtrlAllocUserBuffer(int size) {
	memid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "pathBuf", PSP_SMEM_High, size, NULL);
	return (memid >= 0) ? sceKernelGetBlockHeadAddr(memid) : NULL;
}
//OK
void zeroCtrlFreeUserBuffer(void) {
	if(memid >= 0) {
		sceKernelFreePartitionMemory(memid);
	}
}
//OK
void zeroCtrlIcacheClearAll(void)
{
	__asm__ volatile("\
	.word 0x40088000; .word 0x24091000; .word 0x7D081240;\
	.word 0x01094804; .word 0x4080E000; .word 0x4080E800;\
	.word 0x00004021; .word 0xBD010000; .word 0xBD030000;\
	.word 0x25080040; .word 0x1509FFFC; .word 0x00000000;\
	"::);
}
//OK
void zeroCtrlDcacheWritebackAll(void)
{
	__asm__ volatile("\
	.word 0x40088000; .word 0x24090800; .word 0x7D081180;\
	.word 0x01094804; .word 0x00004021; .word 0xBD140000;\
	.word 0xBD140000; .word 0x25080040; .word 0x1509FFFC;\
	.word 0x00000000; .word 0x0000000F; .word 0x00000000;\
	"::);
}
//OK
void ClearCaches(void) {
	zeroCtrlIcacheClearAll();
	zeroCtrlDcacheWritebackAll();
}
//OK
char *zeroCtrlGetFileType(const char *file) {
    char *ret = NULL;

    if (!file) {
        zeroCtrlWriteDebug("--> Is NULL\n");
        return ret;
    }

    zeroCtrlWriteDebug("file: %s\n", file);
    ret = strrchr((char*) file, '.');

    if (!ret) {
        zeroCtrlWriteDebug("--> No Extension\n");
        return ret;
    }

    zeroCtrlWriteDebug("Success\n");

    return ret;
}
//OK
int zeroCtrlIoGetStat(const char *file, SceIoStat *stat, const char *ext) {
	int ret;

	usermem = zeroCtrlAllocUserBuffer(256);

    if(!usermem) {
    	zeroCtrlWriteDebug("Cannot allocate 256 bytes of memory, abort\n");
		pspSdkSetK1(k1);

    	return sceIoGetstat(file, stat);
    }

    if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp"))) {
        // Copy data from 21 onwards into string
		if(model == 4)	{
			sprintf(usermem, "ef0:/PSP/VSH/%s", file + 21); // flash0:/vsh/resource/	
		} else {
			sprintf(usermem, "ms0:/PSP/VSH/%s", file + 21); // flash0:/vsh/resource/	
		}
    }
	else if ((!strcmp(ext, ".pgf")) && (!zeroCtrlIsBlacklistedFound())) {
        // Copy data from 13 onwards into string
		if(model == 4)	{
			sprintf(usermem, "ef0:/PSP/VSH/%s", file + 13); // flash0:/font/
		} else {
			sprintf(usermem, "ms0:/PSP/VSH/%s", file + 13); // flash0:/font/
		}
    }

    zeroCtrlWriteDebug("new file: %s\n", usermem);
	pspSdkSetK1(k1);

	ret = sceIoGetstat(usermem, stat);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> found, using custom file\n\n");
    } else {
        zeroCtrlWriteDebug("--> not found, using default file\n\n");
        ret = sceIoGetstat(file, stat);
    }

    pspSdkSetK1(0);
	zeroCtrlFreeUserBuffer();
	pspSdkSetK1(k1);
    return ret;
}
//OK
int zeroCtrlIoOpen(const char *file, int flags, SceMode mode, const char *ext) {
	int ret;

	usermem = zeroCtrlAllocUserBuffer(256);

    if(!usermem) {
    	zeroCtrlWriteDebug("Cannot allocate 256 bytes of memory, abort\n");
		pspSdkSetK1(k1);

    	return sceIoOpen(file, flags, mode);
    }

    if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp"))) {
         // Copy data from 21 onwards into string
		if(model == 4)	{
			sprintf(usermem, "ef0:/PSP/VSH/%s", file + 21); // flash0:/vsh/resource/	
		} else {
			sprintf(usermem, "ms0:/PSP/VSH/%s", file + 21); // flash0:/vsh/resource/	
		}
    } 
	else if ((!strcmp(ext, ".pgf")) && (!zeroCtrlIsBlacklistedFound())) {
       // Copy data from 13 onwards into string
		if(model == 4)	{
			sprintf(usermem, "ef0:/PSP/VSH/%s", file + 13); // flash0:/font/
		} else {
			sprintf(usermem, "ms0:/PSP/VSH/%s", file + 13); // flash0:/font/
		}
    }

    zeroCtrlWriteDebug("new file: %s\n", usermem);
	pspSdkSetK1(k1);

	ret = sceIoOpen(usermem, flags, mode);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> found, using custom file\n\n");
    } else {
        zeroCtrlWriteDebug("--> not found, using default file\n\n");
        ret = sceIoOpen(file, flags, mode);
    }

    pspSdkSetK1(0);
	zeroCtrlFreeUserBuffer();
	pspSdkSetK1(k1);
    return ret;
}
//OK
int sceIoGetstat_patched(const char *file, SceIoStat *stat) {
	char *ext = NULL;

    k1 = pspSdkSetK1(0);
    zeroCtrlWriteDebug("file: %s, k1 value: 0x%08X\n", file, k1);

	if (strncmp(file, "flash0:/", 8) != 0) {
        zeroCtrlWriteDebug("file is not being accessed from flash0\n\n");
        pspSdkSetK1(k1);
        int ret = sceIoGetstat(file, stat);
		zeroCtrlWriteDebug("sceIoGetstat returned %08X\n", ret);
		return ret;
    }

	ext = zeroCtrlGetFileType(file);

    if (!ext) {
    	zeroCtrlWriteDebug("Error getting file extension\n\n");
    	pspSdkSetK1(k1);
    	return sceIoGetstat(file, stat);
    } else if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp")) | (!strcmp(ext, ".pgf"))) {
        return zeroCtrlIoGetStat(file, stat, ext);
    }

    zeroCtrlWriteDebug("file ext is not our type: %s\n\n", ext);
    pspSdkSetK1(k1);
    return sceIoGetstat(file, stat);
}
//OK
SceUID sceIoOpen_patched(const char *file, int flags, SceMode mode) {
    char *ext = NULL;

    k1 = pspSdkSetK1(0);
    zeroCtrlWriteDebug("file: %s, k1 value: 0x%08X\n", file, k1);

    if (strncmp(file, "flash0:/", 8) != 0) {
        zeroCtrlWriteDebug("file is not being accessed from flash0\n\n");
        pspSdkSetK1(k1);
        return sceIoOpen(file, flags, mode);
    }

    ext = zeroCtrlGetFileType(file);

    if (!ext) {
    	zeroCtrlWriteDebug("Error getting file extension\n\n");
    	pspSdkSetK1(k1);
    	return sceIoOpen(file, flags, mode);
    } else if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp")) | (!strcmp(ext, ".pgf"))) {
        return zeroCtrlIoOpen(file, flags, mode, ext);
    }

    zeroCtrlWriteDebug("file ext is not our type: %s\n\n", ext);
    pspSdkSetK1(k1);
    return sceIoOpen(file, flags, mode);
}

SceUID zeroCtrlLoadModuleVSH(const char *path, int flags, SceKernelLMOption *option, const char *ext) {
	int ret;

	usermem = zeroCtrlAllocUserBuffer(256);

    if(!usermem) {
    	zeroCtrlWriteDebug("Cannot allocate 256 bytes of memory, abort\n");
		pspSdkSetK1(k1);

    	return vshKernelLoadModuleVSH(path, flags, option);
    }

    if ((!strncmp(path, "flash0:/kd/", 11))) {
		 // Copy data from 11 onwards into string
		if(model == 4)	{
			sprintf(usermem, "ef0:/PSP/VSH/%s", path + 11); // flash0:/kd/
		} else {
			sprintf(usermem, "ms0:/PSP/VSH/%s", path + 11); // flash0:/kd/
		}
	}
	else if ((!strncmp(path, "flash0:/vsh/module/", 19))){
        // Copy data from 19 onwards into string
		if(model == 4)	{
			sprintf(usermem, "ef0:/PSP/VSH/%s", path + 19); // flash0:/vsh/module/
		} else {
			sprintf(usermem, "ms0:/PSP/VSH/%s", path + 19); // flash0:/vsh/module/
		}
    }

    zeroCtrlWriteDebug("new file: %s\n", usermem);
	pspSdkSetK1(k1);

	ret = vshKernelLoadModuleVSH(usermem, flags, option);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> found, using custom file\n\n");
    } else {
        zeroCtrlWriteDebug("--> not found, using default file\n\n");
        ret = vshKernelLoadModuleVSH(path, flags, option);
    }

    pspSdkSetK1(0);
	zeroCtrlFreeUserBuffer();
	pspSdkSetK1(k1);
    return ret;
}

SceUID vshKernelLoadModuleVSH_patched(const char *path, int flags, SceKernelLMOption *option) {
    char *ext = NULL;

    k1 = pspSdkSetK1(0);
    zeroCtrlWriteDebug("path: %s, k1 value: 0x%08X\n", path, k1);

    if (strncmp(path, "flash0:/", 8) != 0) {
        zeroCtrlWriteDebug("file is not being accessed from flash0\n\n");
        pspSdkSetK1(k1);
        return vshKernelLoadModuleVSH(path, flags, option);
    }

    ext = zeroCtrlGetFileType(path);

    if (!ext) {
    	zeroCtrlWriteDebug("Error getting file extension\n\n");
    	pspSdkSetK1(k1);
    	return vshKernelLoadModuleVSH(path, flags, option);
    } else if ((!strcmp(ext, ".prx"))) {
        return zeroCtrlLoadModuleVSH(path, flags, option, ext);
    }

    zeroCtrlWriteDebug("file ext is not our type: %s\n\n", ext);
    pspSdkSetK1(k1);
    return vshKernelLoadModuleVSH(path, flags, option);
}
//OK
void zeroCtrlPatchModule(SceModule2 *mod, int syscall, const char *mod_name) {
	const char *lib = syscall ? "IoFileMgrForUser" : "IoFileMgrForKernel";
	hook_import_bynid(mod, lib, 0x109F50BC, sceIoOpen_patched, syscall, mod_name);
	hook_import_bynid(mod, lib, 0xACE946E8, sceIoGetstat_patched, syscall, mod_name);
	if(syscall) {
		hook_import_bynid(mod, "sceVshBridge", 0xA5628F0D, vshKernelLoadModuleVSH_patched, syscall, NULL);
	}
    ClearCaches();
}
//OK
int OnModuleStart(SceModule2 *mod) {
    zeroCtrlWriteDebug("Relocating: %s\n", mod->modname);
    zeroCtrlWriteDebug("Version: %d.%d\n", mod->version[1], mod->version[0]);
    zeroCtrlWriteDebug("Attribute: 0x%04X\n", mod->attribute);

	zeroCtrlSetBlackListItems(mod->modname, g_blacklist_mod, 1);
    if((mod->text_addr & 0x80000000) == 0x80000000) {
    	zeroCtrlWriteDebug("Mode: kernel\n");
    	// only patch VSH control for now
    	if(strcmp(mod->modname, "VshCtrl") == 0 || strcmp(mod->modname, "VshControl") == 0) {
    		zeroCtrlPatchModule(mod, 0, NULL);
    	}
    } else {
    	zeroCtrlWriteDebug("Mode: user\n");
    	zeroCtrlPatchModule(mod, 1, "sceIOFileManager");
    }
    zeroCtrlWriteDebug("end\n\n");
    return previous ? previous(mod) : 0;
}

int module_start(SceSize args, void *argp) {
	zeroCtrlResolveNids();

    zeroCtrlWriteDebug("ZeroVSH Patcher v0.1\n");
    zeroCtrlWriteDebug("Copyright 2011 (C) NightStar3 and codestation\n");
    zeroCtrlWriteDebug("http://elitepspgamerz.forummotion.com\n\n");

    // patch the PRO/ME module if is already loaded before us
    SceModule2 *mod = sceKernelFindModuleByName("VshCtrl");
    if(mod == NULL) {
    	mod = sceKernelFindModuleByName("VshControl");
    }
    if(mod != NULL) {
        zeroCtrlPatchModule(mod, 0, NULL);
    }
 	
	model = sceKernelGetModel();
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    return 0;
}
//OK
int module_stop(SceSize args, void *argp) {
	return 0;
}

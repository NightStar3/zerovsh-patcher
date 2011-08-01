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
#include <pspsysevent.h>

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

SceUID sceIoOpen_user(const char *file, int flags, SceMode mode);
int sceIoGetstat_user(const char *file, SceIoStat *stat);

SceUID (*sceIoOpen_func)(const char *file, int flags, SceMode mode);
int (*sceIoGetstat_func)(const char *file, SceIoStat *stat);

STMOD_HANDLER previous = NULL;

int k1;

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

    	return sceIoGetstat_func(file, stat);
    }

    if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp"))) {
        // Copy data from 21 onwards into string
        sprintf(usermem, "ms0:/PSP/VSH/%s", file + 21); // flash0:/vsh/resource/
    }
	else if ((!strcmp(ext, ".pgf")) && (!zeroCtrlIsBlacklistedFound())) {
        // Copy data from 13 onwards into string
        sprintf(usermem, "ms0:/PSP/VSH/%s", file + 13); // flash0:/font/
    }

    zeroCtrlWriteDebug("new file: %s\n", usermem);
	pspSdkSetK1(k1);

	ret = sceIoGetstat_func(usermem, stat);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> found, using custom file\n\n");
    } else {
        zeroCtrlWriteDebug("--> not found, using default file\n\n");
        ret = sceIoGetstat_func(file, stat);
    }

	zeroCtrlFreeUserBuffer();
    return ret;
}
//OK
int zeroCtrlIoOpen(const char *file, int flags, SceMode mode, const char *ext) {
	int ret;

	usermem = zeroCtrlAllocUserBuffer(256);

    if(!usermem) {
    	zeroCtrlWriteDebug("Cannot allocate 256 bytes of memory, abort\n");
		pspSdkSetK1(k1);

    	return sceIoOpen_func(file, flags, mode);
    }

    if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp"))) {
        // Copy data from 21 onwards into string
        sprintf(usermem, "ms0:/PSP/VSH/%s", file + 21); // flash0:/vsh/resource/
    } 
	else if ((!strcmp(ext, ".pgf")) && (!zeroCtrlIsBlacklistedFound())) {
        // Copy data from 13 onwards into string
        sprintf(usermem, "ms0:/PSP/VSH/%s", file + 13); // flash0:/font/
    }

    zeroCtrlWriteDebug("new file: %s\n", usermem);
	pspSdkSetK1(k1);

	ret = sceIoOpen_func(usermem, flags, mode);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> found, using custom file\n\n");
    } else {
        zeroCtrlWriteDebug("--> not found, using default file\n\n");
        ret = sceIoOpen_func(file, flags, mode);
    }

	zeroCtrlFreeUserBuffer();
    return ret;
}
//OK
int sceIoGetstat_patched(const char *file, SceIoStat *stat) {
	char *ext = NULL;

    k1 = pspSdkSetK1(0);
    zeroCtrlWriteDebug("file: %s, k1 value: 0x%08X\n", file, k1);

	sceIoGetstat_func = !k1 ? sceIoGetstat : sceIoGetstat_user;

	if (strncmp(file, "flash0:/", 8) != 0) {
        zeroCtrlWriteDebug("file is not being accessed from flash0\n\n");
        pspSdkSetK1(k1);
        return sceIoGetstat_func(file, stat);
    }

	ext = zeroCtrlGetFileType(file);

    if (!ext) {
    	zeroCtrlWriteDebug("Error getting file extension\n\n");
    	pspSdkSetK1(k1);
    	return sceIoGetstat_func(file, stat);
    } else if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp")) | (!strcmp(ext, ".pgf"))) {
        return zeroCtrlIoGetStat(file, stat, ext);
    }

    zeroCtrlWriteDebug("file ext is not our type: %s\n\n", ext);
    pspSdkSetK1(k1);
    return sceIoGetstat_func(file, stat);
}
//OK
SceUID sceIoOpen_patched(const char *file, int flags, SceMode mode) {
    char *ext = NULL;

    k1 = pspSdkSetK1(0);
    zeroCtrlWriteDebug("file: %s, k1 value: 0x%08X\n", file, k1);

	sceIoOpen_func = !k1 ? sceIoOpen : sceIoOpen_user;

    if (strncmp(file, "flash0:/", 8) != 0) {
        zeroCtrlWriteDebug("file is not being accessed from flash0\n\n");
        pspSdkSetK1(k1);
        return sceIoOpen_func(file, flags, mode);
    }

    ext = zeroCtrlGetFileType(file);

    if (!ext) {
    	zeroCtrlWriteDebug("Error getting file extension\n\n");
    	pspSdkSetK1(k1);
    	return sceIoOpen_func(file, flags, mode);
    } else if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp")) | (!strcmp(ext, ".pgf"))) {
        return zeroCtrlIoOpen(file, flags, mode, ext);
    }

    zeroCtrlWriteDebug("file ext is not our type: %s\n\n", ext);
    pspSdkSetK1(k1);
    return sceIoOpen_func(file, flags, mode);
}
//OK
int OnModuleRelocated(SceModule2 *mod) {
    zeroCtrlWriteDebug("Relocating: %s\n", mod->modname);
    zeroCtrlWriteDebug("Version: %d.%d\n", mod->version[1], mod->version[0]);
    zeroCtrlWriteDebug("Attribute: 0x%04X\n\n", mod->attribute);

	zeroCtrlSetBlackListItems(mod->modname, g_blacklist_mod, 1);
	ClearCaches();

    if((mod->text_addr & 0x80000000) == 0x80000000) {
    	zeroCtrlWriteDebug("Mode: kernel\n\n");
    	hook_import_bynid(mod, "IoFileMgrForKernel", 0x109F50BC, sceIoOpen_patched, 0, 0);
		hook_import_bynid(mod, "IoFileMgrForKernel", 0xACE946E8, sceIoGetstat_patched, 0, 0);
    } else {
    	zeroCtrlWriteDebug("Mode: user\n\n");
    	if(hook_import_bynid(mod, "IoFileMgrForUser", 0x109F50BC, sceIoOpen_patched, 1, 1) == -2) {
			zeroCtrlWriteDebug("hook skipped\n");
		}
		if(hook_import_bynid(mod, "IoFileMgrForUser", 0xACE946E8, sceIoGetstat_patched, 1, 1) == -2) {
			zeroCtrlWriteDebug("hook skipped\n");
		}
    }
    return previous ? previous(mod) : 0;
}
//OK
int module_start(SceSize args, void *argp) {
	
	zeroCtrlResolveNids();

    zeroCtrlWriteDebug("ZeroVSH Patcher v0.1\n");
    zeroCtrlWriteDebug("Copyright 2011 (C) NightStar3 and codestation\n");
    zeroCtrlWriteDebug("http://elitepspgamerz.forummotion.com\n\n");
 	
    previous = sctrlHENSetStartModuleHandler(OnModuleRelocated);  
    return 0;
}
//OK
int module_stop(SceSize args, void *argp) {
	return 0;
}

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

#include "logger.h"
#include "blacklist.h"
#include "resolver.h"
#include "hook.h"

PSP_MODULE_INFO("ZeroVSH_Patcher_Module", PSP_MODULE_KERNEL, 0, 1);
PSP_MAIN_THREAD_ATTR(0);

#define UNUSED __attribute__((unused))

const char *g_blacklist_mod[] = {
		"sceHVNetfront_Module"
};

const char *exts[] = { ".rco", ".pmf", ".bmp", ".pgf", ".prx", ".dat" };

int k1, model;
SceUID memid;

PspIoDrv *lflash;
PspIoDrv *fatms;
static PspIoDrvArg * ms_drv = NULL;
STMOD_HANDLER previous = NULL;

int (*msIoOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
int (*msIoGetstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);

int (*IoOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
int (*IoGetstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);

//OK
void *zeroCtrlAllocUserBuffer(int size) {
	void *addr;
	k1 = pspSdkSetK1(0);
	memid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "pathBuf", PSP_SMEM_High, size, NULL);
	addr = (memid >= 0) ? sceKernelGetBlockHeadAddr(memid) : NULL;
	pspSdkSetK1(k1);
	return addr;
}
//OK
void zeroCtrlFreeUserBuffer(void) {
	if(memid >= 0) {
		k1 = pspSdkSetK1(0);
		sceKernelFreePartitionMemory(memid);
		pspSdkSetK1(k1);
	}
}
//OK
inline void zeroCtrlIcacheClearAll(void)
{
	__asm__ volatile("\
	.word 0x40088000; .word 0x24091000; .word 0x7D081240;\
	.word 0x01094804; .word 0x4080E000; .word 0x4080E800;\
	.word 0x00004021; .word 0xBD010000; .word 0xBD030000;\
	.word 0x25080040; .word 0x1509FFFC; .word 0x00000000;\
	"::);
}
//OK
inline void zeroCtrlDcacheWritebackAll(void)
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
    k1 = pspSdkSetK1(0);
    ret = strrchr((char*) file, '.');
    pspSdkSetK1(k1);

    if (!ret) {
        zeroCtrlWriteDebug("--> No Extension\n");
        return ret;
    }

    zeroCtrlWriteDebug("Success\n");

    return ret;
}
char *zeroCtrlSwapFile(const char *file, const char *ext) {
	char *ret = NULL;
	char *usermem = zeroCtrlAllocUserBuffer(256);

    if(!usermem) {
    	zeroCtrlWriteDebug("Cannot allocate 256 bytes of memory, abort\n");
    	return NULL;
    }

    k1 = pspSdkSetK1(0);

    if ((!strcmp(ext, ".rco")) | (!strcmp(ext, ".pmf")) | (!strcmp(ext, ".bmp"))) {
        // Copy data from 14 onwards into string
		sprintf(usermem, "/PSP/VSH/%s", file + 14); // /vsh/resource/
    }
	else if ((!strcmp(ext, ".pgf")) && (!zeroCtrlIsBlacklistedFound())) {
        // Copy data from 6 onwards into string
		sprintf(usermem, "/PSP/VSH/%s", file + 6); // /font/
	} else if (!strcmp(ext, ".dat")) {

	        zeroCtrlWriteDebug(".dat detected: %s\n", file);
	        if ((!strncmp(file, "/vsh/etc/", 9))) {
	            // Copy data from 4 onwards into string
	            sprintf(usermem, "/PSP/VSH/%s", file + 9); // /vsh/etc/
	            ret = usermem;
	            zeroCtrlWriteDebug("using %s as redirect\n", usermem);
	        } else if ((!strncmp(file, "/vsh/resource/", 14))) {
	            // Copy data from 12 onwards into string
	            sprintf(usermem, "/PSP/VSH/%s", file + 14); // /vsh/resource/
	            ret = usermem;
	            zeroCtrlWriteDebug("using %s as redirect\n", usermem);
	        }
    } else if ((!strcmp(ext, ".prx"))) {
    	if ((!strncmp(file, "/kd/", 4))) {
			// Copy data from 4 onwards into string
			sprintf(usermem, "/PSP/VSH/%s", file + 4); // /kd/
		} else if ((!strncmp(file, "/vsh/module/", 12))) {
			// Copy data from 12 onwards into string
			sprintf(usermem, "/PSP/VSH/%s", file + 12); // /vsh/module/
		}
    }
    pspSdkSetK1(k1);
    zeroCtrlWriteDebug("new file: %s\n", usermem);
	return usermem;
}
//OK
int zeroCtrlIoGetstatEX(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, const char *ext) {
	int ret;
	PspIoDrvArg *drv;
	char *new_path = zeroCtrlSwapFile(file, ext);

	if(!new_path) {
    	return IoGetstat(arg, file, stat);
    }
	drv = arg->drv;
	arg->drv = ms_drv;
	ret = fatms->funcs->IoGetstat(arg, new_path, stat);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> %s found, using custom file\n\n", new_path);
    } else {
        zeroCtrlWriteDebug("--> %s not found, using default file\n\n", new_path);
        arg->drv = drv;
        ret = IoGetstat(arg, file, stat);
    }

	zeroCtrlFreeUserBuffer();
    return ret;
}
//OK
int zeroCtrlIoOpenEX(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode, const char *ext) {
	int ret;
	PspIoDrvArg *drv;
	char *new_path = zeroCtrlSwapFile(file, ext);
	if(!new_path) {
    	return IoOpen(arg, file, flags, mode);
    }
	drv = arg->drv;
	arg->drv = ms_drv;
	ret = fatms->funcs->IoOpen(arg, new_path, flags, mode);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> %s found, using custom file\n\n", new_path);
    } else {
        zeroCtrlWriteDebug("--> %s not found, using default file\n\n", new_path);
        arg->drv = drv;
        ret = IoOpen(arg, file, flags, mode);
    }

	zeroCtrlFreeUserBuffer();
    return ret;
}

int zeroCtrlMsIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	// do not add logging in this function to avoid infinite recursion
	ms_drv = arg->drv;
	return msIoOpen(arg, file, flags, mode);
}

int zeroCtrlMsIoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
	// do not use sceIoGetstat in this function to avoid infinite recursion
	return msIoGetstat(arg, file, stat);
}

int zeroCtrlIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	char *ext = zeroCtrlGetFileType(file);
	if(ms_drv && ext != NULL) {
		for(u32 i = 0; i < ITEMSOF(exts); i++) {
			if(strcmp(ext, exts[i]) == 0) {
				return zeroCtrlIoOpenEX(arg, file, flags, mode, ext);
			}
		}
		zeroCtrlWriteDebug("unknown file extension: %s\n\n", ext);
	}
	return IoOpen(arg, file, flags, mode);
}

int zeroCtrlIoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
	char *ext = zeroCtrlGetFileType(file);
	if(ms_drv && ext != NULL) {
		for(u32 i = 0; i < ITEMSOF(exts); i++) {
			if(strcmp(ext, exts[i]) == 0) {
				return zeroCtrlIoGetstatEX(arg, file, stat, ext);
			}
		}
		zeroCtrlWriteDebug("unknown file extension: %s\n\n", ext);
	}
	return IoGetstat(arg, file, stat);
}
//OK
int zeroCtrlHookDriver(void) {
	int intr;
	SceUID fd;

	fatms = sctrlHENFindDriver( model == 4 ? "fatef" : "fatms");
	lflash = sctrlHENFindDriver("flashfat");

	if(!lflash || !fatms) {
		zeroCtrlWriteDebug("failed to hook drivers: lflash: %08X, fatms: %08X\n", (u32)lflash, (u32)fatms);
		return 0;
	}

	if(fatms) {
		msIoOpen = fatms->funcs->IoOpen;
		msIoGetstat = fatms->funcs->IoGetstat;
	}
	IoOpen = lflash->funcs->IoOpen;
	IoGetstat = lflash->funcs->IoGetstat;
	zeroCtrlWriteDebug("suspending interrupts\n");
	intr = sceKernelCpuSuspendIntr();

	if(fatms) {
		fatms->funcs->IoOpen = zeroCtrlMsIoOpen;
		fatms->funcs->IoGetstat = zeroCtrlMsIoGetstat;
	}

	lflash->funcs->IoOpen = zeroCtrlIoOpen;
	lflash->funcs->IoGetstat = zeroCtrlIoGetstat;

	sceKernelCpuResumeIntr(intr);
	ClearCaches();
	zeroCtrlWriteDebug("interrupts restored\n");
	fd = sceIoOpen( model == 4 ? "ef0:/_dummy.prx" : "ms0:/_dummy.prx", PSP_O_RDONLY, 0644 );
	if(fd >= 0) {
		// just in case that someone has a file like this
		sceIoClose(fd);
	}
	zeroCtrlWriteDebug("ms_drv addr: %08X\n", (u32)ms_drv);

	return 1;
}
//The 2nd arg is a SceLoadCoreExecFileInfo *, but we don't need it for now
int zeroCtrlModuleProbe(void *data, void *exec_info) {
	char filename[256];
	char prxname[32];
	SceSize size;

	char *modname = (char *)data + (((u32 *)data)[0x10] & 0x7FFFFFFF) + 4;

	zeroCtrlSetBlackListItems(modname, g_blacklist_mod, 1);

	if(strcmp(modname, "vsh_module") == 0) {
		strcpy(prxname, "vshmain.prx");
	} else if(strcmp(modname, "scePaf_Module") == 0) {
		strcpy(prxname, "paf.prx");
	} else if(strcmp(modname, "sceVshCommonGui_Module") == 0) {
		strcpy(prxname, "common_gui.prx");
	} else {
		*prxname = '\0';
	}

    if(*prxname) {
    	zeroCtrlWriteDebug("Probing: %s\n", prxname);
    	if (model == 4) {
    		sprintf(filename, "ef0:/PSP/VSH/%s", prxname);
    	} else {
    		sprintf(filename, "ms0:/PSP/VSH/%s", prxname);
    	}
    	SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0644);
    	if(fd >= 0) {
    		zeroCtrlWriteDebug("Writting buffer\n");
    		size = sceIoLseek(fd, 0, PSP_SEEK_END);
    		sceIoLseek(fd, 0, PSP_SEEK_SET);
    		sceIoRead(fd, data, size);
    		sceIoClose(fd);
    		ClearCaches();
    	} else {
    		zeroCtrlWriteDebug("%s not found, leaving buffer untouched\n", filename);
    	}
    }
    return sceKernelProbeExecutableObject(data, exec_info);
}
//OK
void zeroCtrlHookModule(void) {	
	SceModule2 *module = (SceModule2 *)sceKernelFindModuleByName("sceModuleManager");

	if(module) {
		if(hook_import_bynid(module, "LoadCoreForKernel", moduleprobe_nid, zeroCtrlModuleProbe, 0) < 0) {
			zeroCtrlWriteDebug("failed to hook function, nid: %08X\n", moduleprobe_nid);
		} else {
			zeroCtrlWriteDebug("hook success: ProbeExecutableObject nid: %08X, addr: %08X\n", moduleprobe_nid, (u32)sceKernelProbeExecutableObject);
		}
	}
}
//OK
int module_start(SceSize args UNUSED, void *argp UNUSED) {
	zeroCtrlInitDebug();
	zeroCtrlResolveNids();
	
	zeroCtrlWriteDebug("ZeroVSH Patcher v0.1\n");
	zeroCtrlWriteDebug("Copyright 2011 (C) NightStar3 and codestation\n");
	zeroCtrlWriteDebug("http://elitepspgamerz.forummotion.com\n\n");

	model = sceKernelGetModel();

	zeroCtrlHookModule();
	zeroCtrlHookDriver();

    return 0;
}
//OK
int module_stop(SceSize args UNUSED, void *argp UNUSED) {
	return 0;
}

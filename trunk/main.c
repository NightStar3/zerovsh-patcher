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
#include "minini/minIni.h"

PSP_MODULE_INFO("ZeroVSH_Patcher_Module", PSP_MODULE_KERNEL, 0, 2);
PSP_MAIN_THREAD_ATTR(0);

#define UNUSED __attribute__((unused))

typedef struct {
    const char *modname;
    const char *modfile;
} modules;

modules g_modules_mod[] = {
        { "vsh_module", "vshmain.prx" },
        { "scePaf_Module", "paf.prx" },
        { "sceVshCommonGui_Module", "common_gui.prx" },
};

const char *exts[] = { ".rco", ".pmf", ".bmp", ".pgf", ".prx", ".dat" };

int k1, model;
SceUID memid;

PspIoDrv *lflash;
PspIoDrv *fatms;
static PspIoDrvArg * ms_drv = NULL;
STMOD_HANDLER previous = NULL;

static char redir_path[128];

int (*msIoOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
int (*msIoGetstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);

int (*IoOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode);
int (*IoGetstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat);

//OK
void *zeroCtrlAllocUserBuffer(int size) {
    void *addr;
    k1 = pspSdkSetK1(0);
    memid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "pathBuf",
            PSP_SMEM_High, size, NULL);
    addr = (memid >= 0) ? sceKernelGetBlockHeadAddr(memid) : NULL;
    pspSdkSetK1(k1);
    return addr;
}
//OK
void zeroCtrlFreeUserBuffer(void) {
    if (memid >= 0) {
        k1 = pspSdkSetK1(0);
        sceKernelFreePartitionMemory(memid);
        pspSdkSetK1(k1);
    }
}
//OK
inline void zeroCtrlIcacheClearAll(void) {
    __asm__ volatile("\
	.word 0x40088000; .word 0x24091000; .word 0x7D081240;\
	.word 0x01094804; .word 0x4080E000; .word 0x4080E800;\
	.word 0x00004021; .word 0xBD010000; .word 0xBD030000;\
	.word 0x25080040; .word 0x1509FFFC; .word 0x00000000;\
	"::);
}
//OK
inline void zeroCtrlDcacheWritebackAll(void) {
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
int zeroCtrlIsValidFileType(const char *file) {
    int ret = 0;
    const char *ext;
    if (!file) {
        zeroCtrlWriteDebug("--> Is NULL\n");
        return 0;
    }

    zeroCtrlWriteDebug("file: %s\n", file);
    k1 = pspSdkSetK1(0);
    ext = strrchr(file, '.');
    if (!ext) {
        zeroCtrlWriteDebug("--> No Extension\n");
    } else {
        for (int i = 0; i < ITEMSOF(exts); i++) {
            if (strcmp(ext, exts[i]) == 0) {
                zeroCtrlWriteDebug("Success\n");
                ret = 1;
                break;
            }
        }
    }
    pspSdkSetK1(k1);
    return ret;
}
//OK
const char *zeroCtrlGetFileName(const char *file) {
    char *ret = NULL;

    if (!file) {
        zeroCtrlWriteDebug("--> Is NULL\n");
        return ret;
    }

    zeroCtrlWriteDebug("file: %s\n", file);
    k1 = pspSdkSetK1(0);
    ret = strrchr(file, '/');
    pspSdkSetK1(k1);

    // blacklist this one
    if (strcmp(file, "/codepage/cptbl.dat") == 0) {
        ret = NULL;
    }

    if (!ret) {
        zeroCtrlWriteDebug("--> No path\n");
    } else {
        zeroCtrlWriteDebug("Success\n");
    }
    return ret;
}
//OK
char *zeroCtrlSwapFile(const char *file) {
    const char *oldfile;
    char *newfile = zeroCtrlAllocUserBuffer(256);

    if (!newfile) {
        zeroCtrlWriteDebug("Cannot allocate 256 bytes of memory, abort\n");
        return NULL;
    }

    k1 = pspSdkSetK1(0);

    *newfile = '\0';
    oldfile = zeroCtrlGetFileName(file);
    if (!oldfile) {
        zeroCtrlWriteDebug("-> File not found, abort\n\n");
        pspSdkSetK1(k1);
        return NULL;
    }

    if (zeroCtrlIsBlacklistedFound()) {
        if (strcmp(oldfile, "/ltn0.pgf") == 0) {
            zeroCtrlWriteDebug("-> File is blacklisted, abort\n\n");
            pspSdkSetK1(k1);
            return NULL;
        }
    }

    sprintf(newfile, "%s%s", redir_path, oldfile);
    pspSdkSetK1(k1);

    zeroCtrlWriteDebug("-> Redirected file: %s\n", newfile);
    return newfile;
}
//OK
int zeroCtrlIoGetstatEX(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
    int ret;
    char *new_path;
    PspIoDrvArg *drv;

    new_path = zeroCtrlSwapFile(file);
    if (!new_path) {
        return IoGetstat(arg, file, stat);
    }

    drv = arg->drv;
    arg->drv = ms_drv;
    ret = msIoGetstat(arg, new_path, stat);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> %s found, using custom file\n\n", new_path);
    } else {
        zeroCtrlWriteDebug("--> %s not found, using default file\n\n", file);
        arg->drv = drv;
        ret = IoGetstat(arg, file, stat);
    }

    zeroCtrlFreeUserBuffer();
    return ret;
}
//OK
int zeroCtrlIoOpenEX(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
    int ret;
    char *new_path;
    PspIoDrvArg *drv;

    if ((new_path = zeroCtrlSwapFile(file)) == NULL) {
        return IoOpen(arg, file, flags, mode);
    }

    drv = arg->drv;
    arg->drv = ms_drv;
    ret = msIoOpen(arg, new_path, flags, mode);

    if (ret >= 0) {
        zeroCtrlWriteDebug("--> %s found, using custom file\n\n", new_path);
    } else {
        zeroCtrlWriteDebug("--> %s not found, using default file\n\n", file);
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
    if (ms_drv && zeroCtrlIsValidFileType(file)) {
        return zeroCtrlIoOpenEX(arg, file, flags, mode);
    } else {
        zeroCtrlWriteDebug("cannot redirect file: %s\n\n", file);
    }
    return IoOpen(arg, file, flags, mode);
}

int zeroCtrlIoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
    if (ms_drv && zeroCtrlIsValidFileType(file)) {
        return zeroCtrlIoGetstatEX(arg, file, stat);
    } else {
        zeroCtrlWriteDebug("cannot redirect file: %s\n\n", file);
    }
    return IoGetstat(arg, file, stat);
}
//OK
int zeroCtrlHookDriver(void) {
    int intr;
    SceUID fd;

    fatms = sctrlHENFindDriver(model == 4 ? "fatef" : "fatms");
    lflash = sctrlHENFindDriver("flashfat");

    if (!lflash || !fatms) {
        zeroCtrlWriteDebug("failed to hook drivers: lflash: %08X, fatms: %08X\n", (u32)lflash, (u32)fatms);
        return 0;
    }

    msIoOpen = fatms->funcs->IoOpen;
    msIoGetstat = fatms->funcs->IoGetstat;

    IoOpen = lflash->funcs->IoOpen;
    IoGetstat = lflash->funcs->IoGetstat;

    zeroCtrlWriteDebug("suspending interrupts\n");
    intr = sceKernelCpuSuspendIntr();

    fatms->funcs->IoOpen = zeroCtrlMsIoOpen;
    fatms->funcs->IoGetstat = zeroCtrlMsIoGetstat;

    lflash->funcs->IoOpen = zeroCtrlIoOpen;
    lflash->funcs->IoGetstat = zeroCtrlIoGetstat;

    sceKernelCpuResumeIntr(intr);
    ClearCaches();
    zeroCtrlWriteDebug("interrupts restored\n");

    fd = sceIoOpen(model == 4 ? "ef0:/_dummy.prx" : "ms0:/_dummy.prx", PSP_O_RDONLY, 0644);

    // just in case that someone has a file like this
    if (fd >= 0) {
        sceIoClose(fd);
    }

    zeroCtrlWriteDebug("ms_drv addr: %08X\n", (u32)ms_drv);

    return 1;
}
//The 2nd arg is a SceLoadCoreExecFileInfo *, but we don't need it for now
int zeroCtrlModuleProbe(void *data, void *exec_info) {
    char filename[256];
    SceSize size;
    SceUID fd;

    char *modname = (char *) data + (((u32 *) data)[0x10] & 0x7FFFFFFF) + 4;

    zeroCtrlSetBlackListItems(modname);

    for (int i = 0; i < ITEMSOF(g_modules_mod); i++) {
        if (strcmp(modname, g_modules_mod[i].modname) == 0) {
            zeroCtrlWriteDebug("probing: %s\n", g_modules_mod[i].modfile);
            sprintf(filename, "%s%s/%s", model == 4 ? "ef0:" : "ms0:", redir_path, g_modules_mod[i].modfile);
            fd = sceIoOpen(filename, PSP_O_RDONLY, 0644);
            if (fd >= 0) {
                zeroCtrlWriteDebug("writting %s into buffer\n", filename);
                size = sceIoLseek(fd, 0, PSP_SEEK_END);
                sceIoLseek(fd, 0, PSP_SEEK_SET);
                sceIoRead(fd, data, size);
                sceIoClose(fd);
                ClearCaches();
            } else {
                zeroCtrlWriteDebug("%s not found, leaving buffer untouched\n", filename);
            }
            break;
        }
    }
    return sceKernelProbeExecutableObject(data, exec_info);
}
//OK
void zeroCtrlHookModule(void) {
    SceModule2 *module = (SceModule2 *) sceKernelFindModuleByName("sceModuleManager");

    if (!module || hook_import_bynid(module, "LoadCoreForKernel", moduleprobe_nid, zeroCtrlModuleProbe, 0) < 0) {
        zeroCtrlWriteDebug("failed to hook ProbeExecutableObject, nid: %08X\n", moduleprobe_nid);
    } else {
        zeroCtrlWriteDebug("ProbeExecutableObject nid: %08X, addr: %08X\n", moduleprobe_nid, (u32)sceKernelProbeExecutableObject);
    }
}
//OK
int module_start(SceSize args UNUSED, void *argp UNUSED) {

    model = sceKernelGetModel();
    zeroCtrlInitDebug(model, 1, 0);

    zeroCtrlWriteDebug("ZeroVSH Patcher v0.2\n");
    zeroCtrlWriteDebug("Copyright 2011 (C) NightStar3 and codestation\n");
    zeroCtrlWriteDebug("http://elitepspgamerz.forummotion.com\n\n");

    zeroCtrlResolveNids();

    const char *config = (model == 4) ? "ef0:/seplugins/zerovsh.ini" : "ms0:/seplugins/zerovsh.ini";

    ini_gets("General", "RedirPath", "/PSP/VSH", redir_path, sizeof(redir_path), config);
    zeroCtrlWriteDebug("using [%s] as RedirPath\n", redir_path);

    zeroCtrlHookModule();
    zeroCtrlHookDriver();

    return 0;
}
//OK
int module_stop(SceSize args UNUSED, void *argp UNUSED) {
    return 0;
}

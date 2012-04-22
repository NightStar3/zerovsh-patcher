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
#include <pspiofilemgr_fcntl.h>
#include <pspsysmem_kernel.h>
#include <pspctrl.h>
#include <psprtc.h>
#include <pspdisplay.h>

#include <string.h>
#include <stdio.h>

// from CFW SDK
#include "../kernel/psploadcore.h"
#include "../kernel/systemctrl.h"
#include "../kernel/systemctrl_se.h"

PSP_MODULE_INFO("ZeroVSH_Patcher_User", 0x0007, 0, 1);

#define UNUSED __attribute__((unused))
#define MAKE_CALL(a, f) _sw(0x0C000000 | (((u32)(f) >> 2) & 0x03FFFFFF), a); 
#define REDIRECT_FUNCTION(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a); _sw(0x00000000, a+4); 
#define MAKE_JUMP(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a);

int devkit;

enum zeroCtrlSlideState {
        ZERO_SLIDE_LOADING = 1,
        ZERO_SLIDE_STARTING,
        ZERO_SLIDE_STARTED,
        ZERO_SLIDE_STOPPING,
	ZERO_SLIDE_STOPPED,
	ZERO_SLIDE_UNLOADED,
};


typedef struct
{
        void *unk; //0
        int id; //4
        char *regkey; //8
        char *text; //C
        char *subtitle; //10
        char *page; //14
} SceSysconfItem; //18

STMOD_HANDLER previous = NULL;
int level = -1;

void (* AddSysconfItem)(u32 *option, SceSysconfItem **item) = NULL;
void add_sysconf_item_stub();

void slide_check_stub();

int (* vshCtrlReadBufferPositive)(SceCtrlData *pad, int count) = NULL;
void vsh_ctrl_stub();

int zeroCtrlGetSlideState(void);
void zeroCtrlSetSlideState(int state);

int SetSpeed(int cpufreq, int busfreq);

//OK
void *zeroCtrlRedir2Stub(u32 address, void *stub, void *func) {
	_sw(_lw(address), (u32)stub);
	_sw(_lw(address + 4), (u32)stub + 4);
    
	MAKE_JUMP((u32)stub + 8, address + 8);
	REDIRECT_FUNCTION(address, func);
    
	return stub;
}
//OK
void zeroCtrlAddSysconfItem(u32 *option, SceSysconfItem **item) {
	if(strcmp((* item)->page, "page_psp_config_slide_action") == 0) {
		return;
	}
	
	AddSysconfItem(option, item);
}
//OK
int zeroCtrlDummyFunc(void) {
	return -1;
}
//OK
int zeroCtrlDummyFunc2(void) {
	return 0;
}
//OK
int zeroCtrlReadBufferPositive(SceCtrlData *pad, int count) {
	int ret, i, j;
	int k1 = pspSdkSetK1(0);	
		
	ret = vshCtrlReadBufferPositive(pad, count);
	
        if(zeroCtrlGetSlideState() == ZERO_SLIDE_STOPPED) {
		if(pad->Buttons & PSP_CTRL_HOME) {     
			//Fixes 'jump' bug with clock
			SetSpeed(266, 133);
			
                        zeroCtrlSetSlideState(ZERO_SLIDE_STARTING);                     
                }
        } else if(zeroCtrlGetSlideState() == ZERO_SLIDE_STARTED) {
		if(pad->Buttons & PSP_CTRL_HOME) {         		
                        zeroCtrlSetSlideState(ZERO_SLIDE_STOPPING);                        
                } else if(pad->Buttons & PSP_CTRL_UP) {
			level++;
			
			for(i=0; i<6; i++) {
				sceDisplayWaitVblankStart();
			}
                } else if(pad->Buttons & PSP_CTRL_DOWN) {
			level--;
			
			for(j=0; j<6; j++) {
				sceDisplayWaitVblankStart();
			}
                }
        }        
	
	pspSdkSetK1(k1);
        return ret;
}
//OK
int zeroCtrlGetCurrentClockLocalTime(pspTime *time) {
	int ret;
	int k1 = pspSdkSetK1(0);
	
	ret = sceRtcGetCurrentClockLocalTime(time);

	if(level != -1) {
		time->hour = level;
		time->minutes = 0;
	}

	pspSdkSetK1(k1);
	return ret;
}
//OK
int OnModuleStart(SceModule2 *mod) {       
        if(strcmp(mod->modname, "vsh_module") == 0) {
		if(devkit == 0x06020010) {								
			zeroCtrlRedir2Stub(mod->text_addr+0x6D78, slide_check_stub, zeroCtrlDummyFunc);
			vshCtrlReadBufferPositive = zeroCtrlRedir2Stub(mod->text_addr+0x3F1B0, vsh_ctrl_stub, zeroCtrlReadBufferPositive);
		} else if((devkit >= 0x06030010) && (devkit <= 0x06030910)) {		
			zeroCtrlRedir2Stub(mod->text_addr+0x6F6C, slide_check_stub, zeroCtrlDummyFunc);
			vshCtrlReadBufferPositive = zeroCtrlRedir2Stub(mod->text_addr+0x3FA3C, vsh_ctrl_stub, zeroCtrlReadBufferPositive);
		} else if(devkit == 0x06060010) {			
			zeroCtrlRedir2Stub(mod->text_addr+0x6F84, slide_check_stub, zeroCtrlDummyFunc);
			vshCtrlReadBufferPositive = zeroCtrlRedir2Stub(mod->text_addr+0x3FAF0, vsh_ctrl_stub, zeroCtrlReadBufferPositive);
		}
        } else if(strcmp(mod->modname, "sysconf_plugin_module") == 0) {
		if(devkit == 0x06020010) {			
			AddSysconfItem = zeroCtrlRedir2Stub(mod->text_addr+0x27918, add_sysconf_item_stub, zeroCtrlAddSysconfItem);		
		} else if((devkit >= 0x06030010) && (devkit <= 0x06030910)) {			
			AddSysconfItem = zeroCtrlRedir2Stub(mod->text_addr+0x2828C, add_sysconf_item_stub, zeroCtrlAddSysconfItem);		
		} else if(devkit == 0x06060010) {
			AddSysconfItem = zeroCtrlRedir2Stub(mod->text_addr+0x286AC, add_sysconf_item_stub, zeroCtrlAddSysconfItem);			
		}
		
		//To avoid the 'open slide' prompt after format 
		MAKE_CALL(mod->text_addr+0x240, zeroCtrlDummyFunc2);
	} else if(strcmp(mod->modname, "slide_plugin_module") == 0) {
		MAKE_CALL(mod->text_addr+0xC990, zeroCtrlGetCurrentClockLocalTime);
	}
	
       return previous ? previous(mod) : 0;
}
//OK
int module_start(SceSize args UNUSED, void *argp UNUSED) {  	
	devkit = sceKernelDevkitVersion();	
	
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);        
	return 0;
}
//OK
int module_stop(SceSize args UNUSED, void *argp UNUSED) {
    return 0;
}

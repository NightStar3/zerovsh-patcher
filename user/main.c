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

int devkit, model;

typedef struct
{
    char text[48];
    int play_sound;
    int action;
    int action_arg;
} SceContextItem; // 60

typedef struct
{
    int id; // 0
    int relocate; // 4
    int action; // 8
    int action_arg; // 12
    SceContextItem *context; // 16
    char *subtitle; // 20
    int unk; // 24
    char play_sound; // 28
    char memstick; // 29
    char umd_icon; // 30
    char image[4]; // 31
    char image_shadow[4]; // 35
    char image_glow[4]; // 39
    char text[37]; // 43
} SceVshItem; // 80

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

int (*AddVshItem)(void *arg, int topitem, SceVshItem *item) = NULL;
void (* AddSysconfItem)(u32 *option, SceSysconfItem **item) = NULL;
int (*CheckDriver)(int drvbit, u32 *umd_data, u32 *a2, u32 *a3) = NULL;

int zeroCtrlGetModelReal(void);

//OK
int zeroCtrlAddVshItem(void *arg, int topitem, SceVshItem *item) {	
	if(strcmp(item->text,"msg_bt_device_settings") == 0) {		
		return 0;	
	} 
	
	return AddVshItem(arg, topitem, item);
}
//OK
void zeroCtrlAddSysconfItem(u32 *option, SceSysconfItem **item) {
	if(strcmp((* item)->page, "page_psp_config_slide_action") == 0) {
		return;
	} else if(strcmp((*item)->page, "page_psp_config_color_space_mode") == 0) {
		return;
	}
	
	AddSysconfItem(option, item);
}
//OK
int zeroCtrlCheckDriver(int drvbit UNUSED, u32 *umd_data, u32 *a2, u32 *a3) {
	//0x1 == memstick
	//0x2 == umd
	//0x4 == usb(unused or ?)
	//0x8 == internal flash	
	return CheckDriver((0x1 | 0x2 | 0x4), umd_data, a2, a3);
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
int OnModuleStart(SceModule2 *mod) {       
        if(strcmp(mod->modname, "vsh_module") == 0) {
		if(devkit == 0x06020010) {					
			REDIRECT_FUNCTION(mod->text_addr+0x6CEC, zeroCtrlDummyFunc);	
			MAKE_CALL(mod->text_addr+0x1EEAC, zeroCtrlDummyFunc2);
			
			CheckDriver = (void *)(mod->text_addr+0x375FC);
			MAKE_CALL(mod->text_addr+0x14580, zeroCtrlCheckDriver);	
			
			AddVshItem = (void *)(mod->text_addr+0x21E18);		
			MAKE_CALL(mod->text_addr+0x206F8, zeroCtrlAddVshItem);					
		} else if((devkit >= 0x06030010) && (devkit <= 0x06030910)) {
			REDIRECT_FUNCTION(mod->text_addr+0x6E58, zeroCtrlDummyFunc);
			MAKE_CALL(mod->text_addr+0x1F660, zeroCtrlDummyFunc2);
			
			CheckDriver = (void *)(mod->text_addr+0x37EA8);
			MAKE_CALL(mod->text_addr+0x14B9C, zeroCtrlCheckDriver);
			
			AddVshItem = (void *)(mod->text_addr+0x22608);			
			MAKE_CALL(mod->text_addr+0x20EBC, zeroCtrlAddVshItem);			
		} else if(devkit == 0x06060010) {
			REDIRECT_FUNCTION(mod->text_addr+0x6E58, zeroCtrlDummyFunc);
			MAKE_CALL(mod->text_addr+0x1F644, zeroCtrlDummyFunc2);
			
			CheckDriver = (void *)(mod->text_addr+0x37F34);
			MAKE_CALL(mod->text_addr+0x14C64, zeroCtrlCheckDriver);
			
			AddVshItem = (void *)(mod->text_addr+0x22648);		
			MAKE_CALL(mod->text_addr+0x20EFC, zeroCtrlAddVshItem);			
		}
        } else if(strcmp(mod->modname, "sysconf_plugin_module") == 0) {
		if(devkit == 0x06020010) {
			AddSysconfItem = (void *)(mod->text_addr+0x27918);
			MAKE_CALL(mod->text_addr+0x1CA3C, zeroCtrlAddSysconfItem);
			
			if(model <= 1) {
				MAKE_CALL(mod->text_addr+0x1C924, zeroCtrlAddSysconfItem);
			}			
		} else if((devkit >= 0x06030010) && (devkit <= 0x06030910)) {
			AddSysconfItem = (void *)(mod->text_addr+0x2828C);
			MAKE_CALL(mod->text_addr+0x1D2BC, zeroCtrlAddSysconfItem);
			
			if(model <= 1) {
				MAKE_CALL(mod->text_addr+0x1D1A4, zeroCtrlAddSysconfItem);
			}		
		} else if(devkit == 0x06060010) {
			AddSysconfItem = (void *)(mod->text_addr+0x286AC);
			MAKE_CALL(mod->text_addr+0x1D6C8, zeroCtrlAddSysconfItem);
			
			if(model <= 1) {
				MAKE_CALL(mod->text_addr+0x1D5B0, zeroCtrlAddSysconfItem);
			}	
		}
	} 
	
       return previous ? previous(mod) : 0;
}
//OK
int module_start(SceSize args UNUSED, void *argp UNUSED) {  
	model = zeroCtrlGetModelReal();
	devkit = sceKernelDevkitVersion();	
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);        
	return 0;
}
//OK
int module_stop(SceSize args UNUSED, void *argp UNUSED) {
    return 0;
}

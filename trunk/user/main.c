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

int devkit;

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

//OK
int zeroCtrlAddVshItem(void *arg, int topitem, SceVshItem *item) {
	
	if(strcmp(item->text,"msg_bt_device_settings") == 0) {		
		return 0;	
	} else if(strcmp(item->text,"msg_em") == 0) {
		return 0;
	} else if(strcmp(item->text, "msgtop_game_savedata") == 0) {
		if(item->id != 25) {			
			return 0;
		}
	} 
	
	return AddVshItem(arg, topitem, item);
}
//OK
void zeroCtrlAddSysconfItem(u32 *option, SceSysconfItem **item) {
	if(strcmp((* item)->page, "page_psp_config_slide_action") == 0) {
		return;
	}
	
	AddSysconfItem(option, item);
}
//OK
int OnModuleStart(SceModule2 *mod) {       
        if(strcmp(mod->modname, "vsh_module") == 0) {
		if(devkit == 0x06020010) {
			AddVshItem = (void *)(mod->text_addr+0x21E18);
			
			//MAKE_CALL(mod->text_addr+0x20694, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x206F8, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x2242C, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x2248C, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x22508, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x22670, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x231E8, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x232C8, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x233A8, zeroCtrlAddVshItem);		
			//MAKE_CALL(mod->text_addr+0x2348C, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x235EC, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x29944, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x29954, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x29964, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x29978, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x29988, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A4A8, zeroCtrlAddVshItem);			
		} else if((devkit >= 0x06030010) && (devkit <= 0x06030910)) {
			AddVshItem = (void *)(mod->text_addr+0x22608);
			
			//MAKE_CALL(mod->text_addr+0x20E48, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x20EBC, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x22C1C, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x22C7C, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x22CF8, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x22E60, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x239D8, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x23AB8, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x23B98, zeroCtrlAddVshItem);		
			//MAKE_CALL(mod->text_addr+0x23C7C, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x23DDC, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A1B4, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A1C4, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A1D4, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A1E8, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A1F8, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2AD18, zeroCtrlAddVshItem);	
		} else if(devkit == 0x06060010) {
			AddVshItem = (void *)(mod->text_addr+0x22648);
			
			//MAKE_CALL(mod->text_addr+0x20E88, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x20EFC, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x22C7C, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x22CDC, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x22D5C, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x22EC0, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x23A44, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x23B24, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x23C04, zeroCtrlAddVshItem);		
			//MAKE_CALL(mod->text_addr+0x23CE8, zeroCtrlAddVshItem);
			//MAKE_CALL(mod->text_addr+0x23E48, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A240, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A250, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A260, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A274, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2A284, zeroCtrlAddVshItem);
			MAKE_CALL(mod->text_addr+0x2ADA4, zeroCtrlAddVshItem);	
		}
        }
        
        if(strcmp(mod->modname, "sysconf_plugin_module") == 0) {
		if(devkit == 0x06020010) {
			AddSysconfItem = (void *)(mod->text_addr+0x27918);
			MAKE_CALL(mod->text_addr+0x1CA3C, zeroCtrlAddSysconfItem);
		} else if((devkit >= 0x06030010) && (devkit <= 0x06030910)) {
			AddSysconfItem = (void *)(mod->text_addr+0x2828C);
			MAKE_CALL(mod->text_addr+0x1D2BC, zeroCtrlAddSysconfItem);
		} else if(devkit == 0x06060010) {
			AddSysconfItem = (void *)(mod->text_addr+0x286AC);
			MAKE_CALL(mod->text_addr+0x1D6C8, zeroCtrlAddSysconfItem);
		}
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

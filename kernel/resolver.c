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

#include "resolver.h"

#include <pspmoduleinfo.h>
#include <psploadcore.h>
#include <psputilsforkernel.h>
#include <string.h>
#include "logger.h"

#define MAKE_JUMP(a, f) _sw(0x08000000 | (((u32)(f) & 0x0ffffffc) >> 2), a);

u32 sctrlHENFindFunction(const char *modname, const char *libname, u32 nid);
unsigned int sceKernelQuerySystemCall(void * function);

u32 moduleprobe_nid;

nid nids[] =
{
    {
        	0x237DBD4F,
        	0x5630F321,
		0xEA349DC6,
        	0xE5E18A99,
        	0x4621A9CC,
		0x7158CE7E,
        	(u32)sceKernelAllocPartitionMemory,
    },
    {
        	0xB6D61D02,
        	0xAFBE8876,
		0xE8120E5C,
        	0x093DE56A,
        	0x8FDAFC4C,
		0xC1A26C6F,
        	(u32)sceKernelFreePartitionMemory,
    },
    {
        	0x9D9A5BA1,
        	0x950BCB31,
		0x2BDA1AC9,
        	0xFEB5C72B,
        	0x52B54B93,
		0xF12A62F7,
        	(u32)sceKernelGetBlockHeadAddr,
    },
    {
        	0x8B61808B,
        	0xEB988556,
		0x3D5EE53F,
        	0xAC9306F0,
        	0x399FF74C,
		0xF153B371,
        	(u32)sceKernelQuerySystemCall,
    },
    {
            0xBF983EF2,
            0x618C92FF,
            0x41BDF7C2,
            0xB95FA50D,
            0x7B411250,
            0x41D10899,
            (u32)sceKernelProbeExecutableObject,
    },
    {
            0xCF8A41B1,
            0x0B53340F,
            0xDEADBEEF,
            0xBEF0A05E,
            0xEF8A0BEA,
            0xF6B1BF0F,
            (u32)sceKernelFindModuleByName,
    },
    {
            0xBA889C07,
            0x4D473652,
            0xDEADBEEF,
            0x5352C26C,
            0x412D6ECC,
            0x4E62C48A,
            (u32)sceKernelLoadModuleBuffer,
    },
    {
            0x50F0C1EC,
            0xBB8C8FDF,
            0xDEADBEEF,
            0xDF8FFFAB,
            0xE6BF3960,
            0x3FF74DF1,
            (u32)sceKernelStartModule,
    },
};

libname libs[] = {
                { "sceSystemMemoryManager", "SysMemForKernel"              , 3 },
                { "sceInterruptManager"   , "InterruptManagerForKernel"    , 4 },
                { "sceLoaderCore"         , "LoadCoreForKernel"    	       , 6 },
		{ "sceModuleManager", "ModuleMgrForKernel", 8 },
};

void zeroCtrlResolveNids(void) {

    int fw_version;
    u32 fw_nid, func;
    int count = 0;

    fw_version = sceKernelDevkitVersion();

    for(u32 i = 0; i < ITEMSOF(nids); i++) {
    	if(i == libs[count].count) {
    		count++;
    	}
    	if(fw_version <= 0x03050210) {
    		fw_nid = nids[i].nidsha1;
    	} else if((fw_version >= 0x05000010) && (fw_version <= 0x05050010)) {
            fw_nid = nids[i].nid5xx;
        } else if(fw_version == 0x05070010) {
            fw_nid = nids[i].nid570;
    	} else if(fw_version == 0x06020010) {
    		fw_nid = nids[i].nid620;
    	} else if((fw_version >= 0x06030010) && (fw_version <= 0x06030910)) {
    		fw_nid = nids[i].nid63x;
    	} else if(fw_version == 0x06060010) {
    		fw_nid = nids[i].nid660;
    	} else {
    		zeroCtrlWriteDebug("unknown firmware version: %08X\n", fw_version);
    		return;
    	}

    	func = sctrlHENFindFunction(libs[count].prxname, libs[count].name, fw_nid);
    	if(!func) {
    		zeroCtrlWriteDebug("Cannot find address for nid: %08X\n", fw_nid);
    		continue;
    	}

    	if(nids[i].stub == (u32)sceKernelProbeExecutableObject) {
    		   moduleprobe_nid = fw_nid;
    	}

    	MAKE_JUMP(nids[i].stub, func);
    	_sw(0, nids[i].stub + 4); // NOP
    	sceKernelDcacheWritebackInvalidateRange((const void *)nids[i].stub, 8);
    	sceKernelIcacheInvalidateRange((const void *)nids[i].stub, 8);
    }
}

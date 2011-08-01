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

u32 sctrlHENFindFunction(char *modname, char *libname, u32 nid);
unsigned int sceKernelQuerySystemCall(void * function);


nid nids[] =
{
    {
    		0x237DBD4F,
    		0x5630F321,
    		0xE5E18A99,
    		0x4621A9CC,
    		(u32)sceKernelAllocPartitionMemory,
    },
    {
    		0xB6D61D02,
    		0xAFBE8876,
    		0x093DE56A,
    		0x8FDAFC4C,
    		(u32)sceKernelFreePartitionMemory,
    },
    {
    		0x9D9A5BA1,
    		0x950BCB31,
    		0xFEB5C72B,
    		0x52B54B93,
    		(u32)sceKernelGetBlockHeadAddr,
    },
    {
    		0x8B61808B,
    		0xEB988556,
    		0xAC9306F0,
    		0x399FF74C,
    		(u32)sceKernelQuerySystemCall,
    },
};

libname libs[] = {
		{ "sceSystemMemoryManager", "SysMemForKernel"          , 3 },
		{ "sceInterruptManager"   , "InterruptManagerForKernel", 4 },
};

void zeroCtrlResolveNids(void) {

	int fw_version;
	u32 fw_nid, func;
	int count = 0;

	fw_version = sceKernelDevkitVersion();

	for(int i = 0; i < sizeof(nids) / sizeof(nid); i++) {
		if(i == libs[count].count) {
			count++;
		}
		if((fw_version >= 0x05000010) & (fw_version <= 0x05050010)) {
			fw_nid = nids[i].nid5xx;		 
		} else if(fw_version == 0x06020010) {
			fw_nid = nids[i].nid620;
		} else if((fw_version >= 0x06030010) & (fw_version <= 0x06030910)) {
			fw_nid = nids[i].nid63x;
		} else {
			return;
		}
		zeroCtrlWriteDebug("Searching for %s, %s, %08X\n", libs[count].prxname, libs[count].name, nid);
		func = sctrlHENFindFunction(libs[count].prxname, libs[count].name, fw_nid);
		if(!func) {
			zeroCtrlWriteDebug("Cannot find address for nid: %08X\n", nid);
			continue;
		}
		MAKE_JUMP(nids[i].stub, func);
		_sw(0, nids[i].stub + 4); // NOP
		sceKernelDcacheWritebackInvalidateRange((const void *)nids[i].stub, 8);
		sceKernelIcacheInvalidateRange((const void *)nids[i].stub, 8);
	}
}

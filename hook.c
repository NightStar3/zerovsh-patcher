/*
 * Copyright 2010 by Coldbird
 *
 * hook.c - Import Hooking Functions
 *
 * This file is part of Adhoc Emulator.
 *
 * Adhoc Emulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Adhoc Emulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Adhoc Emulator.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <psputils.h>
#include <string.h>

#include "systemctrl.h"
#include "hook.h"
#include "logger.h"


#define SYSCALL_OPCODE 0x0000000C
#define MAKE_SYSCALL( a, f ) _sw(SYSCALL_OPCODE | ( ( ( unsigned int )( f ) & 0x03ffffff ) << 6), a )
#define EXTRACT_SYSCALL(x) (((x) & ~SYSCALL_OPCODE) >> 6)

typedef struct
{
  const char * name;
  unsigned short version;
  unsigned short attribute;
  unsigned char entLen;
  unsigned char varCount;
  unsigned short funcCount;
  unsigned int * fnids;
  unsigned int * funcs;
  unsigned int * vnids;
  unsigned int * vars;
}PspModuleImport;

unsigned int sceKernelQuerySystemCall(void * function);

PspModuleImport * find_import_lib(SceModule2 * module, const char * library)
{
  //library import pointer
  PspModuleImport * result = NULL;

  //valid arguments
  if(module && library)
  {
    //check stub table
    unsigned int x = 0; while(x < module->stub_size)
    {
      //grab library stub
      result = (PspModuleImport *)(module->stub_top + x);
      //library found
      if(result->name && strcmp(result->name, library) == 0)
      {
        //stop search
        break;
      }

      //move forward
      x += result->entLen * 4;
    }

    //library not found
    if(x == module->stub_size) result = NULL;
  }

  //return library import pointer
  return result;
}

void list_imports(SceModule2 * module, const char * library)
{
  //find library import
  PspModuleImport * import = find_import_lib(module, library);

  //found import
  if(import)
  {
    //find stub
    unsigned int x = 0; for(; x < import->funcCount; x++)
    {
      //found stub
    }
  }
}

unsigned int find_import_bynid(SceModule2 * module, const char * library, unsigned int nid)
{
  //stub address
  unsigned int result = 0;

  //find library import
  PspModuleImport * import = find_import_lib(module, library);

  //found import
  if(import)
  {
	//kprintf("%s: Found import lib: %s\n", __FILE__, library);
    //find stub
    unsigned int x = 0; for(; x < import->funcCount; x++)
    {
	  //kprintf("%s: checking import NID: %08X\n", __FILE__, import->fnids[x]);
      //found stub
      if(import->fnids[x] == nid)
      {
        //save stub address
        result = (unsigned int)&import->funcs[x * 2];

        //stop search
        break;
      }
    }
  }

  //return stub address
  return result;
}

void api_hook_import(unsigned int address, void * function)
{
  //valid address
  if(address)
  {
    //asm jmp
    *(unsigned int *)(address) = 0x08000000 | ((unsigned int)function & 0x0FFFFFFF) >> 2;

    //asm nop
    *(unsigned int *)(address + 4) = 0;

    //flush cache
    sceKernelDcacheWritebackInvalidateRange((const void *)address, 8);
    sceKernelIcacheInvalidateRange((const void *)address, 8);
  }
}

void api_hook_import_syscall(unsigned int address, void * function)
{
  //valid address
  if(address)
  {
    //asm jr $ra
    *(unsigned int *)(address) = 0x03E00008;

    //asm syscall #
    MAKE_SYSCALL(address + 4, sceKernelQuerySystemCall(function));

    //flush cache
    sceKernelDcacheWritebackInvalidateRange((const void *)address, 8);
    sceKernelIcacheInvalidateRange((const void *)address, 8);
  }
}

int syscall_patched(const char *mod_name, const char *library, u32 nid, u32 stub) {
	u32 stub_sys, func_sys, sys_op;

	void *func = (void *)sctrlHENFindFunction(mod_name, library, nid);
	zeroCtrlWriteDebug("mod: %s, lib: %s, nid: %08X, func: %08X\n", mod_name, library, nid, func);
	sys_op = *(u32 *)(stub + 4);

	zeroCtrlWriteDebug("Stub opcode: %08X\n", *(u32 *)(stub));
	if(!(sys_op & SYSCALL_OPCODE) || *(u32 *)(stub) != 0x03E00008) {
		zeroCtrlWriteDebug("Stub already patched\n");
		return 1;
	}

	if(func != NULL) { // check if a syscall is placed

		zeroCtrlWriteDebug("Syscall opcode: %08X\n", sys_op);
		stub_sys = EXTRACT_SYSCALL(sys_op);
		zeroCtrlWriteDebug("Stub syscall number: %i\n", stub_sys);
		func_sys = sceKernelQuerySystemCall(func);
		zeroCtrlWriteDebug("Real syscall number: %i\n", func_sys);

		// func_sys == -1 :      syscall already patched
		// func_sys != stub_sys: syscall replaced by another one
		if(func_sys == -1 || func_sys != stub_sys) {
			zeroCtrlWriteDebug("Syscall already patched\n");
			return 1;
		}
	}
	return 0;

}

int hook_import_bynid(SceModule2 * module, const char * library, unsigned int nid, void * function, int syscall, const char *mod_name)
{
  //result
  int result = 0;
  //find import stub
  unsigned int stub = find_import_bynid(module, library, nid);

  //found stub
  if(stub)
  {
    //write syscall
    if(syscall) {
		if(mod_name && syscall_patched(mod_name, library, nid, stub)) {
			return -2;
		}
		api_hook_import_syscall(stub, function);
	}
    //write jump
    else  {
		api_hook_import(stub, function);
	}
  }

  //stub not found
  else result = -1;

  //return result
  return result;
}

.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForKernel" 0x00090000, 0x00020005
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_FUNC 0x159AF5CC, sctrlHENFindFunction
	STUB_END

	STUB_START "IoFileMgrForUser" 0x00010000, 0x00020005
	STUB_FUNC 0x109F50BC, sceIoOpen_user
	STUB_FUNC 0xACE946E8, sceIoGetstat_user
	STUB_END

	//STUB_START "SysMemForKernel" 0x00090000, 0x00030005
	//STUB_FUNC 0xDEADC0DE, sceKernelAllocPartitionMemory
	//STUB_FUNC 0xDEADC0DE, sceKernelFreePartitionMemory
	//STUB_FUNC 0xDEADC0DE, sceKernelGetBlockHeadAddr
	//STUB_END

	//STUB_START "InterruptManagerForKernel" 0x00090000, 0x00010005
	//STUB_FUNC 0xDEADC0DE, sceKernelQuerySystemCall
	//STUB_END

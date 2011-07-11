.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForKernel" 0x00090000, 0x00010005
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_END

	STUB_START "IoFileMgrForUser" 0x00010000, 0x00020005
	STUB_FUNC 0x109F50BC, sceIoOpen_user
	STUB_FUNC 0xACE946E8, sceIoGetstat_user
	STUB_END

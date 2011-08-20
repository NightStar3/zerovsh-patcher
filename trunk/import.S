.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForKernel" 0x00090000, 0x00030005
	STUB_FUNC 0x159AF5CC, sctrlHENFindFunction
	STUB_FUNC 0x78E46415, sctrlHENFindDriver
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_END

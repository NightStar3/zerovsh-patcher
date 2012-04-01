.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForUser" 0x40090000, 0x00010005
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_END

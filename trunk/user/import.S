.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForUser" 0x40090000, 0x00010005
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_END

	STUB_START "ZeroCtrlForUser" 0x00090000, 0x00010005
	STUB_FUNC 0x01337357, zeroCtrlGetModelReal
	STUB_END
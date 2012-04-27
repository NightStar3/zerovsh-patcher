.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForUser" 0x40090000, 0x00020005
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_FUNC 0x98012538,SetSpeed
	STUB_END

	STUB_START "ZeroCtrlForUser" 0x40090000, 0x00060005
	STUB_FUNC 0x13373570, zeroCtrlGetSlideState
	STUB_FUNC 0x13373571, zeroCtrlSetSlideState
	STUB_FUNC 0x13373572, zeroCtrlGetSlideConfig
	STUB_FUNC 0x13373573, zeroCtrlSetSlideConfig
	STUB_FUNC 0x13373574, zeroCtrlGetModel
	STUB_FUNC 0x13373575, zeroCtrlContrast2Hour
	STUB_END

	STUB_START "sceRtc" 0x40090000, 0x00010005
	STUB_FUNC 0xE7C27D1B, sceRtcGetCurrentClockLocalTime
	STUB_END


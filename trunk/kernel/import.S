.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForKernel" 0x00090000, 0x00040005
	STUB_FUNC 0x159AF5CC, sctrlHENFindFunction
	STUB_FUNC 0x78E46415, sctrlHENFindDriver
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_FUNC 0xCC9ADCF8,sctrlHENSetSpeed
	STUB_END

	STUB_START "sceVshBridge" 0x00090000, 0x00010005
	STUB_FUNC 0x639C3CB3,vshImposeGetParam
	STUB_END

	STUB_START "sceCtrl" 0x40090000, 0x00010005
	STUB_FUNC 0x0B588501,sceCtrlReadLatch
	STUB_END

	STUB_START "sceReg" 0x00090000, 0x00070005
	STUB_FUNC 0x92E41280,sceRegOpenRegistry
	STUB_FUNC 0x1D8A762E,sceRegOpenCategory
	STUB_FUNC 0x39461B4D,sceRegFlushRegistry
	STUB_FUNC 0xFA8A5739,sceRegCloseRegistry
	STUB_FUNC 0x17768E14,sceRegSetKeyValue
	STUB_FUNC 0x0CAE832B,sceRegCloseCategory
	STUB_FUNC 0x0D69BF40,sceRegFlushCategory
	STUB_END

	STUB_START "sceSyscon_driver" 0x00090000, 0x00010005
	STUB_FUNC 0x18BFBE65,sceSysconCtrlLED
	STUB_END


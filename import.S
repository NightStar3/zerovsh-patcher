.set noreorder

#include "pspstub.s"

	STUB_START "SystemCtrlForKernel" 0x00090000, 0x00020005
	STUB_FUNC 0x1C90BECB, sctrlHENSetStartModuleHandler
	STUB_FUNC 0x159AF5CC, sctrlHENFindFunction
	STUB_END

	STUB_START "sceVshBridge",0x40090000,0x00010005
    STUB_FUNC 0xA5628F0D, vshKernelLoadModuleVSH
    STUB_END

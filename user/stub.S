.macro CREATE_STUB funcname
    .set noreorder
    .globl \funcname
    .type \funcname @function
    .ent \funcname, 0
\funcname:
    nop
    nop
    j 0
    nop
    .end \funcname
.endm 


CREATE_STUB add_sysconf_item_stub
CREATE_STUB slide_check_stub

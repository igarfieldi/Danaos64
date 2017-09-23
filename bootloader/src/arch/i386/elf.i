

.set        ELF_MAGIC1_OFF,     0
.set        ELF_MAGIC2_OFF,     1
.set        ELF_MAGIC3_OFF,     2
.set        ELF_MAGIC4_OFF,     3
.set        ELF_CLASS_OFF,      4

.set        ELF_MAGIC1,         0x7F
.set        ELF_MAGIC2,         'E'
.set        ELF_MAGIC3,         'L'
.set        ELF_MAGIC4,         'F'
.set        ELF_CLASS_32,       1
.set        ELF_CLASS_64,       2


.type		_loadElf, @function
// eax: file start
_load_elf:
    movb    (%ax), %bl
    cmpb    %bl, 0x7F
    jne     not_elf
    cmpb    ELF_MAGIC2_OFF(%ax), $ELF_MAGIC2
    jne     not_elf
    cmpb    ELF_MAGIC3_OFF(%ax), $ELF_MAGIC3
    jne     not_elf
    cmpb    ELF_MAGIC4_OFF(%ax), $ELF_MAGIC4
    jne     not_elf
    cmpb    ELF_CLASS_OFF(%ax), $ELF_CLASS_32
    jne     not_elf

not_elf:
    movw    $not_elf_msg, %si
    call    _printMsg
    hlt

not_elf_msg:
    .asciz  "The file is not ELF32!" 

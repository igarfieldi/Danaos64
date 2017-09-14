ISA_C_FLAGS	:= -m64 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone
ISA_CPP_FLAGS	:= -m64 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone
ISA_ASM_FLAGS	:= --64
ISA_LD_FLAGS	:= -mno-red-zone -z max-page-size=0x1000

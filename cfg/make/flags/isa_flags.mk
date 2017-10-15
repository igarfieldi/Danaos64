i386-CCFLAGS		:= -m32
i386-CPPFLAGS		:= -m32
i386-ASMFLAGS		:= --32
i386-LDFLAGS		:= -m32

x86_64-CCFLAGS		:= -m64 -mcmodel=kernel -mno-mmx -mno-sse -mno-sse2 -mno-red-zone
x86_64-CPPFLAGS		:= -m64 -mcmodel=kernel -mno-mmx -mno-sse -mno-sse2 -mno-red-zone
x86_64-ASMFLAGS		:= --64
x86_64-LDFLAGS		:= -m64 -mcmodel=kernel -mno-red-zone -z max-page-size=0x1000

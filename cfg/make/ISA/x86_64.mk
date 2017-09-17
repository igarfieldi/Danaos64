CCFLAGS		+= -m64 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone
CPPFLAGS	+= -m64 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone
ASMFLAGS	+= --64
LDFLAGS		+= -m64 -mno-red-zone -z max-page-size=0x1000

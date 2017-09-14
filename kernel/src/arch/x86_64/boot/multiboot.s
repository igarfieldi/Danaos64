// Multiboot2 header constants
.set			MAGIC,					0xE85250D6			// Magic number
.set			ARCHITECTURE,			0

// Multiboot section
.section		.multiboot, "a", @progbits

.align			8
_multibootHeaderStart:
.long			MAGIC
.long			ARCHITECTURE
.long			_multibootHeaderEnd - _multibootHeaderStart
.long			-(MAGIC + ARCHITECTURE + (_multibootHeaderEnd - _multibootHeaderStart))
// Tag - Elf symbols
.align			8
_infoTagStart:
.short			1
.short			0
.int			_infoTagEnd - _infoTagStart
.int			2			// Request boot loader name
.int			9			// Request ELF symbols
_infoTagEnd:
// Final tag
.align			8
.short			0			// Type
.short			0			// Flags
.int			8			// Size
_multibootHeaderEnd:

	// XXX - Fix these to work on other architectures.
	.macro .alignhere
	.balign 8
	.endm

	.macro .cell value
	.quad \value
	.endm
	// XXX - Fix these to work on other architectures.

	.include "script_dict_def.s"

	dict_end

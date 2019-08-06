	.set link, 0

	.macro defdict name, label, flags=0
	.globl dict_\label
dict_\label :
	.int link
	.set link,dict_\label

	.byte \flags
	.byte 1f - 0f
0:
	.ascii "\name"
1:
	.balign 4
xt_\label:
	.endm

	.macro defcode name, label, flags=0
	defdict "\name", \label, \flags
	.int script_word_\label
	.endm

	.macro cfa name
	.int script_word_\name
	.endm

	.macro xt label
	.int xt_\label
	.endm

	.macro relpos pos
	.int \pos - .
	.endm

	#include "script.h"

dict_start:
	.global script_dict_start
	defcode "sp", sp
	defcode "sp0", sp0
	defcode "rp", rp
	defcode "rp0", rp0
	defcode "//",comment_line
	defcode "lit",lit
	defcode "rel",rel
	defcode "@",fetch
	defcode "!",store
	defcode "c@",cfetch
	defcode "c!",cstore
	defcode "dup",dup
	defcode "?dup",dup_if
	defcode "drop",drop
	defcode "over",over
	defcode "swap",swap
	defcode "r>",rpop
	defcode ">r",rpush
	defcode "r@",rfetch
	defcode "&",and
	defcode "|",or
	defcode "^", xor
	defcode "~", invert
	defcode "+", add
	defcode "-", sub
	defcode "negate", negate
	defcode "<<", lshift
	defcode ">>", rshift
	defcode "<", lt
	defcode "<=", lteq
	defcode ">", gt
	defcode ">=", gteq
	defcode "0=", is_zero
	defcode "*", mult
	defcode "/mod", divmod
	defcode ",", comma
	defcode "c,", char_comma
	defcode "\",", quote_comma
	defcode "0branch",zero_branch
	defcode "branch",branch
	defcode "docol", docol
	defcode "exit", exit
	defcode "docon", docon
	defcode "dovar", dovar
	defcode "get-current", get_current
	defcode "set-current", set_current
	defcode "latest", latest
	defcode "cell", cell
	defcode "align", align
	defcode "aligned", aligned
	defcode "[",interp_mode, 1
	defcode "]",compile_mode
	defcode "compiling", compiling
	defcode "base", base
	defcode "dp", dp
	defcode "find-nt", find_name
	defcode "execute", execute
	defcode "tib", tib
	defcode "#tib", tiblen
	defcode "in>", tibpos
	defcode "pad", pad
	defcode ".", pop_and_display
	defcode ".s", stack_dump
	defcode "accept", accept
	defcode "parse-name", parse_name
	defcode "type", type
	defcode "restart", restart
	defcode "quit", quit
	defcode "context", context
	defcode "#order", norder

	defdict "header", header
	cfa docol
	xt dp
	xt fetch
	xt latest
	xt store
	xt get_current
	xt fetch
	xt comma
	xt lit
	.int 0
	xt char_comma
	xt parse_name
	xt quote_comma
	xt align
	xt exit

	defdict ":", colon
	cfa docol
	xt header
	xt lit
	cfa docol
	xt comma
	xt compile_mode
	xt exit

	defdict ";", semi, 1
	cfa docol
	xt interp_mode
	xt lit
	xt exit
	xt comma
	xt latest
	xt fetch
	xt get_current
	xt store
	xt exit

	defdict "immediate", immediate, 1
	cfa docol
	xt lit
	.int 1
	xt latest
	xt fetch
	xt cell
	xt add
	xt cstore
	xt exit

	.macro dict_end
	.set _dict_end_link, link
	defdict "_dict_end", _dict_end
	cfa docon
	.int _dict_end_link

	.balign 4
	.global script_dict_end
script_dict_end:
	.int link
	.endm

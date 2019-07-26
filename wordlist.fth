get-current constant core

// Wordlists
// -----------------
: wordlist here 0 , ;

wordlist constant flash
_dict_end flash !

variable #order 0 #order !
create context 16 cells allot

: get-order #order @ context over 1 - cells + swap begin dup while >r dup @ swap cell - r> 1 - repeat 2drop #order @ ;
: set-order dup #order ! context swap begin dup while >r swap over ! cell + r> 1 - repeat 2drop ;

: find-nt context #order @
    begin
	dup
    while
	    >r >r
	    2dup r@
	    @ wordlist-find-nt ?dup if -rot 2drop r> drop r> drop exit then
	    r> cell +
	    r> 1 -
    repeat
    2drop 2drop 0
;

: words get-order begin ?dup while >r @ (words) r> 1 - repeat ;

: ' parse-name find-nt nt>xt ;

: process-name
    dup 0= if drop drop exit then
    2dup find-nt ?dup if
	-rot 2drop dispatch-word exit
    else
	dispatch-number
    then ;

: outer begin available while parse-name process-name repeat prompt ;
: quit begin tib 256 accept #tib ! 0 in> ! outer again ;

core 1 set-order
quit

hex

: available in> @ #tib @ < ;
: advance in> @ 1 + in> ! ;
: cur@ tib in> @ + c@ ;
: seek-tib true begin available & while cur@ over execute dup if advance then repeat drop ;
: key available if cur@ advance then ;

: is-delim? 21 < ;
: skip-delim ['] is-delim? seek-tib ;

: char skip-delim key ;
: [char] char compile, lit , ; immediate

: not-nl? A = 0= ;
: // ['] not-nl? seek-tib ;

: not-delim? is-delim? 0= ;
: gather-word in> @ tib over + swap ['] not-delim? seek-tib in> @ swap - ;
: parse-name skip-delim gather-word ;

// Number Conversion Routines
// --------------------------
: check-base dup base @ < ;

: c>n
    dup [char] 0 >= over [char] 9 <= & if [char] 0 - check-base exit then
    dup [char] a >= if lit [ char a char A - , ] - then
    dup [char] A >= over [char] Z <= & if [char] A - A + check-base exit then
    false ;

: >number begin dup 0 > while
	    over c@ c>n 0=
	    if drop exit then
	    >r rot base @ * r> + -rot 1 - swap 1 + swap
    repeat ;

: n>c [char] 0 + dup [char] 9 > if lit [ char A char : - , ] + then ;
: (number>) dup rot begin dup while base @ /mod >r n>c c!+ r> repeat drop over - ;
: number> (number>) 2dup creverse ;

create pad 50 allot
: . pad number> type ;

// String Handling
// ---------------

: not-quote? [char] " = 0= ;
: " skip-delim in> @ tib over + swap ['] not-quote? seek-tib in> @ advance swap - ;
: ", dup c, here over allot swap cmove ;
: count dup c@ swap 1 + swap ;
: (c") r> dup dup c@ + 1 + >r count ;
: ["] compile, (c") " ", ; immediate
: ." [ ' ["] , ] compile, type ; immediate


// Error Handlings
// ---------------
defer abort
' restart ' abort defer!

: abort" [ ' ." , ] compile, abort ; immediate

// Outer Interpreter/Compiler
// --------------------------

defer ingest-number

: compile-number compiling @ if compile, lit , then ;
' compile-number is ingest-number
: convert-number 0 -rot >number dup 0 > if ." failed to ingest number: " type abort else 2drop then ;
: dispatch-number convert-number ingest-number ;

: dispatch-word dup is-immediate? compiling @ 0= | swap nt>xt swap if execute else , then ;

: process-name
    dup 0= if drop drop exit then
    2dup find-nt ?dup if -rot 2drop dispatch-word exit else dispatch-number then ;

: prompt (c") [ 4 c, 20 c, char o c, char k c, A c, ] type ;

: outer begin available while parse-name process-name repeat prompt ;

: quit begin tib 80 accept #tib ! 0 in> ! outer again ;

quit

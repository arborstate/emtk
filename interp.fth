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

// Outer Interpreter

: not-delim? is-delim? 0= ;
: gather-word in> @ tib over + swap ['] not-delim? seek-tib in> @ swap - ;
: parse-name skip-delim gather-word ;

: check-base dup base @ < ;

: c>n
    dup [char] 0 >= over [char] 9 <= & if [char] 0 - check-base exit then
    dup [char] a >= if lit [ char a char A - , ] - then
    dup [char] A >= over [char] Z <= & if [char] A - A + check-base exit then
    false ;

: >number begin dup 0 > while over c@ c>n 0= if drop exit then >r rot base @ * r> + -rot 1 - swap 1 + swap repeat ;

: dispatch-word dup is-immediate? compiling @ 0= | swap nt>xt swap if execute else , then ;
: dispatch-number 0 -rot >number 2drop compiling @ if compile, lit , then ;
: process-name
    dup 0= if drop drop exit then
    2dup find-nt ?dup if -rot 2drop dispatch-word exit else dispatch-number then ;

: count dup c@ swap 1 + swap ;

: (.") r> dup count type dup c@ + 1 + >r ;
: not-quote? [char] " = 0= ;
: " in> @ tib over + swap ['] not-quote? seek-tib in> @ advance swap - ;
: ", dup c, here over allot swap cmove ;
: ." compile, (.") " ", ; immediate

: outer begin available while parse-name process-name repeat ." bok" ;
: quit begin tib 80 accept #tib ! 0 in> ! outer again ;

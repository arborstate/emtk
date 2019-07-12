hex

: available in> @ #tib @ < ;
: advance in> @ 1 + in> ! ;
: cur@ tib in> @ + c@ ;

: isspace? 0 over 20 = | over 9 = | over A = | over D = | swap drop ;
: notspace? isspace? 0= ;
: seek-tib true begin available & while cur@ over execute dup if advance then repeat drop ;
: skip-space ['] isspace? seek-tib ;
: gather-word in> @ tib over + swap ['] notspace? seek-tib in> @ swap - ;

: parse-name skip-space gather-word ;

: key available if cur@ advance then ;
: char skip-space key ;
: [char] char compile, lit , ; immediate

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

: outer begin available while parse-name process-name repeat ;

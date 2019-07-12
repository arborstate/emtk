hex

: isspace? 0 over 20 = | over 9 = | over A = | over D = | swap drop ;

: available in> @ #tib @ < ;
: advance in> @ 1 + in> ! ;
: cur@ tib in> @ + c@ ;

: skip-space begin available while cur@ isspace? 0= if exit then advance repeat ;
: gather-word in> @ tib over + swap 1 begin available & while cur@ isspace? 0= dup if advance then repeat in> @ swap - ;

: parse-word skip-space gather-word ;
: process-word dup 0= if drop drop exit then findxt if execute then ;

: outer begin available while parse-word process-word repeat ;

variable state

: [ 0 state ! ; immediate
: ] 1 state ! ;

here link @ , 0 c, parse-name (header) ", align
docol, ] here link @ , 0 c, parse-name ", align link ! exit [ link !

(header) : docol, ] (header) docol, ] exit [
: ; [ parse-name [ findxt drop , ] lit exit , exit [ 1 link @ cell + c!

: immediate 1 link @ cell + c! ;
: ' parse-name findxt drop ;
: xt>cfa @ ;
: xt>pf cell + ;
: cells cell * ;

(header) latestxt ' dovar xt>cfa , 0 ,
: header (header) here latestxt ! ;
: : header docol, ] ;

: ['] lit [ ' lit , ] , ' , ; immediate
: compile, ['] lit , ' , ['] , , ; immediate

: create header docol, compile, lit here 0 , compile, exit compile, exit here swap ! ;
: (does) latestxt @ xt>pf 2 cells + ! ;
: does> compile, lit here 0 , compile, (does) compile, exit here swap ! docol, ; immediate

: constant create , does> @ ;
: variable create 0 , ;

: allot dp @ + dp ! ;

: hex 16 base ! ;
: decimal 10 base ! ;

: = - 0= ;

: (stash) compile, lit here 0 , ;
: (if) swap 0= & r> + >r ;
: if (stash) compile, (if) ; immediate
: (patch) here 2 cell * - over -  swap ! ;
: (else) r> + >r ;
: else (stash) compile, (else) swap (patch) ; immediate
: then (patch) ; immediate

: begin here ; immediate
: (again) r> drop >r ;
: again compile, lit , compile, (again) ; immediate
: (until) swap 0= if r> drop >r exit then drop ;
: until compile, lit , compile, (until) ; immediate
: while (stash) compile, (until) ; immediate
: repeat swap [ ' again , ] here swap ! ; immediate

: 2dup over over ;
: cmove begin dup while >r over c@ over c! 1 + swap 1 + swap r> 1 - repeat drop drop drop ;

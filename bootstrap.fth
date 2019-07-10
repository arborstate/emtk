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

: constant header lit [ ' docon xt>cfa , ] , , ;
: create header lit [ ' dovar xt>cfa , ] , ;
: variable create 0 , ;

: noop ;
: make header docol, compile, lit here 0 , compile, noop compile, exit here swap ! ;
: (does) latestxt @ xt>pf 2 cells + ! ;
: does> compile, lit here 0 , compile, (does) compile, exit here swap ! docol, ; immediate

: allot dp @ + dp ! ;

: hex 16 base ! ;
: decimal 10 base ! ;

: = - 0= ;

hex

: (stash) ['] lit , here 0 , ;
: (if) swap 0= & r> + >r ;
: if (stash) ['] (if) , ; immediate
: (patch) here 2 cell * - over -  swap ! ;
: (else) r> + >r ;
: else (stash) ['] (else) , swap (patch) ; immediate
: then (patch) ; immediate

: begin here ; immediate
: (again) r> drop >r ;
: again ['] lit , , ['] (again) , ; immediate
: (until) swap 0= if r> drop >r exit then drop ;
: until ['] lit , , ['] (until) , ; immediate

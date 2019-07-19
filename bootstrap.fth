// Dictionary Helpers
// ------------------
: immediate 1 latest @ cell + c! ;
: is-immediate? cell + c@ 1 & ;

: here dp @ ;
: allot dp @ + dp ! ;

: nt>xt cell + 1 + dup c@ + 1 + aligned ;
: xt>cfa @ ;
: xt>pf cell + ;
: cells cell * ;

: ' parse-name find-nt nt>xt ;
: ['] lit [ ' lit , ] , ' , ; immediate
: compile, ['] lit , ' , ['] , , ; immediate
: postpone ' , ; immediate

// Complex Word Defining
// ---------------------

: ref< here ;
: <ref here - , ;

: ref> here 0 , ;
: >ref here over - swap ! ;

: create header docol, compile, rel ref> compile, exit compile, exit >ref latest @ link ! ;
: xt>here xt>pf cell + dup @ + ;
: (does) latest @ nt>xt xt>pf 2 cells + ! ;
: does> compile, rel ref> compile, (does) compile, exit >ref docol, ; immediate

: constant create , does> @ ;
: variable create 0 , ;

: defer create compile, exit does> @ execute ;
: deferaddr xt>here ;
: is ' deferaddr ! ;
: defer! deferaddr ! ;
: defer@ deferaddr @ ;

: marker create link @ , here cell + , does> dup @ link ! cell + @ dp ! ;

// Conditionals And Looping Constructs
// -----------------------------------

: if compile, 0branch ref> ; immediate
: else compile, branch ref> swap >ref ; immediate
: then >ref ; immediate

: begin ref< ; immediate
: again compile, branch <ref ; immediate
: until compile, 0branch <ref ; immediate
: while compile, 0branch ref> ; immediate
: repeat swap compile, branch <ref >ref ; immediate


// Extended Stack Manipulation
// ---------------------------
: ?dup dup if dup then ;
: 2dup over over ;
: 2drop drop drop ;

: -rot swap >r swap r> ;
: rot >r swap r> swap ;


// Helpers
// -------

: hex 16 base ! ;
: decimal 10 base ! ;

: true 1 ;
: false 0 ;

: = - 0= ;
: / /mod swap drop ;
: min 2dup > if swap then drop ;

// Character Memory Access
// -----------------------
: c!+ over c! 1 + ;
: c!- over c! 1 - ;
: c@+ dup c@ swap 1 + swap ;

: cmove begin dup while >r over c@ over c! 1 + swap 1 + swap r> 1 - repeat drop drop drop ;
: creverse
    2dup + 1 - swap 2 /
    begin dup while
	    1 - >r
	    2dup
	    c@ swap
	    c@ swap
	    -rot
	    c!-
	    -rot
	    c!+
	    swap
	    r>
    repeat
    drop drop drop
;

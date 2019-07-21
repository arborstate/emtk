// Dictionary Helpers
// ------------------
: is-immediate? cell + c@ 1 & ;

: here dp @ ;
: allot dp @ + dp ! ;

: nt>xt cell + 1 + dup c@ + 1 + aligned ;
: xt>cfa @ ;
: xt>pf cell + ;
: cells cell * ;

: ' parse-name find-nt nt>xt ;
: ['] immediate lit [ ' lit , ] , ' , ;
: compile, immediate ['] lit , ' , ['] , , ;
: postpone immediate ' , ;

// Complex Word Defining
// ---------------------

: ref< here ;
: <ref here - , ;

: ref> here 0 , ;
: >ref here over - swap ! ;

: create header docol, compile, rel ref> compile, exit compile, exit >ref latest @ link ! ;
: xt>here xt>pf cell + dup @ + ;
: (does) latest @ nt>xt xt>pf 2 cells + ! ;
: does> immediate compile, rel ref> compile, (does) compile, exit >ref docol, ;

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

: if immediate compile, 0branch ref> ;
: else immediate compile, branch ref> swap >ref ;
: then immediate >ref ;

: begin immediate ref< ;
: again immediate compile, branch <ref ;
: until immediate compile, 0branch <ref ;
: while immediate compile, 0branch ref> ;
: repeat immediate swap compile, branch <ref >ref ;


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

// Memory Access Helpers
// ---------------------

: +! swap over @ + swap ! ;


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

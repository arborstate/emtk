// Dictionary Helpers
// ------------------
: is-immediate? cell + c@ 1 & ;

: nt>xt cell + 1 + dup c@ + 1 + aligned ;
: xt>cfa @ ;
: xt>pf cell + ;
: cells cell * ;

: ' parse-name find-nt nt>xt ;
: ['] immediate lit [ ' lit , ] , ' , ;
: postpone immediate ['] lit , ' , ['] , , ;

: here dp @ ;
: allot dp @ + dp ! ;

: abort restart ;


// Conditionals And Looping Constructs
// -----------------------------------

: ref< here ;
: <ref here - , ;

: ref> here 0 , ;
: >ref here over - swap ! ;

: if immediate postpone 0branch ref> ;
: else immediate postpone branch ref> swap >ref ;
: then immediate >ref ;

: begin immediate ref< ;
: again immediate postpone branch <ref ;
: until immediate postpone 0branch <ref ;
: while immediate postpone 0branch ref> ;
: repeat immediate swap postpone branch <ref >ref ;


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


// Complex Word Defining
// ---------------------

: docol, ['] docol xt>cfa , ;
: create header docol, postpone rel ref> postpone exit postpone exit >ref latest @ link ! ;
: xt>here xt>pf cell + dup @ + ;
: (does) latest @ nt>xt xt>pf 2 cells + ! ;
: does> immediate postpone rel ref> postpone (does) postpone exit >ref docol, ;

: constant create , does> @ ;
: variable create 0 , ;

: defer create postpone exit does> @ execute ;
: deferaddr xt>here ;
: is ' deferaddr ! ;
: defer! deferaddr ! ;
: defer@ deferaddr @ ;

: marker create link @ , here cell + , does> dup @ link ! cell + @ dp ! ;

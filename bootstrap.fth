// Let's start at the beginning...

// Word Defining / Dictionary Access
// ---------------------------------
dp @ link @ , 0 c, parse-name (header) ", align
docol, ] dp @ link @ , 0 c, parse-name ", align link ! exit [ link !

(header) : docol, ] (header) docol, ] exit [
: nt>xt cell + 1 + dup c@ + 1 + aligned exit [
: ' parse-name find-nt nt>xt exit [

: ; [ ' [ , ] lit exit , exit [ 1 link @ cell + c!

: immediate 1 link @ cell + c! ;
: is-immediate? cell + c@ 1 & ;

: here dp @ ;
: allot dp @ + dp ! ;

: xt>cfa @ ;
: xt>pf cell + ;
: cells cell * ;

(header) latestxt ' dovar xt>cfa , 0 ,
: header (header) here latestxt ! ;
: : header docol, ] ;

: ['] lit [ ' lit , ] , ' , ; immediate
: compile, ['] lit , ' , ['] , , ; immediate


// Helpers
// -------

: hex 16 base ! ;
: decimal 10 base ! ;

: true 1 ;
: false 0 ;

: = - 0= ;
: / /mod swap drop ;
: min 2dup > if swap then drop ;


// Complex Word Defining
// ---------------------

: create header docol, compile, lit here 0 , compile, exit compile, exit here swap ! ;
: (does) latestxt @ xt>pf 2 cells + ! ;
: does> compile, lit here 0 , compile, (does) compile, exit here swap ! docol, ; immediate

: constant create , does> @ ;
: variable create 0 , ;

: defer create compile, exit does> @ execute ;
: deferaddr xt>pf cell + @ ;
: is ' deferaddr ! ;
: defer! deferaddr ! ;
: defer@ deferaddr @ ;


// Conditionals And Looping Constructs
// -----------------------------------
: if compile, 0branch here 0 , ; immediate
: else compile, branch here 0 , swap here swap ! ; immediate
: then here swap ! ; immediate

: begin here ; immediate
: again compile, branch , ; immediate
: until compile, 0branch , ; immediate
: while compile, 0branch here 0 , ; immediate
: repeat swap compile, branch , here swap ! ; immediate


// Extended Stack Manipulation
// ---------------------------
: ?dup dup if dup then ;
: 2dup over over ;
: 2drop drop drop ;

: -rot swap >r swap r> ;
: rot >r swap r> swap ;


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

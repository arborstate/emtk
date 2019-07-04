here link @ , 0 c, parse-name header ", align
docol, ] here link @ , 0 c, parse-name ", align link ! exit [ link !

header : docol, ] header docol, ] exit [

: ' parse-name findxt drop exit [
: immediate 1 link @ cell + c! exit [

: ; [ ' [ , ] lit [ ' exit , ] , exit [ immediate

: xt>cfa @ ;

: constant header lit [ ' docon xt>cfa , ] , , ;
: create header lit [ ' dovar xt>cfa , ] , ;
: variable create 0 , ;

: allot dp @ + dp ! ;

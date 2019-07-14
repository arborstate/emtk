
variable old-ingest-number

: stash-byte over c! 1 + ;
: bytes[ dup ['] ingest-number defer@ old-ingest-number ! ['] stash-byte ['] ingest-number defer! ;
: ]bytes old-ingest-number @ ['] ingest-number defer! over - ;

: dump begin dup while over c@ . 1 - swap 1 + swap repeat drop drop ;

hex

: reverse 0 swap begin dup while >r 1 << over 1 & | swap 1 >> swap r> 1 - repeat drop swap drop ;

04C11DB7 20 reverse constant crc32-poly

: generate-table-val 8 begin dup while >r dup 1 & if 1 >> over ^ else 1 >> then r> 1 - repeat drop swap drop ;
: generate-table >r 0 begin dup 100 < while r@ over generate-table-val , 1 + repeat drop r> drop ;

create crc32-table crc32-poly generate-table

: c@+ dup c@ swap 1 + swap ;
: crc32 FFFFFFFF >r over + swap begin 2dup > while c@+ r@ FF & ^ cells crc32-table + @ r> 8 >> ^ >r repeat 2drop r> FFFFFFFF ^ ;

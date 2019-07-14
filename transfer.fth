
variable old-ingest-number

: bytes[ dup ['] ingest-number defer@ old-ingest-number ! ['] c!+ ['] ingest-number defer! ;
: ]bytes old-ingest-number @ ['] ingest-number defer! over - ;

: dump begin dup while over c@ . 1 - swap 1 + swap repeat drop drop ;

hex

: reverse 0 swap begin dup while >r 1 << over 1 & | swap 1 >> swap r> 1 - repeat drop swap drop ;

: generate-table-val 8 begin dup while >r dup 1 & if 1 >> over ^ else 1 >> then r> 1 - repeat drop swap drop ;
: generate-table >r 0 begin dup 100 < while r@ over generate-table-val , 1 + repeat drop r> drop ;

create crc32-table 04C11DB7 20 reverse generate-table
: crc32 FFFFFFFF >r over + swap begin 2dup > while c@+ r@ FF & ^ cells crc32-table + @ r> 8 >> ^ >r repeat 2drop r> FFFFFFFF ^ ;

create crc32c-table 1EDC6F41 20 reverse generate-table
: crc32c FFFFFFFF >r over + swap begin 2dup > while c@+ r@ FF & ^ cells crc32c-table + @ r> 8 >> ^ >r repeat 2drop r> FFFFFFFF ^ ;

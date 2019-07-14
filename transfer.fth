
variable old-ingest-number

: stash-byte over c! 1 + ;
: bytes[ dup ['] ingest-number defer@ old-ingest-number ! ['] stash-byte ['] ingest-number defer! ;
: ]bytes old-ingest-number @ ['] ingest-number defer! over - ;

: dump begin dup while over c@ . 1 - swap 1 + swap repeat drop drop ;

create buf 128 allot

buf bytes[ DE AD BE EF CA FE BA BE ]bytes

(sp SYSTEM_TABLE
    ^64 Signature
    ^32 Revision
    ^32 HeaderSize
    ^32 CRC32
    ^32 Reserved)
    
(df main [^64 pImageHandle; ^64 pSystemTable]
    (ret (rsea SYSTEM_TABLE Signature pSystemTable)))

(df printf_no_scam []
    (ret) # return 2 times to make sure I won't do too much work
    (ret))

# to make me not use 1000 syntaxes
# kinda inspired by efi docs
# sp					-> THIS_CASE
# func					-> ThisCase
# var					-> thisCase
# selem	(Struct element)		-> ThisCase
# vars meant to be used as addresses	-> prefix(p)
# aas (address as struct)		-> prefix(ps)
# vars / selems meant to be called	-> prefix(f)

(df xiao [^64 one; ^64 two; ^64 three; ^64 fourth; ^64 fifth]
    (dv ^64 diao)
    (ret diao)
    (ret one)
    (ret two))
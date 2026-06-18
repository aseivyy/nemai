(sp firstStruct
    ^64 first
    ^64 second
    ^64 third)

(sp secStruct
    ^8 first
    ^16 second
    ^32 third
    ^64 fourth)
    
(df main[^u mimi]
    (dv ^u miao)
    (dv ^u diao)
    (dv ^u xiao)

    (ds secStruct myStruct)

    (ase myStruct fourth 4)

    (ret (rse myStruct fourth)))

(df printf_no_scam[]
    (ret)
    (ret))

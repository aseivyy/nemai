(dv ^64 i)
(dv ^32 j)

(df ^32 main []
    (if (== xina me)
    	(= i 1))
    (fi
	(= i 2))
    (ret 0)
)
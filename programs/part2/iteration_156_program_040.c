// Looking for this pattern:
cc_reg = compare((reg - 1), 0)

// Which in RTL might look like:
(set (reg:CC cc_reg)
     (compare (plus:SI (reg:SI reg_orig)
                       (const_int -1))
              (const_int 0)))

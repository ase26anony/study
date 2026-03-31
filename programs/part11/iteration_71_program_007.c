Before:                    After:
  jump L1                   jump L1
  nop (delay slot)          add r1, r2, r3  ; moved from L1
  ...                       ...
L1:                        L1:
  add r1, r2, r3            ...  ; instruction removed from here
  ...

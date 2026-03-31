// This RTL pattern:
(set (cc_reg) (compare (plus (reg) -1) 0))

// Which corresponds to C code like:
if (--reg == 0)
// or
while (--reg != 0)

// SET operation where:
cc_reg = COMPARE((reg + -1), 0)
// Which is equivalent to:
cc_reg = (reg - 1) < 0?  // or similar comparison

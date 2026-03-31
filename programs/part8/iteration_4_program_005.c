   cmp_orig = XVECEXP (PATTERN (prev_insn), 0, 0);
   if (GET_CODE (cmp_orig) != SET) return 0;

rtx *loc = DF_REF_REAL_LOC (ref);
/* Use obstack allocation instead of alloca for longer lifetime */
rtx new_reg = gen_raw_REG (GET_MODE (*loc), 
                          LAST_VIRTUAL_REGISTER + 1);
/* Or if gen_raw_REG isn't available: */
rtx new_reg = gen_rtx_REG (GET_MODE (*loc), 
                          ++virtual_reg_counter);

validate_change (insn, loc, new_reg, 1);

rtx *loc = DF_REF_REAL_LOC (ref);
rtx new_reg = gen_rtx_REG (GET_MODE (*loc), 
                           LAST_VIRTUAL_REGISTER + 1);
validate_change (insn, loc, new_reg, 1);

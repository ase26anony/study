rtx *loc = DF_REF_REAL_LOC (ref);
machine_mode mode = GET_MODE (*loc);
rtx new_reg = gen_rtx_REG (mode, LAST_VIRTUAL_REGISTER + 1 + mode);
validate_change (insn, loc, new_reg, 1);

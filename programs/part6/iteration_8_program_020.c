rtx new_reg = gen_reg_rtx (GET_MODE (*loc));
validate_change (insn, loc, new_reg, 1);

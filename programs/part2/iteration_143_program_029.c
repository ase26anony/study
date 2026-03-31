rtx *loc = DF_REF_REAL_LOC (ref);
machine_mode mode = GET_MODE (*loc);
rtx new_reg = gen_reg_rtx (mode);

/* If you need to set specific register number (virtual register) */
if (REGNO (new_reg) != LAST_VIRTUAL_REGISTER + 1 + (unsigned int)mode) {
    /* Create a new register with specific number */
    new_reg = gen_rtx_REG (mode, LAST_VIRTUAL_REGISTER + 1 + (unsigned int)mode);
}

validate_change (insn, loc, new_reg, 1);

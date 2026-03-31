rtx *loc = DF_REF_REAL_LOC (ref);
/* Use ggc_alloc() for persistent RTL or obstack_alloc() for temporary but safe allocation */
rtx new_reg = gen_raw_REG (GET_MODE (*loc), 
                          LAST_VIRTUAL_REGISTER + 1 + (int)GET_MODE (*loc));
/* Or better: allocate a proper virtual register number */
int new_regno = get_new_virtual_regno (GET_MODE (*loc));
rtx new_reg = gen_raw_REG (GET_MODE (*loc), new_regno);
validate_change (insn, loc, new_reg, 1);

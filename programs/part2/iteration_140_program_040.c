// Could use a loop or variadic template approach if supported
// Or create a helper function that takes the ops array and count

static rtx
gen_function_call (enum insn_code icode, rtx *ops, int nargs)
{
  switch (nargs) {
    case 0: return GEN_FCN(icode)();
    case 1: return GEN_FCN(icode)(ops[0].value);
    case 2: return GEN_FCN(icode)(ops[0].value, ops[1].value);
    // ... etc up to the maximum needed
    case 10: return GEN_FCN(icode)(ops[0].value, ops[1].value, ops[2].value,
                                   ops[3].value, ops[4].value, ops[5].value,
                                   ops[6].value, ops[7].value, ops[8].value,
                                   ops[9].value);
    case 11: return GEN_FCN(icode)(ops[0].value, ops[1].value, ops[2].value,
                                   ops[3].value, ops[4].value, ops[5].value,
                                   ops[6].value, ops[7].value, ops[8].value,
                                   ops[9].value, ops[10].value);
    default: gcc_unreachable();
  }
}

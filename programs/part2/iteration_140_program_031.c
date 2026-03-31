static rtx
call_with_n_args (int n, rtx *values)
{
  switch (n) {
    case 0: return GEN_FCN(icode)();
    case 1: return GEN_FCN(icode)(values[0]);
    case 2: return GEN_FCN(icode)(values[0], values[1]);
    // ... cases up to the maximum needed
    default: gcc_unreachable();
  }
}

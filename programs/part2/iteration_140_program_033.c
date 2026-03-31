// Alternative approach using a helper function
static rtx
call_with_n_args (rtx (*fcn)(), rtx *args, int n)
{
  switch (n)
    {
    case 0: return fcn();
    case 1: return fcn(args[0]);
    case 2: return fcn(args[0], args[1]);
    // ... cases up to maximum needed
    default: gcc_unreachable();
    }
}

// Then in the switch:
return call_with_n_args(GEN_FCN(icode), ops, n);

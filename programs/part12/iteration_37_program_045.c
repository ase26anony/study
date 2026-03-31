// Instead of hardcoding each case, you could use a loop or variadic approach
// Example using a switch with a helper function:
static rtx
gen_function_call (int nargs, rtx *ops, rtx (*fcn)())
{
  switch (nargs) {
    case 0: return fcn();
    case 1: return fcn(ops[0].value);
    case 2: return fcn(ops[0].value, ops[1].value);
    // ... etc
    // Or better yet, use a more dynamic approach
  }
}

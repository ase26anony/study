// Example of a variadic approach if supported
return call_with_args(icode, ops, num_args);

// Or using a macro for code generation
#define CALL_WITH_N_ARGS(n) \
  case n: \
    return GEN_FCN(icode)(ops[0].value, ops[1].value /* ... */ ops[n-1].value);

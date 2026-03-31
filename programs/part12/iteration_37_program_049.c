/* Generate a function call with N arguments */
#define GEN_CALL(N) \
  case N: \
    return GEN_FCN(icode)( \
      ops[0].value, ops[1].value, ops[2].value, \
      ops[3].value, ops[4].value, ops[5].value, \
      ops[6].value, ops[7].value, ops[8].value, \
      ops[9].value ops_10_to_N(N) \
    );

/* Helper to generate arguments 10 to N-1 */
#define ops_10_to_N(N) \
  ops_10_to_N_impl(N)
  
#define ops_10_to_N_impl(N) \
  ops_10_to_N_##N

/* Define for each N */
#define ops_10_to_N_10  /* empty for 10 args */
#define ops_10_to_N_11 , ops[10].value
#define ops_10_to_N_12 , ops[10].value, ops[11].value
/* ... and so on */

// Could use variadic templates or macros to generate these cases
#define CALL_GEN_FCN(n) \
  case n: \
    return GEN_FCN(icode)(ops[0].value, ops[1].value, ops[2].value, \
                          ops[3].value, ops[4].value, ops[5].value, \
                          ops[6].value, ops[7].value, ops[8].value, \
                          ops[9].value ops_10_to_n(n));

// Then in switch:
CALL_GEN_FCN(10)
CALL_GEN_FCN(11)

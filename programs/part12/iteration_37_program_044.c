// Helper macro to generate argument list
#define GEN_ARGS(n) ops[0].value, ops[1].value, ops[2].value, \
                    ops[3].value, ops[4].value, ops[5].value, \
                    ops[6].value, ops[7].value, ops[8].value, \
                    ops[9].value GEN_MORE_ARGS(n)

#define GEN_MORE_ARGS(n) \
    GEN_MORE_ARGS_##n

#define GEN_MORE_ARGS_10  // No additional args for case 10
#define GEN_MORE_ARGS_11 , ops[10].value
#define GEN_MORE_ARGS_12 , ops[10].value, ops[11].value
// ... and so on

// Then in the switch:
case 10:
    return GEN_FCN(icode)(GEN_ARGS(10));
case 11:
    return GEN_FCN(icode)(GEN_ARGS(11));

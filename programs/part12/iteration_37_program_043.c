case 0: return GEN_FCN(icode)();
case 1: return GEN_FCN(icode)(ops[0].value);
case 2: return GEN_FCN(icode)(ops[0].value, ops[1].value);
// ... etc.

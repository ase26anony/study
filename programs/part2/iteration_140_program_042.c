switch (num_args) {
  case 0: return GEN_FCN(icode)();
  case 1: return GEN_FCN(icode)(ops[0].value);
  case 2: return GEN_FCN(icode)(ops[0].value, ops[1].value);
  // ... cases 3-9 ...
  case 10: /* shown above */
  case 11: /* shown above */
  // ... more cases as needed
}

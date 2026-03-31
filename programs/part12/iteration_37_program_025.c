/* Assuming ops_count is available */
return GEN_FCN(icode)(
    ops[0].value, ops[1].value, ops[2].value,
    ops[3].value, ops[4].value, ops[5].value,
    ops[6].value, ops[7].value, ops[8].value,
    ops[9].value, 
    (ops_count > 10) ? ops[10].value : NULL,
    (ops_count > 11) ? ops[11].value : NULL,
    /* ... continue for maximum expected args */
);

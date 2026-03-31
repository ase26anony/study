if ( (x = 3) > 0 && x > 2 ) // UB: x is read and modified in same expression without sequence point between reads/writes in '&&'? Actually, here it's OK because && has a sequence point after left operand. But consider:
if ( x++ > 0 ) // OK: x is read (old value), then incremented after the read (sequence point after full expression).

int x = 5;
if (x++ > 0) {   // x is evaluated and incremented after?
    x = 10;      // Modifying again — order and intent become unclear.
}

int x = 5;
if (x++ > 5) { ... }  // Modifying and reading without intervening sequence point in the same expression? 
// Actually x++ > 5 is fine because x++ returns old value, but side-effect to x is sequenced after value computation.
// A worse example:
if (x = x + 1) { ... } // This is fine too — assignment returns new value, but x is modified and read in same expression, still sequenced.

// Original code might look like:
int x = (some_condition) ? 1 : 0;
if (x == 1) { ... }

// The phi node would be the definition of 'x'
// The condition checks if this phi result equals 1

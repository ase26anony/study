int val = *p;  // Dereference pointer p to get the value at current position
p++;           // Increment pointer to point to next integer
sum += val;    // Accumulate sum (prevents compiler optimization)

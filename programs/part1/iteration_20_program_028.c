// Pattern being detected:
x = phi(a, b);  // PHI node
y = x;          // Simple copy (skipped in while loop)
if (y == 0)     // Conditional with constant 0/1
  // ...

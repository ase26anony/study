// Example code pattern this might optimize:
x = phi(0, 1);  // phi node
if (x == 1)     // conditional comparing phi result to constant
  // branch 1
else
  // branch 2

// Possible optimization without barrier
int a = 1, b = 2, c = 3;
c = (b + 1) * 2;  // Combine a calculation
a = b + 1;
b = c - a;

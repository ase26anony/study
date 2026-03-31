// Possible optimization without barrier
volatile int a = 1, b = 2, c = 3;
int temp_b = b;      // Read b once
a = temp_b + 1;      // Use cached value
c = a * 2;           // a is already in register
b = c - a;           // Both c and a in registers

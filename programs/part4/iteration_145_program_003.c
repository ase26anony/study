// Possible reordering without barrier:
a = b + 1;
c = a * 2;  // Could be moved before the barrier
asm volatile("" ::: "memory");
b = c - a;

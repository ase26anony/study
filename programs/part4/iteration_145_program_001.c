// Possible reordering (optimized):
c = a * 2;      // Moved up (but can't due to RAW on a)
a = b + 1;
b = c - a;

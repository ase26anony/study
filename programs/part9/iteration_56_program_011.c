// Current issues:
// 1. Variables f, h, j, k, l, n, o, q, r, t are declared but not initialized
// 2. Using uninitialized variables (e ^= f) is undefined behavior
// 3. Early return could make later code dead (but asm prevents elimination)

// To fix undefined behavior, add initialization:
int a = trigger, b = a + 1, c = b * 2, d = c - a, e = d / 3;
int f = 1, g = 2, h = 3, i = 4, j = 5, k = 6, l = 7;
int m = 8, n = 9, o = 10, p = 11, q = 12, r = 13, s = 14, t = 15;

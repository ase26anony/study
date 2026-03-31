// Original: Two separate array accesses
int t1 = arr[i * 2] * 3;
int t2 = arr[i * 2 + 1] * 5;

// Could be optimized to:
int idx = i * 2;
int val1 = arr[idx];
int val2 = arr[idx + 1];
int t1 = val1 * 3;
int t2 = val2 * 5;

// Instead of computing i*2 and i*2+1 each time
int *p = arr;
for (i = 0; i < n; i++) {
    int t1 = *p * 3;  // arr[i*2]
    p++;
    int t2 = *p * 5;  // arr[i*2+1]
    p++;
    // ... rest of computation
}

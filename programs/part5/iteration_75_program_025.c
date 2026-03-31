int *ptr = arr;
for (i = 0; i < n; i++) {
    int t1 = *ptr * 3;  // arr[i*2]
    ptr++;
    int t2 = *ptr * 5;  // arr[i*2+1]
    ptr++;
    // ... rest of loop
}

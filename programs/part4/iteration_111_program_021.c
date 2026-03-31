int t = cond;  // Initial value
for (int i = 0; i < 100; i++) {
    if (t > 0) {
        arr[i] = i;
        t = i;  // t changes for next iteration
    }
}

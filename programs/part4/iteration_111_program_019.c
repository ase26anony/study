// WRONG optimization (what compiler might incorrectly do):
int temp = cond > 0;
for (int i = 0; i < 100; i++) {
    if (temp) {  // Using cached value
        arr[i] = i;
        cond = i;  // This modification doesn't affect the cached condition!
    }
}

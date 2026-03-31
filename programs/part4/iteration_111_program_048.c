int arr[100];
int cond = some_value();
if (cond > 0) {
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;  // This assignment is now redundant
    }
}
// If cond <= 0, the loop does nothing

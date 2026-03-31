int arr[100];
int cond = some_value();
// Precompute condition results or restructure logic
if (cond > 0) {
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
} else {
    for (int i = 0; i < 100; i++) {
        // Handle else case
    }
}

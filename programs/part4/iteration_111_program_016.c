int arr[100];
int cond = some_value();
if (cond > 0) {
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;  // This is now just the last value
    }
} else {
    for (int i = 0; i < 100; i++) {
        // arr[i] remains uninitialized or set to some default
    }
}

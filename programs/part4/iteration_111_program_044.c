int arr[100];
int cond = some_value();
int temp = cond;  // Use temp for the test
for (int i = 0; i < 100; i++) {
    if (temp > 0) {
        arr[i] = i;
        cond = i;  // Modify original cond if needed
    }
}

int arr[100];
int cond = some_value();
if (cond > 0) {
    arr[0] = 0;
    cond = 0;
}
// The rest of the loop iterations do nothing since cond = 0
for (int i = 1; i < 100; i++) {
    // Empty - condition is always false
}

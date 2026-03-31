int arr[100];
int cond = some_value();
for (int i = 0; i < 100; i++) {
    if (cond > 0) {  // Could be hoisted - cond never changes
        arr[i] = i;
        // No modification of cond here
    }
}

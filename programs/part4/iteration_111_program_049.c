int arr[100];
int cond = some_value();
int should_execute = (cond > 0);  // Evaluate once outside loop
for (int i = 0; i < 100; i++) {
    if (should_execute) {  // Now truly invariant
        arr[i] = i;
        cond = i;  // Can still modify original cond if needed
    }
}

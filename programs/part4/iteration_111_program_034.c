int arr[100];
int cond = some_value();
if (cond > 0) {  // WRONG: hoisted outside loop
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
        cond = i;
    }
}

int arr[100];
int cond = some_value();
for (int i = 0; i < 100; i++) {
    arr[i] = 0;  // Uninitialized elements would be 0 anyway
}
if (cond > 0) {
    arr[0] = 0;  // This is redundant since arr[0] would be 0 anyway
    cond = 0;
}

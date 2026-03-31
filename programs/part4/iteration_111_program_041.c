int arr[100];
int cond = some_value();
for (int i = 0; i < 100 && cond > 0; i++) {
    arr[i] = i;
    cond = i;
}

int arr[100];
int *p = arr;
for (int i = 0; i < 100; i++) {
    int val = *p++;  // Combined load and increment
    sum += val;
}

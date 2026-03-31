int arr[100];
// Initialize array
for (int i = 0; i < 100; i++) {
    arr[i] = i;  // or some meaningful values
}

int sum = 0;
int *p = arr;
for (int i = 0; i < 100; i++) {
    int val = *p;
    p++;
    sum += val;
}

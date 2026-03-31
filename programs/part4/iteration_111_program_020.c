int arr[100];
int cond = some_value();
if (cond > 0) {
    arr[0] = 0;
    cond = 0;
}
// Rest of arr remains uninitialized (contains garbage values)

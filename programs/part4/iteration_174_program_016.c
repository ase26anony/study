int shift = 0;
for (int j = 0; j < 10; ++j) {
    arr[j] = (expensive >> shift) & 0xF;
    sum += arr[j] * (i + j);
    // ...
    shift += 4;
}

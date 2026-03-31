// Pointer-based version (often generates same assembly)
int* ptr = data;
int* end = data + N;
while (ptr < end) {
    sum += *ptr++;
}

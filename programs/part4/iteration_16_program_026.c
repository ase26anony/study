// Unrolled version (reduces loop overhead)
for (i = 0; i < N-3; i += 4) {
    sum += data[i];
    sum += data[i+1];
    sum += data[i+2];
    sum += data[i+3];
}
// Handle remaining elements
for (; i < N; i++) {
    sum += data[i];
}

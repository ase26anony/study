int main() {
    const int n = 1024;
    int *a = aligned_alloc(64, n * sizeof(int));
    // ... allocate and initialize b, c, d, out1, out2
    
    int result = process_comparisons(a, b, c, d, out1, out2, n);
    
    // ... use results
}

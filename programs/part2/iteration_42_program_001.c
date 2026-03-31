#pragma omp simd
for (int i = 0; i < n; ++i) {
    // Branchless version of: if (a[i] > b[i]) out1[i] = a[i] - b[i]; else out1[i] = 0;
    int condition = a[i] > b[i];
    out1[i] = condition * (a[i] - b[i]);
    
    // The ternary operator typically compiles well
    out2[i] = (c[i] >= d[i]) ? (c[i] & 0xFF) : (d[i] & 0xFF);
    
    sum += out1[i] + out2[i];
}

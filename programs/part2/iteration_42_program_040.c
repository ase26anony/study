// Compiler might auto-vectorize this too with -O3 -march=native
for (int i = 0; i < n; ++i) {
    out1[i] = a[i] > b[i] ? a[i] - b[i] : 0;
    out2[i] = (c[i] >= d[i]) ? (c[i] & 0xFF) : (d[i] & 0xFF);
    sum += out1[i] + out2[i];
}

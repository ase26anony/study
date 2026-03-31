int x1 = 0;
for (int i = 0; i < N; i++) {
    if (i % 2 == 0) {
        x2 = 1;
    } else {
        x3 = 0;
    }
    x4 = φ(x2, x3);  // Phi node merging both branches
    if (x4 == 0) {   // x4 is NOT constant - depends on phi node
        // ...
    }
}

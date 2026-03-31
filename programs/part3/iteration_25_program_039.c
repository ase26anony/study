int x0 = 0;
for (int i = 0; i < N; i++) {
    if (i % 2 == 0) {
        x1 = 1;
    } else {
        x2 = 0;
    }
    x3 = φ(x1, x2);  // Phi node merges x1 and x2
    if (x3 == 0) {   // x3 is NOT constant - depends on phi node
        // ...
    }
}

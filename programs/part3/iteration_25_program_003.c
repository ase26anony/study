int x = 0;
for (int i = 0; i < N; i++) {
    if (i % 2 == 0) {
        x = 1;  // Creates phi node for x
    } else {
        x = 0;
    }
    if (i % 2 != 0) {  // Optimized: replaced x == 0 with i % 2 != 0
        // ...
    }
}

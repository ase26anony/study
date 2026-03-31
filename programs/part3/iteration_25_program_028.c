int x = 0;
for (int i = 0; i < N; i++) {
    if (i % 2 == 0) {
        x = 1;  // Still needed if x is used elsewhere
    } else {
        x = 0;
    }
    if (i % 2 != 0) {  // Optimized: direct check instead of using x
        // ...
    }
}

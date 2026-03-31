for (int i = 0; i < N; i++) {
    if (i % 2 != 0) {  // Direct check, no phi node needed
        // ... (odd iteration code)
    }
    // Even iterations skip the block entirely
}

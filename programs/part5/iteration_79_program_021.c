for (int i = 0; i < N; i++) {
    // Execute shared blocks once, then decide which loops to run
    shared_block_1();
    shared_block_2();
    
    if (condition_a && condition_b) {
        // Both conditions true - merge loops if possible
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    } else if (condition_a) {
        // Only condition_a true
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    } else if (condition_b) {
        // Only condition_b true
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
    // shared blocks already executed at the beginning
}

for (int i = 0; i < N; i++) {
    shared_block_1(); // Executed once per outer iteration
    
    if (condition_a) {
        for (int j = 0; j < M; j++) {
            // shared_block_1(); // REMOVED - already executed above
            unique_to_A();
        }
    }
    
    shared_block_2();
    
    if (condition_b) {
        for (int k = 0; k < P; k++) {
            // shared_block_2(); // REMOVED - already executed above
            unique_to_B();
        }
    }
}

for (int i = 0; i < N; i++) {
    // Execute shared_block_1 once per outer iteration
    shared_block_1();
    
    if (condition_a) {
        // shared_block_1() already executed, so don't repeat in inner loop
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    }
    
    // Execute shared_block_2 once per outer iteration
    shared_block_2();
    
    if (condition_b) {
        // shared_block_2() already executed, so don't repeat in inner loop
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

for (int i = 0; i < N; i++) {
    // Execute shared_block_1() once per iteration
    shared_block_1();
    
    if (condition_a) {
        for (int j = 0; j < M; j++) {
            // shared_block_1() removed from here
            unique_to_A();
        }
    }
    
    // Execute shared_block_2() once per iteration  
    shared_block_2();
    
    if (condition_b) {
        for (int k = 0; k < P; k++) {
            // shared_block_2() removed from here
            unique_to_B();
        }
    }
}

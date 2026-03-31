for (int i = 0; i < N; i++) {
    // Execute shared blocks once, then decide which inner loops to run
    shared_block_1();
    
    if (condition_a) {
        // Execute shared_block_1's effect once, then run unique part in loop
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    }
    
    shared_block_2();
    
    if (condition_b) {
        // Execute shared_block_2's effect once, then run unique part in loop
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

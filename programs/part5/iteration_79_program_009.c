for (int i = 0; i < N; i++) {
    // Execute shared_block_1 once per outer iteration
    shared_block_1();
    
    // Execute L2 iterations if condition_a is true
    if (condition_a) {
        for (int j = 0; j < M; j++) {
            unique_to_A();  // Only unique code remains
        }
    }
    
    // Execute shared_block_2 once per outer iteration
    shared_block_2();
    
    // Execute L3 iterations if condition_b is true
    if (condition_b) {
        for (int k = 0; k < P; k++) {
            unique_to_B();  // Only unique code remains
        }
    }
}

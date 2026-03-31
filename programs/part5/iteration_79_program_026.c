for (int i = 0; i < N; i++) {
    // Execute shared_block_1() once instead of potentially M+1 times
    shared_block_1();
    
    if (condition_a) {
        // Move shared_block_1() out of the inner loop
        for (int j = 0; j < M; j++) {
            unique_to_A();  // Only unique code remains
        }
    }
    
    // Similarly for shared_block_2()
    shared_block_2();
    
    if (condition_b) {
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

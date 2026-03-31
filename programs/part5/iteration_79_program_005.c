for (int i = 0; i < N; i++) {
    // Calculate once per outer iteration
    result1 = shared_block_1();
    result2 = shared_block_2();
    
    if (condition_a) {
        for (int j = 0; j < M; j++) {
            use_result1(result1);  // Use precomputed result
            unique_to_A();
        }
    }
    
    if (condition_b) {
        for (int k = 0; k < P; k++) {
            use_result2(result2);  // Use precomputed result
            unique_to_B();
        }
    }
}

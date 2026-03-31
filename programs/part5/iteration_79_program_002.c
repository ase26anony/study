for (int i = 0; i < N; i++) {
    shared_block_1(); // Executed once per outer iteration
    
    if (condition_a) {
        // Move shared_block_1() out of the inner loop
        for (int j = 0; j < M; j++) {
            unique_to_A(); // Only unique code remains
        }
    }
    
    shared_block_2();
    
    if (condition_b) {
        // Move shared_block_2() out of the inner loop  
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

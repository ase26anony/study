for (int i = 0; i < N; i++) {
    shared_block_1();
    
    if (condition_a) {
        // Execute shared_block_1() once before the loop
        for (int j = 0; j < M; j++) {
            unique_to_A();  // Only unique code in loop
        }
    }
    
    shared_block_2();
    
    if (condition_b) {
        // Execute shared_block_2() once before the loop  
        for (int k = 0; k < P; k++) {
            unique_to_B();  // Only unique code in loop
        }
    }
}

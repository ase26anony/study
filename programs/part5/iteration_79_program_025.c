for (int i = 0; i < N; i++) {
    // Execute shared_block_1() only once per outer iteration
    shared_block_1();
    
    if (condition_a) {
        // Move unique_to_A() inside the loop, keep shared_block_1() outside
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    }
    
    // Execute shared_block_2() only once per outer iteration
    shared_block_2();
    
    if (condition_b) {
        // Move unique_to_B() inside the loop, keep shared_block_2() outside
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

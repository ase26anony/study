for (int i = 0; i < N; i++) {
    // Execute shared blocks only once per iteration
    shared_block_1();
    shared_block_2();
    
    // Handle inner loops based on conditions
    if (condition_a) {
        // Move unique_to_A() inside, keep shared_block_1() outside
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    }
    
    if (condition_b) {
        // Move unique_to_B() inside, keep shared_block_2() outside
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

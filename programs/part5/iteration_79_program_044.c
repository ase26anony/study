for (int i = 0; i < N; i++) {
    if (condition_a) {
        shared_block_1(); // Execute once per inner loop
        for (int j = 0; j < M; j++) {
            // shared_block_1() removed from here
            unique_to_A();
        }
    } else {
        shared_block_1(); // Only if needed when condition_a is false
    }
    
    if (condition_b) {
        shared_block_2(); // Execute once per inner loop
        for (int k = 0; k < P; k++) {
            // shared_block_2() removed from here
            unique_to_B();
        }
    } else {
        shared_block_2(); // Only if needed when condition_b is false
    }
}

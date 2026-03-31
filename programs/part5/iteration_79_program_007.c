for (int i = 0; i < N; i++) {
    if (condition_a) {
        for (int j = 0; j < M; j++) {
            shared_block_1();  // Now only called here
            unique_to_A();
        }
    } else {
        shared_block_1();  // Execute once if condition_a is false
    }
    
    if (condition_b) {
        for (int k = 0; k < P; k++) {
            shared_block_2();  // Now only called here
            unique_to_B();
        }
    } else {
        shared_block_2();  // Execute once if condition_b is false
    }
}

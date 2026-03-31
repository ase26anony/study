for (int i = 0; i < N; i++) {
    if (condition_a) {
        shared_block_1();  // Execute once before loop A
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    } else {
        shared_block_1();  // Still need to execute if condition_a is false
    }
    
    if (condition_b) {
        shared_block_2();  // Execute once before loop B
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    } else {
        shared_block_2();  // Still need to execute if condition_b is false
    }
}

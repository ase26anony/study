for (int i = 0; i < N; i++) {
    if (condition_a) {
        shared_block_1();  // Execute once per outer iteration when condition_a is true
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    } else {
        shared_block_1();  // Still execute when condition_a is false
    }
    
    if (condition_b) {
        shared_block_2();  // Execute once per outer iteration when condition_b is true
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    } else {
        shared_block_2();  // Still execute when condition_b is false
    }
}

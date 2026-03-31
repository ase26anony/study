// If shared_block_1() and shared_block_2() don't depend on i, j, or k
for (int i = 0; i < N; i++) {
    if (condition_a) {
        shared_block_1(); // Execute once per inner loop
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    } else {
        shared_block_1(); // Still execute if condition_a is false
    }
    
    if (condition_b) {
        shared_block_2();
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    } else {
        shared_block_2();
    }
}

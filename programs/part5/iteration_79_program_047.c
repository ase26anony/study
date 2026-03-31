for (int i = 0; i < N; i++) {
    if (condition_a) {
        shared_block_1(); // Execute only once per iteration
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    }
    
    if (condition_b) {
        shared_block_2(); // Execute only once per iteration
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

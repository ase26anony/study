for (int i = 0; i < N; i++) {
    if (condition_a) {
        // Execute shared_block_1() once before the loop
        shared_block_1();
        for (int j = 0; j < M; j++) {
            if (j > 0) shared_block_1(); // Skip first iteration
            unique_to_A();
        }
    } else {
        // Only execute once if condition_a is false
        shared_block_1();
    }
    
    // Similar logic for condition_b and shared_block_2()
}

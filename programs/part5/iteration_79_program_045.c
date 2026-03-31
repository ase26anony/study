for (int i = 0; i < N; i++) {
    if (condition_a) {
        // Execute shared_block_1() M+1 times by adjusting loop bounds
        for (int j = 0; j < M + 1; j++) {
            shared_block_1();
            if (j < M) {  // Only execute unique_to_A() for first M iterations
                unique_to_A();
            }
        }
    } else {
        // condition_a is false, just execute shared_block_1() once
        shared_block_1();
    }
    
    // Similar optimization for shared_block_2()...
}

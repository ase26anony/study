for (int i = 0; i < N; i++) {
    // Execute shared_block_1() only once per iteration
    if (condition_a) {
        shared_block_1();  // Execute once before loop A
        for (int j = 0; j < M; j++) {
            unique_to_A();  // Only unique code in inner loop
        }
    } else {
        shared_block_1();  // Still execute if condition_a is false
    }
    
    // Execute shared_block_2() only once per iteration  
    if (condition_b) {
        shared_block_2();  // Execute once before loop B
        for (int k = 0; k < P; k++) {
            unique_to_B();  // Only unique code in inner loop
        }
    } else {
        shared_block_2();  // Still execute if condition_b is false
    }
}

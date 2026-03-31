// Precompute which iterations need inner loops
bool needs_A[N], needs_B[N];
for (int i = 0; i < N; i++) {
    needs_A[i] = condition_a;
    needs_B[i] = condition_b;
}

// Process all A-related work first for better locality
for (int i = 0; i < N; i++) {
    if (needs_A[i]) {
        shared_block_1();
        for (int j = 0; j < M; j++) {
            unique_to_A();
        }
    }
}

// Process all B-related work
for (int i = 0; i < N; i++) {
    if (needs_B[i]) {
        shared_block_2();
        for (int k = 0; k < P; k++) {
            unique_to_B();
        }
    }
}

// Process outer loop work that doesn't need inner loops
for (int i = 0; i < N; i++) {
    if (!needs_A[i]) shared_block_1();
    if (!needs_B[i]) shared_block_2();
}

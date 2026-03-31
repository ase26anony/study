#pragma acc parallel loop
for (int i = 0; i < N; i++) {
    #pragma acc loop
    for (int j = 0; j < M; j++) {
        /* computation */
    }
}

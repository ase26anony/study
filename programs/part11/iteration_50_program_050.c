#pragma acc kernels
{
    #pragma acc loop gang worker
    for (int i = 0; i < N; i++) {
        #pragma acc loop vector
        for (int j = 0; j < M; j++) {
            /* computation */
        }
    }
}

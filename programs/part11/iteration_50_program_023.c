#pragma acc kernels
{
    #pragma acc loop gang vector
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            /* computation */
        }
    }
}

#pragma acc kernels
{
    #pragma acc loop gang vector collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            /* computation */
        }
    }
}

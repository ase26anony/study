#pragma acc kernels copyin(data[0:N*M]) copyout(result[0:N*M])
{
    #pragma acc loop gang(num: N/32)  // Control number of gangs
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker vector_length(32)  // Control vector length
        for (int j = 0; j < M; j++) {
            /* computation */
        }
    }
}

#pragma omp task depend(inout: arr)
{
    #pragma omp target data map(tofrom: arr) depend(inout: arr)
    {
        // Offload computation to device
        #pragma omp target teams distribute parallel for
        for(int i = 0; i < N; i++) {
            arr[i] = ...;
        }
    }
}

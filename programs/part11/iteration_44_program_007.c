#pragma omp target data map(to: A) depend(in: A)
{
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        // Read A on device
    }
}

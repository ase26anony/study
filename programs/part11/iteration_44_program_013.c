#pragma omp task depend(out: arr)
{
    // Initialize arr
}

#pragma omp target data map(tofrom: arr) depend(inout: arr)
{
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        // Compute on arr
    }
}

#pragma omp task depend(in: arr)
{
    // Use computed arr
}

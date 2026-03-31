#pragma omp target data map(tofrom: arr[0:N])
{
    // Some target operations
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        arr[i] = ...;
    }
}

// Then a task that depends on arr being ready
#pragma omp task depend(inout: arr)
{
    /* Task that uses/modifies arr */
}

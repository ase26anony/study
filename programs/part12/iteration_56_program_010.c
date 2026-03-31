#pragma omp target data map(tofrom: arr[0:N])
{
    // Some target operations
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < N; i++) {
        arr[i] = ...;
    }
}

// Then a task that depends on the data being back from device
#pragma omp task depend(inout: arr)
{
    // This task executes after arr is mapped back from device
    /* ... */
}

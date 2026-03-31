#pragma omp task depend(inout: arr)
{
    #pragma omp target data map(tofrom: arr[0:N])
    {
        // Target computations here
    }
}

#pragma omp task depend(in: arr)  // Waits for previous task to complete
{
    // Task that depends on arr being updated
}

#pragma omp target data map(tofrom: arr[0:N])
{
    // Tasks inside target data region
    #pragma omp task depend(inout: arr)
    {
        /* Task that depends on arr */
    }
}

#pragma omp task depend(inout: arr)
{
    #pragma omp target data map(tofrom: arr[0:N])
    {
        // Target operations on arr
    }
}

#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(inout: arr[0]) // or depend(inout: arr)
    {
        // Task that depends on arr
    }
    
    #pragma omp taskwait
}

#pragma omp target data map(tofrom: arr[0:N])
{
    // Tasks that operate on the device data
    #pragma omp task depend(inout: arr)
    {
        // Task that uses/updates arr
    }
    
    #pragma omp task depend(in: arr)
    {
        // Task that only reads arr
    }
}

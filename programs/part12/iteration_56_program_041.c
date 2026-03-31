#pragma omp target data map(tofrom: arr[0:N])
{
    // Data is now on device
    
    #pragma omp task depend(in: arr)  // Task depends on arr being available
    {
        // Task that uses arr
        // Note: This task executes on host, not device
    }
    
    #pragma omp taskwait  // Wait for task to complete
}  // Data copied back from device

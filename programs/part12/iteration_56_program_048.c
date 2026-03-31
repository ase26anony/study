#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(inout: arr)
    {
        // Task that depends on arr being available
        #pragma omp target map(tofrom: arr[0:N])
        {
            // Offloaded computation
        }
    }
}

#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(inout: arr)
    { 
        /* Task that depends on arr being available */
        /* This task will wait for the target data region to complete */
    }
}

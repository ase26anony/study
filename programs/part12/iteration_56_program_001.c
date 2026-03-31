#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)
{
    #pragma omp task depend(in: arr)  // Task depends on arr being available
    {
        /* Task that reads from arr */
    }
}

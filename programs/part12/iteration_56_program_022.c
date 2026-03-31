#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)
{
    // Task waits for target data to complete
    #pragma omp task depend(in: arr)
    {
        /* Task code that reads arr */
    }
}

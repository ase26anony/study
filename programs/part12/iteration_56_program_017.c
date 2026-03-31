#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(inout: arr)
    {
        /* Task that reads and writes arr */
    }
}

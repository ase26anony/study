#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)
{
    #pragma omp task depend(in: arr)
    { 
        /* Task that reads from arr */
    }
}

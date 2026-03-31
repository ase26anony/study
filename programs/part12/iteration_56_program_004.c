#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(inout: arr)
    { 
        /* Task that reads and writes arr */
    }
    
    #pragma omp task depend(in: arr)
    {
        /* Task that only reads arr */
    }
    
    #pragma omp task depend(out: arr)
    {
        /* Task that only writes arr */
    }
}

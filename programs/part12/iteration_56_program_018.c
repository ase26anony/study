#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(inout: arr)
    { /* Task that uses arr */ }
    
    #pragma omp task depend(in: arr)
    { /* Task that reads arr */ }
}

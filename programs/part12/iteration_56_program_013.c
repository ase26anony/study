#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(in: arr)
    { /* Task that reads arr */ }
    
    #pragma omp task depend(out: arr) 
    { /* Task that writes arr */ }
}

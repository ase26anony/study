#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)
{
    #pragma omp task depend(in: arr)  // Correct syntax
    { /* Task that reads arr */ }
    
    #pragma omp task depend(out: arr) // Correct syntax
    { /* Task that writes arr */ }
}

#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)
{
    #pragma omp task depend(inout: arr)  // Consistent dependency type
    { /* ... */ }
}

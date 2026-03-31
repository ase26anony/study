#pragma omp task depend(inout: arr)
{
    #pragma omp target map(tofrom: arr[0:N])
    {
        // ... target computation ...
    }
}

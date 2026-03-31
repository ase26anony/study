int in;  // 'in' must be a variable

#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)
{
    #pragma omp task depend(update: in)  // 'in' is a variable here
    {
        /* Task that depends on variable 'in' being updated */
    }
}

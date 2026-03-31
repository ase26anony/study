#pragma omp task depend(inout: arr)
{
    // Task that modifies arr
}

#pragma omp target depend(inout: arr) map(tofrom: arr[0:N])
{
    // Target region that uses/modifies arr
}

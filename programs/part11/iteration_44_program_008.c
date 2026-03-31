#pragma omp task depend(inout: arr)
#pragma omp target data map(tofrom: arr) depend(inout: arr)
{
    // GPU computation using arr
}

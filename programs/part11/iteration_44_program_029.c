// Initialize data on host
#pragma omp target data map(to: arr) depend(in: arr)
{
    // arr is available on device
}

// Process data on device
#pragma omp target data map(tofrom: arr) depend(inout: arr)
{
    // arr is modified on device
}

// Finalize
#pragma omp target data map(from: arr) depend(out: arr)
{
    // arr results copied back to host
}

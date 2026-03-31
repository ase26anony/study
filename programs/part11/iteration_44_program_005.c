#pragma omp task depend(out: arr)
{
    // Initialize arr on host
}

#pragma omp target data map(to: arr) depend(in: arr)
{
    // Use arr on device (read-only)
}

#pragma omp target data map(tofrom: arr) depend(inout: arr)
{
    // Modify arr on device
}

#pragma omp target data depend(destroy: arr)
{
    // Clean up or reuse arr
}

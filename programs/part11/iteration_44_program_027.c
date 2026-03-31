#pragma omp task depend(out: arr)
{
    // Initialize arr on host
}

#pragma omp target data map(to: arr) depend(in: arr)
{
    // Offload computation that reads arr
}

#pragma omp target data map(tofrom: arr) depend(inout: arr)
{
    // Offload computation that modifies arr
}

#pragma omp target data depend(destroy: arr)
// Clean up dependency

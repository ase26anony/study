#pragma omp parallel
#pragma omp single
{
    // Create initial data on device
    #pragma omp target data map(to: arr)
    
    // Process data with dependencies
    #pragma omp task depend(inout: arr)
    #pragma omp target map(tofrom: arr)
    {
        // Kernel execution on device
    }
    
    // Final data retrieval
    #pragma omp task depend(out: arr)
    #pragma omp target data map(from: arr)
}

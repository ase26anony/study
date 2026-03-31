#pragma omp parallel
#pragma omp single
{
    #pragma omp task depend(out: arr)
    #pragma omp target data map(from: arr)
    {
        // Initialize arr on device
    }
    
    #pragma omp task depend(in: arr)
    #pragma omp target data map(to: arr)
    {
        // Read arr on device
    }
    
    #pragma omp task depend(inout: arr)
    #pragma omp target data map(tofrom: arr)
    {
        // Update arr on device
    }
}

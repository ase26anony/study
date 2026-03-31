#pragma omp parallel
#pragma omp single
{
    // Task 1: Initialize arr
    #pragma omp target data map(from: arr) depend(out: arr)
    {
        // Offload computation that initializes arr
    }
    
    // Task 2: Process arr (waits for Task 1)
    #pragma omp target data map(tofrom: arr) depend(inout: arr)
    {
        // Offload computation that modifies arr
    }
    
    // Task 3: Read arr (waits for Task 2)
    #pragma omp target data map(to: arr) depend(in: arr)
    {
        // Offload computation that reads arr
    }
    
    // Clean up dependency
    #pragma omp target data depend(destroy: arr)
}

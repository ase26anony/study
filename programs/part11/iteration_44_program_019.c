#pragma omp parallel
#pragma omp single
{
    // Task 1: Initialize arr on device
    #pragma omp target data map(to: arr) depend(in: arr)
    #pragma omp task
    {
        // Initialize arr on device
    }
    
    // Task 2: Process arr (depends on Task 1)
    #pragma omp target data map(tofrom: arr) depend(inout: arr)
    #pragma omp task
    {
        // Process arr on device
    }
    
    // Task 3: Destroy dependency
    #pragma omp target data depend(destroy: arr)
    #pragma omp task
    {
        // Cleanup
    }
}

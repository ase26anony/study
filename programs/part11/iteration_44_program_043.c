// Task 1: Initialize arr
#pragma omp target data map(from: arr) depend(out: arr)
{
    // Initialize arr on device
}

// Task 2: Process arr (waits for Task 1)
#pragma omp target data map(tofrom: arr) depend(inout: arr)
{
    // Process arr on device
}

// Task 3: Finalize (waits for Task 2)
#pragma omp target data map(from: arr) depend(in: arr)
{
    // Use arr results
}

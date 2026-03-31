#pragma omp parallel
#pragma omp single
{
    // Task 1: Initialize arr
    #pragma omp task depend(out: arr)
    initialize_array(arr);
    
    // Task 2: Process arr (waits for Task 1)
    #pragma omp target data map(tofrom: arr) depend(inout: arr)
    process_on_device(arr);
    
    // Task 3: Finalize (waits for Task 2)
    #pragma omp task depend(in: arr)
    finalize_array(arr);
}

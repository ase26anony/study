#pragma omp task depend(out: arr)
initialize_array(arr, N);  // Task 1: produces arr

#pragma omp target data map(to: arr) depend(in: arr)
{
    // Task 2: uses arr on device (waits for Task 1)
    #pragma omp target teams distribute parallel for
    for(int i = 0; i < N; i++) {
        // computation using arr
    }
}

#pragma omp target data map(from: arr) depend(out: arr)
{
    // Task 3: modifies arr on device (waits for Task 2)
    #pragma omp target teams distribute parallel for
    for(int i = 0; i < N; i++) {
        arr[i] = process(arr[i]);
    }
}

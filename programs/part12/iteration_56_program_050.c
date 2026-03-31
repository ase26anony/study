#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)
{
    // Some device computation that modifies arr
    #pragma omp target teams distribute parallel for
    for(int i = 0; i < N; i++) {
        arr[i] = compute_on_device(arr[i]);
    }
    
    // Now need arr back on host for CPU processing
    #pragma omp task depend(update: in)  // Copies arr device→host
    {
        // This task waits for arr to be copied from device
        process_on_host(arr, N);
    }
}

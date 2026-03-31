#pragma omp target data map(tofrom: data[0:N])
{
    // Update data on device
    #pragma omp target update to(data[0:N])
    
    // Task that depends on the update being complete
    #pragma omp task depend(update: in)
    {
        // Process the updated data
    }
}

#pragma omp target data map(tofrom: arr[0:N])
{
    // The target data region executes here
    // Data transfers happen at entry/exit of this region
}

// Then create a task that depends on arr being available
#pragma omp task depend(inout: arr)
{
    // Task that uses arr after target data region completes
}

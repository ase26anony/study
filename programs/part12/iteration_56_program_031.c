#pragma omp target data map(tofrom: arr[0:N])
{
    // Target data region executes here
    // Tasks inside cannot depend on the target data directive itself
}

// Then create a task that depends on arr being ready
#pragma omp task depend(inout: arr[0:N])
{
    /* Task code that uses arr */
}

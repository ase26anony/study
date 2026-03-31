#pragma omp target data map(tofrom: arr[0:N])
{
    #pragma omp task depend(inout: arr)
    {
        // This task both reads and writes to arr
        // Other tasks with depend(in: arr) will wait for this
        // This task will wait for previous tasks with depend(out: arr) or depend(inout: arr)
    }
}

#pragma omp task depend(inout: arr)
{
    // Task that modifies arr
}

#pragma omp task depend(in: arr)
{
    // Task that reads arr (depends on first task completing)
}

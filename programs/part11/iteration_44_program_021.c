#pragma omp parallel
#pragma omp single
{
    #pragma omp task depend(out: arr)
    initialize_array(arr);
    
    #pragma omp task depend(in: arr)
    process_array(arr);
    
    #pragma omp task depend(inout: arr)
    modify_array(arr);
}

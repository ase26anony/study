#pragma omp task depend(out: arr)
initialize_array(arr);

#pragma omp task depend(in: arr) depend(out: result)
process_array(arr, result);

#pragma omp task depend(in: result)
use_result(result);

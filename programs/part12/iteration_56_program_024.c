#pragma omp target data map(tofrom: arr[0:N]) depend(inout: arr)

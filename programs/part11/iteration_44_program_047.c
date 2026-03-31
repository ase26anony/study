#pragma omp task depend(out: arr)
initialize_array(arr);  // Task 1: produces arr

#pragma omp target data map(to: arr) depend(in: arr)
#pragma omp target teams distribute parallel for
for(...) {
    // Use arr on device
}

#pragma omp task depend(in: arr)
process_results(arr);  // Task 3: consumes arr

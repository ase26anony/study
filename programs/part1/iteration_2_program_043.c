/* Test program to trigger uncovered OpenMP clause pretty-printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define M 100

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Scan directives with inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_scan = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
        partial_sum += arr[i];
    }
    
    /* This should trigger OMP_CLAUSE_INCLUSIVE */
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(partial_sum)
        partial_sum += arr[i];
    }
    
    /* This should trigger OMP_CLAUSE_EXCLUSIVE */
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp scan exclusive(exclusive_scan)
        exclusive_scan += arr[i];
        arr[i] = exclusive_scan;
    }
    
    printf("Scan results: partial_sum = %f, exclusive_scan = %f\n", 
           partial_sum, exclusive_scan);
}

/* Function 2: Enter data with to mapper - triggers OMP_CLAUSE_ENTER with TO */
void test_enter_data(struct DataBlock *block) {
    /* This should trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO set */
    #pragma omp enter data map(to: block->values[0:N]) \
                           map(to: block->indices) \
                           map(to: block->result)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->values[i] * 2.0 + i;
        block->indices[i] = i % 10;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values[0:N])
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *a, double *b, double *c, int n) {
    double sum = 0.0;
    double last_val = 0.0;
    
    /* Complex reduction with lastprivate and linear - may generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for simd reduction(+:sum) lastprivate(last_val) linear(i:1) \
                collapse(2) schedule(dynamic)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            a[idx] = i + j * 0.1;
            b[idx] = i * j * 0.01;
            c[idx] = a[idx] + b[idx];
            sum += c[idx];
            if (j == M - 1) last_val = c[idx];
        }
    }
    
    /* Nested parallelism with conditionals - may generate _CONDTEMP_ */
    #pragma omp parallel if(n > 500) num_threads(4)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            double temp = 0.0;
            for (int j = 0; j < M; j++) {
                int idx = i * M + j;
                if (c[idx] > 50.0) {
                    temp += c[idx];
                }
            }
            #pragma omp atomic
            sum += temp;
        }
        
        /* Scan directive in nested context - may generate _SCANTEMP_ */
        double scan_temp = 0.0;
        #pragma omp for
        for (int i = 0; i < n; i++) {
            #pragma omp scan inclusive(scan_temp)
            scan_temp += i * 0.5;
        }
    }
    
    printf("Internal temporaries test: sum = %f, last_val = %f\n", sum, last_val);
}

/* Function 4: SIMD with complex conditionals */
void test_simd_conditionals(double *arr, int n) {
    #pragma omp simd reduction(+:arr[0:n]) \
                linear(i:1) aligned(arr:64)
    for (int i = 0; i < n; i++) {
        double x = i * 0.1;
        /* Complex conditional that might generate _CONDTEMP_ */
        if (x > 10.0 && x < 90.0) {
            arr[i] = x * x;
        } else if (x <= 10.0) {
            arr[i] = x;
        } else {
            arr[i] = 100.0 - x;
        }
    }
    
    /* Another scan in SIMD context */
    double scan_acc = 0.0;
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        #pragma omp scan exclusive(scan_acc)
        scan_acc += arr[i];
        arr[i] = scan_acc;
    }
}

/* Function 5: Target regions with complex data environment */
void test_target_regions(struct DataBlock *block) {
    double local_buf[N];
    
    /* Map entire structure with different clauses */
    #pragma omp target enter data map(to: block[0:1]) \
                                  map(alloc: local_buf[0:N])
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N]) \
                map(always, to: block->indices[0:N]) \
                reduction(+:block->result)
    for (int i = 0; i < N; i++) {
        block->values[i] = block->indices[i] * 3.14;
        block->result += block->values[i];
    }
    
    /* Complex reduction with scan */
    double scan_val = 0.0;
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N]) \
                reduction(+:scan_val)
    for (int i = 0; i < N; i++) {
        #pragma omp scan inclusive(scan_val)
        scan_val += block->values[i];
        block->values[i] = scan_val;
    }
    
    #pragma omp target exit data map(from: block->result) \
                                 map(release: local_buf[0:N])
}

int main() {
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Allocate test data */
    double *arr1 = (double*)malloc(N * M * sizeof(double));
    double *arr2 = (double*)malloc(N * M * sizeof(double));
    double *arr3 = (double*)malloc(N * M * sizeof(double));
    double *scan_arr = (double*)malloc(N * sizeof(double));
    
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!arr1 || !arr2 || !arr3 || !scan_arr || !block) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 1.5;
        block->indices[i] = i;
        block->result = 0.0;
        scan_arr[i] = 0.0;
    }
    
    /* Test 1: Scan clauses */
    printf("\n=== Testing scan clauses ===\n");
    test_scan_clauses(scan_arr, N);
    
    /* Test 2: Enter data with to mapper */
    printf("\n=== Testing enter data with to mapper ===\n");
    test_enter_data(block);
    
    /* Test 3: Internal temporaries */
    printf("\n=== Testing internal temporaries ===\n");
    test_internal_temporaries(arr1, arr2, arr3, N);
    
    /* Test 4: SIMD conditionals */
    printf("\n=== Testing SIMD conditionals ===\n");
    test_simd_conditionals(scan_arr, N);
    
    /* Test 5: Target regions */
    printf("\n=== Testing target regions ===\n");
    test_target_regions(block);
    
    /* Final verification */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += scan_arr[i] + block->values[i];
    }
    
    printf("\nFinal result: sum = %f, block->result = %f\n", 
           final_sum, block->result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(scan_arr);
    free(block);
    
    printf("Test completed successfully.\n");
    return 0;
}

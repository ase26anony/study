/* Test program to trigger uncovered pretty-printing of OpenMP clauses */
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

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 1.5;
        partial_sum += arr[i];
    }
    
    /* This should generate OMP_CLAUSE_INCLUSIVE and OMP_CLAUSE_EXCLUSIVE */
    #pragma omp parallel for reduction(+:exclusive_prefix)
    for (int i = 1; i < n; i++) {
        #pragma omp scan inclusive(exclusive_prefix)
        exclusive_prefix += arr[i-1];
        
        #pragma omp scan exclusive(partial_sum)
        arr[i] += partial_sum;
        
        partial_sum += arr[i];
    }
}

/* Function 2: Uses enter data with to clause */
void test_enter_data(struct DataBlock *block) {
    /* This should generate OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block[:1]) \
        map(to: block->values[:N]) \
        map(to: block->indices[:N])
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->result)
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 2.0;
        block->indices[i] = i;
        #pragma omp atomic
        block->result += block->values[i];
    }
    
    #pragma omp exit data map(from: block->result) \
        map(release: block->values[:N])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double tmp = 0.0;
    int last_val = 0;
    
    /* Complex reduction with lastprivate and linear - may generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:tmp) lastprivate(last_val) \
        linear(i:1) collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            a[i * M + j] = i + j * 0.5;
            b[i * M + j] = i - j * 0.5;
            tmp += a[i * M + j] * b[i * M + j];
            last_val = i * M + j;
        }
    }
    
    /* SIMD with conditionals - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:tmp) linear(i:1)
    for (int i = 0; i < n * M; i++) {
        if (a[i] > b[i]) {
            tmp += a[i] - b[i];
        } else {
            tmp += b[i] - a[i];
        }
        /* Complex expression that might need temporaries */
        a[i] = (a[i] * b[i] + tmp) / (a[i] + b[i] + 1.0);
    }
    
    /* Nested parallelism with scan - may generate _SCANTEMP_ */
    double scan_temp = 0.0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:scan_temp)
        for (int i = 0; i < n; i++) {
            scan_temp += i * 0.1;
            #pragma omp scan inclusive(scan_temp)
            a[i] += scan_temp;
        }
    }
}

/* Function 4: Mixed OpenMP constructs with conditional compilation */
void test_mixed_constructs(struct DataBlock *block, int threshold) {
    /* Conditional parallel region */
    #pragma omp parallel if(threshold > 500) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Target region with complex mapping */
        #pragma omp target teams distribute parallel for \
            map(tofrom: block->values[:N/2]) \
            if(threshold > 1000)
        for (int i = 0; i < N/2; i++) {
            block->values[i + tid * (N/8)] *= 1.1;
        }
        
        /* Task with dependencies */
        #pragma omp task depend(inout: block->indices[tid*10]) \
            firstprivate(tid)
        {
            for (int j = 0; j < 10; j++) {
                block->indices[tid * 10 + j] = tid * 100 + j;
            }
        }
    }
    
    #pragma omp taskwait
    
    /* SIMD with reduction and linear - likely to generate internal temps */
    double sum = 0.0;
    #pragma omp simd reduction(+:sum) linear(i:1) \
        aligned(block->values:64)
    for (int i = 0; i < N; i++) {
        sum += block->values[i] * block->indices[i];
    }
    block->result = sum;
}

/* Main function that orchestrates all tests */
int main() {
    double *array1 = (double*)malloc(N * M * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    struct DataBlock data_block;
    
    /* Initialize data */
    for (int i = 0; i < N * M; i++) {
        array1[i] = 0.0;
        array2[i] = 0.0;
    }
    data_block.result = 0.0;
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Test 1: Scan clauses */
    test_scan_clauses(array1, N * M);
    printf("Test 1 (scan clauses) completed.\n");
    
    /* Test 2: Enter data with to clause */
    test_enter_data(&data_block);
    printf("Test 2 (enter data) completed. Result: %f\n", data_block.result);
    
    /* Test 3: Internal temporaries */
    test_internal_temporaries(array1, array2, N);
    printf("Test 3 (internal temporaries) completed.\n");
    
    /* Test 4: Mixed constructs */
    test_mixed_constructs(&data_block, N * M);
    printf("Test 4 (mixed constructs) completed. Final result: %f\n", data_block.result);
    
    /* Verify results */
    double final_check = 0.0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < N * M; i++) {
        final_check += array1[i] + array2[i];
    }
    printf("Final array sum check: %f\n", final_check);
    
    free(array1);
    free(array2);
    
    return 0;
}

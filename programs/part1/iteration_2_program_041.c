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
    double prefix_sum[N] = {0};
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    /* Trigger OMP_CLAUSE_INCLUSIVE and OMP_CLAUSE_EXCLUSIVE */
    double scan_temp = 0.0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:scan_temp) nowait
        for (int i = 0; i < n; i++) {
            #pragma omp scan inclusive(scan_temp)
            scan_temp += arr[i];
            prefix_sum[i] = scan_temp;
        }
        
        #pragma omp for reduction(+:scan_temp)
        for (int i = n-1; i >= 0; i--) {
            #pragma omp scan exclusive(scan_temp)
            scan_temp += arr[i];
            prefix_sum[i] += scan_temp;
        }
    }
    
    printf("Scan test completed: final sum = %f\n", scan_temp);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *data) {
    /* Trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: data[0:M]) \
        map(to: data->values[0:N/2]) \
        map(to: data->indices) \
        depend(out: data)
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:M]) \
        depend(inout: data)
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            data[i].values[j] = i * 1000.0 + j;
            data[i].indices[j] = i * N + j;
        }
        data[i].result = 0.0;
    }
    
    #pragma omp exit data map(from: data[0:M]) \
        depend(in: data)
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *a, double *b, int n) {
    double reduction_var = 0.0;
    double last_val = 0.0;
    
    /* Complex nested pragmas to trigger _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel if(n > 500) reduction(+:reduction_var) \
        private(last_val) firstprivate(n)
    {
        #pragma omp for simd reduction(+:reduction_var) \
            lastprivate(last_val) linear(i:1) collapse(2) \
            schedule(static, 16) nowait
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n/10; j++) {
                double temp = a[i] * b[j];
                reduction_var += temp;
                if (i == n-1 && j == n/10 - 1) {
                    last_val = temp;
                }
            }
        }
        
        /* Additional reduction to generate more temporaries */
        #pragma omp for reduction(*:reduction_var) \
            lastprivate(last_val) ordered
        for (int i = 0; i < n; i++) {
            #pragma omp ordered
            {
                reduction_var *= 1.0001;
                last_val = a[i];
            }
        }
    }
    
    printf("Internal temporaries test: reduction = %f, last = %f\n", 
           reduction_var, last_val);
}

/* Function 4: SIMD with conditionals to generate _CONDTEMP_ */
void test_condtemp_clauses(double *arr, int n) {
    #pragma omp parallel simd simdlen(8) \
        reduction(+:arr[:n]) aligned(arr:64) \
        if(n > 100) proc_bind(spread)
    for (int i = 0; i < n; i++) {
        /* Complex conditional to generate condition temporaries */
        if (i % 3 == 0) {
            arr[i] *= 2.0;
        } else if (i % 3 == 1) {
            arr[i] /= 2.0;
        } else {
            arr[i] = arr[i] > 0.5 ? arr[i] : 0.5;
        }
        
        /* Nested condition with OpenMP atomic */
        if (arr[i] > 100.0) {
            #pragma omp atomic update
            arr[0] += 1.0;
        }
    }
}

/* Function 5: Scan with _SCANTEMP_ generation */
void test_scantemp(double *arr, int n) {
    double scan_var = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for simd reduction(inscan,+:scan_var)
        for (int i = 0; i < n; i++) {
            /* This should generate _SCANTEMP_ clauses */
            scan_var += arr[i];
            #pragma omp scan inclusive(scan_var)
            arr[i] = scan_var;
            
            /* Additional computation to create complex scan context */
            if (i % 7 == 0) {
                #pragma omp atomic
                scan_var += 1.0;
            }
        }
    }
    
    printf("Scan temp test: final scan value = %f\n", scan_var);
}

/* Main function orchestrating all tests */
int main() {
    /* Allocate test data */
    double *array1 = (double*)aligned_alloc(64, N * sizeof(double));
    double *array2 = (double*)aligned_alloc(64, N * sizeof(double));
    struct DataBlock *data_blocks = (struct DataBlock*)malloc(M * sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !data_blocks) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    #pragma omp parallel for simd
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
    }
    
    printf("Starting OpenMP clause coverage tests...\n");
    
    /* Test 1: Scan clauses (inclusive/exclusive) */
    test_scan_clauses(array1, N);
    
    /* Test 2: Enter data with to mapper */
    test_enter_data(data_blocks);
    
    /* Test 3: Internal temporaries */
    test_internal_temporaries(array1, array2, N);
    
    /* Test 4: Conditional temporaries */
    test_condtemp_clauses(array1, N/2);
    
    /* Test 5: Scan temporaries */
    test_scantemp(array2, N);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp target teams distribute parallel for \
        map(tofrom: final_sum) reduction(+:final_sum) \
        map(to: array1[0:N], array2[0:N])
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    printf("Final result: %f\n", final_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(data_blocks);
    
    return 0;
}

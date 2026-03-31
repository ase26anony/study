/* Test program to trigger uncovered pretty-printing of OpenMP clauses */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 2000
#define M 100

/* Structure for complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void scan_test(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
        partial_sum += arr[i];
    }
    
    /* This should generate OMP_CLAUSE_INCLUSIVE/EXCLUSIVE */
    #pragma omp parallel
    {
        #pragma omp for reduction(+:exclusive_prefix) nowait
        for (int i = 1; i < n; i++) {
            #pragma omp scan exclusive
            exclusive_prefix += arr[i-1];
            arr[i] += exclusive_prefix;
        }
        
        #pragma omp for reduction(+:partial_sum)
        for (int i = 0; i < n/2; i++) {
            double temp = arr[i];
            #pragma omp scan inclusive
            partial_sum += temp;
            arr[i] = partial_sum;
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void enter_data_test(struct DataBlock *block) {
    /* This should generate OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    #pragma omp target teams distribute parallel for \
            map(tofrom: block->values[0:N/2])
    for (int i = 0; i < N/2; i++) {
        block->values[i] *= 2.0;
        block->indices[i] = i;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values[0:N/2])
}

/* Function 3: Complex nested loops to generate internal temporaries */
void complex_loop_test(double *a, double *b, double *c, int n) {
    double sum = 0.0;
    double last_val = 0.0;
    
    /* Combined clauses to potentially generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for reduction(+:sum) lastprivate(last_val) \
                    linear(i:1) collapse(2) if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            a[idx] = i + j * 0.1;
            b[idx] = a[idx] * 2.0;
            c[idx] = a[idx] + b[idx];
            sum += c[idx];
            if (j == M-1) last_val = c[idx];
        }
    }
    
    /* Additional reduction with array section */
    #pragma omp parallel
    {
        double local_sum = 0.0;
        #pragma omp for nowait
        for (int i = 0; i < n * M; i++) {
            local_sum += a[i];
        }
        #pragma omp atomic
        sum += local_sum;
    }
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ generation */
void simd_cond_test(double *arr, int n) {
    #pragma omp simd reduction(+:arr[:n]) \
                     linear(i:1) safelen(16) if(n > 500)
    for (int i = 0; i < n; i++) {
        /* Complex conditional that might generate _CONDTEMP_ */
        double val = (i % 2 == 0) ? i * 1.5 : i * 0.5;
        if (val > 100.0) {
            arr[i] = val / 2.0;
        } else {
            arr[i] = val * 2.0;
        }
    }
}

/* Function 5: Target region with complex data environment */
void target_region_test(struct DataBlock *block1, struct DataBlock *block2) {
    /* Complex map clauses for temporary generation */
    #pragma omp target teams distribute parallel for \
            map(to: block1->values[0:N]) \
            map(tofrom: block2->values[0:N]) \
            map(alloc: block1->indices[0:N/4]) \
            reduction(+: block1->result) \
            if(N > 1000)
    for (int i = 0; i < N; i++) {
        block2->values[i] += block1->values[i];
        if (i < N/4) {
            block1->indices[i] = i;
        }
        block1->result += block2->values[i];
    }
}

/* Function 6: Scan directive with multiple clauses */
void scan_directive_test(double *arr, int n) {
    double prefix_sum = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for simd reduction(inscan,+:prefix_sum)
        for (int i = 0; i < n; i++) {
            double val = arr[i];
            #pragma omp scan exclusive(prefix_sum)
            arr[i] = prefix_sum;
            prefix_sum += val;
        }
        
        /* Another scan with inclusive */
        #pragma omp for simd reduction(inscan,+:prefix_sum)
        for (int i = n-1; i >= 0; i--) {
            double val = arr[i];
            #pragma omp scan inclusive(prefix_sum)
            prefix_sum += val;
            arr[i] += prefix_sum;
        }
    }
}

int main() {
    /* Allocate and initialize data */
    double *array1 = (double*)malloc(N * M * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    double *array3 = (double*)malloc(N * M * sizeof(double));
    double *scan_array = (double*)malloc(N * sizeof(double));
    
    struct DataBlock block1, block2;
    
    /* Initialize arrays */
    for (int i = 0; i < N * M; i++) {
        array1[i] = 1.0;
        array2[i] = 2.0;
        array3[i] = 0.0;
    }
    
    for (int i = 0; i < N; i++) {
        scan_array[i] = i * 0.1;
        block1.values[i] = i * 0.01;
        block2.values[i] = i * 0.02;
        block1.indices[i] = 0;
        block2.indices[i] = 0;
    }
    block1.result = 0.0;
    block2.result = 0.0;
    
    printf("Starting OpenMP tests...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    scan_test(scan_array, N);
    
    enter_data_test(&block1);
    
    complex_loop_test(array1, array2, array3, N);
    
    simd_cond_test(scan_array, N);
    
    target_region_test(&block1, &block2);
    
    scan_directive_test(scan_array, N/2);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N * M; i++) {
        final_sum += array3[i];
    }
    
    final_sum += block1.result + block2.result;
    
    printf("Final result: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(scan_array);
    
    return 0;
}

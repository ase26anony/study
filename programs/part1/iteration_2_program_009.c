/* Test program to trigger OpenMP clause pretty-printing coverage */
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
void scan_test(int n, double *arr) {
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        double temp = arr[i] * 2.0;
        
        /* Inclusive scan */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += temp;
        arr[i] = prefix_sum;
        
        /* Exclusive scan simulation */
        exclusive_sum = prefix_sum - temp;
        if (i > 0) {
            #pragma omp scan exclusive(exclusive_sum)
            arr[i-1] += exclusive_sum;
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void enter_data_test(struct DataBlock *block) {
    /* Enter data with to clause */
    #pragma omp enter data map(to: block->values[0:N/2]) \
                           map(to: block->indices) \
                           map(alloc: block->result)
    
    /* Use the data */
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[0:N/2])
    for (int i = 0; i < N/2; i++) {
        block->values[i] = block->values[i] * 3.14 + i;
    }
    
    #pragma omp exit data map(from: block->result) \
                          map(release: block->values[0:N/2])
}

/* Function 3: Complex loops to generate internal temporaries */
void complex_reduction_test(int n, double *a, double *b, int *c) {
    double total = 0.0;
    int last_val = 0;
    
    /* Combined clauses to provoke _LOOPTEMP_ and _REDUCTEMP_ */
    #pragma omp parallel for simd reduction(+:total) lastprivate(last_val) \
                linear(i:1) collapse(2) schedule(dynamic, 16)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            double val = a[i] * b[j];
            
            /* Conditional inside SIMD - may generate _CONDTEMP_ */
            if (val > 0.5) {
                total += val;
                c[idx] = 1;
            } else {
                c[idx] = 0;
            }
            
            /* Complex expression with reduction */
            #pragma omp atomic
            total += (val * 0.1);
            
            last_val = idx;
        }
    }
    
    /* Nested parallelism with reduction */
    #pragma omp parallel reduction(+:total)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            total += a[i];
        }
        
        #pragma omp for reduction(max:last_val)
        for (int i = 0; i < n; i++) {
            if (a[i] > last_val) last_val = i;
        }
    }
}

/* Function 4: Target region with complex mapping */
void target_test(struct DataBlock *block1, struct DataBlock *block2) {
    #pragma omp target map(to: block1[0:1]) \
                       map(tofrom: block2->values[0:N]) \
                       map(alloc: block1->indices[0:N/2])
    {
        #pragma omp teams distribute parallel for simd \
                    reduction(+:block1->result)
        for (int i = 0; i < N; i++) {
            block1->result += block2->values[i];
            block1->indices[i % (N/2)] = i;
        }
    }
}

/* Function 5: Conditional parallel regions */
void conditional_test(int n, double *arr, int threshold) {
    /* Conditional parallel region */
    #pragma omp parallel if(n > threshold) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        #pragma omp for schedule(guided) nowait
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] + tid * 0.01;
        }
        
        /* Barrier with scan */
        #pragma omp barrier
        
        #pragma omp for reduction(+:arr[0:n]) schedule(static)
        for (int i = 0; i < n; i++) {
            arr[i] = arr[i] * 2.0;
        }
    }
}

/* Main function orchestrates all tests */
int main() {
    /* Allocate and initialize data */
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    int *array3 = (int*)malloc(N * M * sizeof(int));
    struct DataBlock block1, block2;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
        block1.values[i] = i * 0.5;
        block2.values[i] = i * 0.25;
        block1.indices[i] = i;
        block2.indices[i] = N - i;
    }
    block1.result = 0.0;
    block2.result = 0.0;
    
    /* Call test functions to trigger various OpenMP constructs */
    printf("Starting OpenMP coverage test...\n");
    
    /* Test 1: Scan clauses */
    scan_test(N, array1);
    
    /* Test 2: Enter data with to mapper */
    enter_data_test(&block1);
    
    /* Test 3: Complex reductions for internal temporaries */
    complex_reduction_test(100, array1, array2, array3);
    
    /* Test 4: Target regions */
    target_test(&block1, &block2);
    
    /* Test 5: Conditional parallel regions */
    conditional_test(N, array1, 500);
    
    /* Compute final result */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                schedule(runtime)
    for (int i = 0; i < N; i++) {
        final_result += array1[i] + block1.values[i] + block2.values[i];
    }
    
    final_result += block1.result + block2.result;
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}

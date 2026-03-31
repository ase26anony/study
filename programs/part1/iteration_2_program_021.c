/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1000
#define CHUNK 64

/* Structure for testing complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double prefix_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = (double)(i + 1);
    }
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        
        #pragma omp for reduction(+:prefix_sum) nowait
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
            
            /* This should generate OMP_CLAUSE_INCLUSIVE */
            #pragma omp scan inclusive(local_sum)
            prefix_sum = local_sum;
        }
        
        /* Another loop with exclusive scan */
        #pragma omp for reduction(+:prefix_sum)
        for (int i = 0; i < n; i++) {
            local_sum -= arr[i];
            
            /* This should generate OMP_CLAUSE_EXCLUSIVE */
            #pragma omp scan exclusive(local_sum)
            prefix_sum = local_sum;
        }
    }
    
    printf("Scan test result: %f\n", prefix_sum);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* This should generate OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO set */
    #pragma omp enter data map(to: block->values[:N]) \
                           map(to: block->indices[:N])
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: block->values[:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = (double)i * 2.0;
        block->indices[i] = i;
    }
    
    #pragma omp exit data map(from: block->values[:N]) \
                          map(release: block->indices[:N])
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *a, double *b, double *c, int n) {
    double sum = 0.0;
    double last_val = 0.0;
    
    /* Complex nested parallelism with multiple clauses */
    #pragma omp parallel for reduction(+:sum) lastprivate(last_val) \
                linear(i:1) collapse(2) if(n > 500)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < CHUNK; j++) {
            int idx = i * CHUNK + j;
            if (idx < n) {
                a[idx] = b[idx] + c[idx];
                sum += a[idx];
                last_val = a[idx];
            }
        }
    }
    
    /* SIMD loop with conditionals - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:sum) linear(i:1) \
                simdlen(8) if(n > 100)
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            a[i] = b[i] * 2.0;
        } else {
            a[i] = c[i] / 2.0;
        }
        sum += a[i];
    }
    
    /* Target region with complex data environment */
    #pragma omp target teams distribute parallel for \
                map(to: b[:n], c[:n]) map(from: a[:n]) \
                reduction(+:sum) if(target: n > 100)
    for (int i = 0; i < n; i++) {
        a[i] = b[i] + c[i];
        sum += a[i];
    }
    
    printf("Temporaries test - sum: %f, last: %f\n", sum, last_val);
}

/* Function 4: Mixed constructs for varied clause generation */
void test_mixed_constructs(struct DataBlock *block, int n) {
    /* Task with depend clauses */
    #pragma omp task depend(inout: block->result) \
                if(n > 100) shared(block)
    {
        block->result = 0.0;
        for (int i = 0; i < n; i++) {
            block->result += block->values[i];
        }
    }
    
    /* Parallel sections with reduction */
    #pragma omp parallel sections reduction(+:block->result) \
                if(block->result > 0)
    {
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                block->result += block->indices[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                block->result -= block->indices[i];
            }
        }
    }
    
    /* Loop with schedule(dynamic) and ordered */
    #pragma omp parallel for ordered schedule(dynamic, 16) \
                if(n > 200)
    for (int i = 0; i < n; i++) {
        #pragma omp ordered
        {
            block->values[i] = block->result * i;
        }
    }
}

int main() {
    /* Allocate and initialize test data */
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    struct DataBlock *block = (struct DataBlock*)malloc(sizeof(struct DataBlock));
    
    if (!array1 || !array2 || !array3 || !block) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i;
        array2[i] = (double)(i * 2);
        array3[i] = (double)(i * 3);
        block->values[i] = 0.0;
        block->indices[i] = i;
    }
    block->result = 0.0;
    
    printf("Starting OpenMP coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array1, N);
    test_enter_data(block);
    test_internal_temporaries(array1, array2, array3, N);
    test_mixed_constructs(block, N);
    
    /* Final reduction across all results */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                if(N > 500) schedule(guided)
    for (int i = 0; i < N; i++) {
        final_result += array1[i] + block->values[i];
    }
    
    final_result += block->result;
    
    printf("Final result: %f\n", final_result);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(block);
    
    return 0;
}

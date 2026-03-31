/* Test program to trigger uncovered pretty-printing of OpenMP clauses */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 2000
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
    
    #pragma omp parallel for reduction(+:partial_sum) \
        private(exclusive_prefix) schedule(static)
    for (int i = 0; i < n; i++) {
        /* Scan inclusive clause */
        #pragma omp scan inclusive(partial_sum)
        partial_sum += arr[i];
        arr[i] = partial_sum;
        
        /* Scan exclusive clause */
        #pragma omp scan exclusive(exclusive_prefix)
        exclusive_prefix += arr[i] * 0.5;
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* Enter data with to clause */
    #pragma omp enter data map(to: block->values[0:N/2]) \
        map(to: block->indices[0:N/2])
    
    /* Use the data in target region */
    #pragma omp target map(tofrom: block->result) \
        map(to: block->values[0:N/2]) map(to: block->indices[0:N/2])
    {
        block->result = 0.0;
        for (int i = 0; i < N/2; i++) {
            block->result += block->values[i] * block->indices[i];
        }
    }
    
    #pragma omp exit data map(release: block->values[0:N/2]) \
        map(release: block->indices[0:N/2])
}

/* Function 3: Complex loops to generate internal temporaries */
void test_internal_temporaries(double *a, double *b, int n) {
    double last_val = 0.0;
    int last_idx = 0;
    
    /* Combined clauses to provoke _LOOPTEMP_, _REDUCTEMP_ generation */
    #pragma omp parallel for reduction(+:a[0:n]) \
        lastprivate(last_val, last_idx) linear(i:1) collapse(2) \
        schedule(dynamic)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            a[i] += b[j] * (i + j);
            if (i == n - 1 && j == M - 1) {
                last_val = a[i];
                last_idx = i;
            }
        }
    }
    
    /* Nested parallelism with reduction */
    #pragma omp parallel reduction(+:last_val)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            last_val += a[i];
        }
        
        #pragma omp single
        {
            /* Conditional parallel region */
            #pragma omp parallel if(n > 1000) reduction(max:last_idx)
            {
                #pragma omp for
                for (int i = 0; i < n; i++) {
                    if (a[i] > last_val) last_idx = i;
                }
            }
        }
    }
}

/* Function 4: SIMD with conditionals for _CONDTEMP_ */
void test_simd_conditionals(double *a, double *b, double *c, int n) {
    #pragma omp simd reduction(+:a[:n]) linear(i:1) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        double temp = b[i];
        if (temp > 0.5) {
            a[i] = temp * c[i];
        } else {
            a[i] = temp + c[i];
        }
        
        /* Nested condition to increase complexity */
        #pragma omp simd
        for (int j = 0; j < 4; j++) {
            if (a[i] > 1.0) {
                a[i] /= (j + 1);
            }
        }
    }
}

/* Function 5: Target regions with complex mappings */
void test_target_complex(struct DataBlock *block1, struct DataBlock *block2) {
    /* Multiple map clauses with different types */
    #pragma omp target teams distribute parallel for \
        map(to: block1->values[0:N]) \
        map(tofrom: block2->values[0:N]) \
        map(alloc: block1->indices[0:N/4]) \
        reduction(+:block1->result) \
        num_teams(4) thread_limit(64)
    for (int i = 0; i < N; i++) {
        block2->values[i] = block1->values[i] * 2.0;
        block1->result += block2->values[i];
    }
    
    /* Enter data with conditional */
    if (block1->result > 100.0) {
        #pragma omp enter data map(to: block1->values[N/2:N/2]) \
            map(always, to: block1->indices[N/4:N/2])
    }
}

int main() {
    /* Initialize data */
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    struct DataBlock block1, block2;
    
    srand(42);
    for (int i = 0; i < N; i++) {
        array1[i] = (double)rand() / RAND_MAX;
        array2[i] = (double)rand() / RAND_MAX;
        array3[i] = (double)rand() / RAND_MAX;
        block1.values[i] = array1[i];
        block2.values[i] = array2[i];
        block1.indices[i] = i;
        block2.indices[i] = N - i;
    }
    block1.result = 0.0;
    block2.result = 0.0;
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(array1, N);
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            test_enter_data(&block1);
        }
    }
    
    test_internal_temporaries(array2, array3, N);
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            test_simd_conditionals(array1, array2, array3, N);
        }
        
        #pragma omp section
        {
            test_target_complex(&block1, &block2);
        }
    }
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        schedule(guided)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + array3[i];
    }
    
    final_sum += block1.result + block2.result;
    
    printf("Final result: %f\n", final_sum);
    printf("Verification: %s\n", (final_sum > 0.0) ? "PASS" : "FAIL");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}

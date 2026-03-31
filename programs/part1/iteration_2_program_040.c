/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
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
    double prefix_sum = 0.0;
    double exclusive_sum = 0.0;
    
    #pragma omp parallel for reduction(+:prefix_sum) private(exclusive_sum)
    for (int i = 0; i < n; i++) {
        exclusive_sum = prefix_sum;
        
        #pragma omp scan exclusive(exclusive_sum)
        arr[i] += exclusive_sum;
        
        prefix_sum += arr[i];
        
        #pragma omp scan inclusive(prefix_sum)
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    #pragma omp target enter data map(to: block[0:1]) \
        to(block->values[0:N]) to(block->indices[0:N])
    
    /* Simulate some target computation */
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[0:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = i * 1.5;
    }
    
    #pragma omp target exit data map(from: block[0:1])
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(double *a, double *b, int n) {
    double red = 0.0;
    int last_val = 0;
    
    /* Combined clauses to provoke _LOOPTEMP_ and _REDUCTEMP_ */
    #pragma omp parallel for simd reduction(+:red) lastprivate(last_val) \
        linear(i:1) collapse(2) if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            a[i * M + j] = i + j * 0.5;
            red += a[i * M + j];
            if (j == M - 1) last_val = i;
        }
    }
    
    /* Nested parallelism with conditionals */
    #pragma omp parallel if(n > 500)
    {
        #pragma omp for nowait reduction(+:red)
        for (int i = 0; i < n; i++) {
            #pragma omp simd reduction(+:red)
            for (int j = 0; j < M; j++) {
                double temp = b[i * M + j];
                if (temp > 0.0) {
                    red += temp * 0.1;
                }
                b[i * M + j] = temp * 2.0;
            }
        }
    }
}

/* Function 4: Target regions with complex mappings */
void test_target_temporaries(struct DataBlock *block, double *output) {
    #pragma omp target map(to: block[0:1]) map(from: output[0:N]) \
        map(tofrom: block->values[0:N])
    {
        #pragma omp teams distribute parallel for simd \
            reduction(+:block->result)
        for (int i = 0; i < N; i++) {
            block->values[i] = block->values[i] * 2.0 + i;
            block->result += block->values[i];
            output[i] = block->values[i];
        }
    }
}

/* Function 5: Mixed directives to increase clause variety */
void test_mixed_directives(int *arr, int n) {
    int sum = 0;
    int last = 0;
    
    #pragma omp parallel for simd reduction(+:sum) lastprivate(last) \
        schedule(simd:guided) if(n > 100)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
        sum += arr[i];
        last = i;
        
        /* Conditional inside loop for _CONDTEMP_ generation */
        if (arr[i] % 3 == 0) {
            #pragma omp atomic
            sum += 1;
        }
    }
    
    /* Task with depend clauses */
    #pragma omp task depend(inout: arr[0:n]) shared(arr, n)
    {
        for (int i = 0; i < n; i++) {
            arr[i] += 1;
        }
    }
    
    #pragma omp taskwait
}

int main() {
    double *array1 = (double *)malloc(N * M * sizeof(double));
    double *array2 = (double *)malloc(N * M * sizeof(double));
    double *output = (double *)malloc(N * sizeof(double));
    int *int_array = (int *)malloc(N * sizeof(int));
    
    struct DataBlock block;
    
    /* Initialize data */
    for (int i = 0; i < N * M; i++) {
        array1[i] = i * 0.1;
        array2[i] = i * 0.2;
    }
    
    for (int i = 0; i < N; i++) {
        block.values[i] = i * 0.5;
        block.indices[i] = i;
        int_array[i] = 0;
    }
    block.result = 0.0;
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(array1, N);
    
    test_enter_data(&block);
    
    test_internal_temporaries(array1, array2, N);
    
    test_target_temporaries(&block, output);
    
    test_mixed_directives(int_array, N);
    
    /* Final computation and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum)
    for (int i = 0; i < N * M; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    final_sum += block.result;
    
    for (int i = 0; i < N; i++) {
        final_sum += output[i] + int_array[i];
    }
    
    printf("Final result: %f\n", final_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(output);
    free(int_array);
    
    return 0;
}

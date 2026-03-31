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
    double prefix_sum[N] = {0};
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        
        #pragma omp for reduction(+:local_sum) nowait
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
        }
        
        #pragma omp for ordered schedule(static)
        for (int i = 0; i < n; i++) {
            #pragma omp ordered
            {
                #pragma omp scan inclusive(partial_sum)
                partial_sum += arr[i];
                prefix_sum[i] = partial_sum;
            }
        }
    }
    
    /* Another loop with exclusive scan */
    double exclusive_prefix[N] = {0};
    double running = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for ordered schedule(dynamic)
        for (int i = 0; i < n; i++) {
            #pragma omp ordered
            {
                exclusive_prefix[i] = running;
                #pragma omp scan exclusive(running)
                running += arr[i];
            }
        }
    }
    
    printf("Scan test: final sum = %f\n", partial_sum);
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *data) {
    /* Create device data environment with to mapper */
    #pragma omp target enter data map(to: data[0:1]) \
        map(to: data->values[0:N]) \
        map(to: data->indices[0:N])
    
    /* Perform computation on device */
    #pragma omp target teams distribute parallel for \
        map(alloc: data->values[0:N]) \
        map(tofrom: data->result)
    for (int i = 0; i < N; i++) {
        data->values[i] = data->values[i] * 2.0;
        #pragma omp atomic
        data->result += data->values[i];
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: data->result) \
        map(release: data->values[0:N]) \
        map(delete: data->indices[0:N])
}

/* Function 3: Complex loops to generate internal temporaries */
void test_internal_temporaries(double *matrix, int rows, int cols) {
    double total = 0.0;
    int last_val = 0;
    
    /* Complex parallel loop with multiple clauses */
    #pragma omp parallel for collapse(2) \
        reduction(+:total) \
        lastprivate(last_val) \
        linear(i:1) \
        schedule(dynamic, 16) \
        if(rows > 100)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            double val = matrix[i * cols + j];
            total += val * val;
            
            /* Conditional inside loop to generate _CONDTEMP_ */
            if (val > 0.5) {
                #pragma omp atomic
                total += 0.1;
            }
            
            last_val = i * cols + j;
        }
    }
    
    /* Nested parallelism with reduction */
    #pragma omp parallel reduction(+:total)
    {
        #pragma omp for nowait
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                matrix[i * cols + j] = total / (rows * cols);
            }
        }
        
        #pragma omp single
        {
            printf("Internal temporaries test: total = %f, last = %d\n", 
                   total, last_val);
        }
    }
}

/* Function 4: SIMD with conditionals */
void test_simd_conditionals(double *a, double *b, double *c, int n) {
    /* SIMD loop with conditional that may generate _CONDTEMP_ */
    #pragma omp simd linear(i:1) reduction(+:a[0:n]) \
        safelen(16) aligned(a, b, c: 64)
    for (int i = 0; i < n; i++) {
        double temp = b[i] * c[i];
        
        /* Complex conditional */
        if (temp > 0.0 && i % 2 == 0) {
            a[i] += temp;
        } else if (temp < 0.0) {
            a[i] -= temp;
        } else {
            a[i] = 0.0;
        }
    }
    
    /* Another SIMD with scan */
    double scan_temp = 0.0;
    #pragma omp simd reduction(inscan, +:scan_temp)
    for (int i = 0; i < n; i++) {
        b[i] = scan_temp;
        #pragma omp scan exclusive(scan_temp)
        scan_temp += a[i];
    }
}

/* Function 5: Target regions with complex data clauses */
void test_target_complex(struct DataBlock *data, double *output) {
    /* Complex target region with multiple map types */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data->values[0:N]) \
        map(to: data->indices) \
        map(alloc: output[0:M]) \
        reduction(+: data->result) \
        depend(inout: data->values) \
        if(target: N > 500)
    for (int i = 0; i < N; i++) {
        int idx = data->indices[i] % M;
        output[idx] += data->values[i];
        data->result += data->values[i];
    }
    
    /* Nested target region */
    #pragma omp target data map(to: output[0:M]) map(from: data->result)
    {
        #pragma omp target teams distribute parallel for \
            reduction(max: data->result)
        for (int i = 0; i < M; i++) {
            if (output[i] > data->result) {
                data->result = output[i];
            }
        }
    }
}

int main() {
    /* Initialize data */
    double *array = (double *)malloc(N * sizeof(double));
    double *matrix = (double *)malloc(N * M * sizeof(double));
    double *a = (double *)malloc(N * sizeof(double));
    double *b = (double *)malloc(N * sizeof(double));
    double *c = (double *)malloc(N * sizeof(double));
    double *output = (double *)calloc(M, sizeof(double));
    
    struct DataBlock data;
    
    /* Initialize arrays with some values */
    for (int i = 0; i < N; i++) {
        array[i] = (double)i / N;
        a[i] = 1.0;
        b[i] = 2.0;
        c[i] = 3.0;
        data.values[i] = (double)i * 0.01;
        data.indices[i] = i % M;
    }
    
    for (int i = 0; i < N * M; i++) {
        matrix[i] = (double)(i % 100) * 0.1;
    }
    
    data.result = 0.0;
    
    printf("Starting OpenMP clause coverage test...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    test_scan_clauses(array, N);
    
    /* Conditional compilation path */
    #pragma omp parallel if(N > 1000)
    {
        #pragma omp single
        printf("Conditional parallel region executed\n");
    }
    
    test_enter_data(&data);
    test_internal_temporaries(matrix, N, M);
    test_simd_conditionals(a, b, c, N);
    test_target_complex(&data, output);
    
    /* Final reduction and output */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
        lastprivate(array) \
        linear(i:1)
    for (int i = 0; i < N; i++) {
        final_result += array[i] + a[i] + b[i];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Data result: %f\n", data.result);
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(a);
    free(b);
    free(c);
    free(output);
    
    return 0;
}

/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define CHUNK 64

struct data_t {
    double values[N];
    int indices[N];
    double sum;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void scan_test(double *arr, int n) {
    double partial_sum = 0.0;
    double exclusive_prefix = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum) schedule(static, CHUNK)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    #pragma omp parallel for reduction(+:exclusive_prefix) schedule(dynamic)
    for (int i = 0; i < n; i++) {
        double temp = arr[i] * 2.0;
        
        /* This should generate scan clauses */
        #pragma omp scan inclusive(partial_sum)
        partial_sum += temp;
        
        #pragma omp scan exclusive(exclusive_prefix)
        exclusive_prefix = partial_sum - temp;
        
        arr[i] = exclusive_prefix;
    }
}

/* Function 2: Uses enter data with to mapper */
void enter_data_test(struct data_t *data) {
    /* Force generation of enter clause with to mapper */
    #pragma omp enter data map(to: data->values[0:N/2]) \
                           map(to: data->indices) \
                           map(alloc: data->sum)
    
    #pragma omp target enter data map(to: data->values[N/2:N/2]) \
                                  map(to: data) if(N > 500)
    
    /* Complex reduction to generate internal temporaries */
    #pragma omp target teams distribute parallel for \
                reduction(+:data->sum) map(tofrom:data->values[0:N])
    for (int i = 0; i < N; i++) {
        data->sum += data->values[i];
    }
    
    #pragma omp exit data map(from: data->sum) map(release: data->values)
}

/* Function 3: Complex nested loops to generate _LOOPTEMP_, _REDUCTEMP_ */
void nested_loop_test(int *a, int *b, int *c, int n) {
    int i, j, k;
    
    /* This complex construct should generate internal temporaries */
    #pragma omp parallel for collapse(2) private(j, k) \
                reduction(+:a[0:n]) lastprivate(i) linear(j:1) if(n > 100)
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            k = i * n + j;
            #pragma omp simd reduction(*:b[0:n]) linear(k:1)
            for (int l = 0; l < n; l++) {
                b[l] *= a[i] + c[j];
                k++;
            }
            a[i] += j;
        }
    }
    
    /* Another complex case with multiple clauses */
    #pragma omp target teams distribute parallel for simd \
                reduction(max:b[0:n]) reduction(min:c[0:n]) \
                lastprivate(j) linear(i:2) schedule(static, 16)
    for (i = 0; i < n * n; i += 2) {
        j = i / n;
        b[j] = b[j] > i ? b[j] : i;
        c[j] = c[j] < i ? c[j] : i;
    }
}

/* Function 4: SIMD with conditionals to generate _CONDTEMP_ */
void simd_cond_test(double *arr, int n) {
    double threshold = 0.5;
    
    #pragma omp parallel for simd schedule(static) \
                reduction(+:threshold) if(n > 200)
    for (int i = 0; i < n; i++) {
        /* Complex conditional that might generate _CONDTEMP_ */
        if (arr[i] > threshold) {
            arr[i] = arr[i] * 2.0;
            threshold += 0.01;
        } else if (arr[i] < -threshold) {
            arr[i] = arr[i] / 2.0;
            threshold -= 0.005;
        } else {
            arr[i] = 0.0;
        }
        
        /* Nested conditional */
        for (int j = 0; j < 4; j++) {
            if (arr[i] > j * 0.25) {
                arr[i] += j * 0.1;
            }
        }
    }
}

/* Function 5: Scan with temporaries for _SCANTEMP_ */
void scan_complex_test(double *x, double *y, int n) {
    double prefix_sum = 0.0;
    double exclusive_scan = 0.0;
    
    #pragma omp parallel for simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        /* Multiple scan directives */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += x[i];
        
        y[i] = prefix_sum;
        
        #pragma omp scan exclusive(exclusive_scan)
        exclusive_scan = prefix_sum - x[i];
        
        x[i] = exclusive_scan;
    }
    
    /* Mixed directives to increase clause complexity */
    #pragma omp target parallel for simd \
                map(tofrom: x[0:n], y[0:n]) \
                reduction(+:prefix_sum, exclusive_scan) \
                in_reduction(+:prefix_sum)
    for (int i = 0; i < n; i++) {
        x[i] += y[i];
        prefix_sum += x[i];
        exclusive_scan += y[i];
    }
}

int main() {
    double *array1 = (double*)malloc(N * sizeof(double));
    double *array2 = (double*)malloc(N * sizeof(double));
    int *array3 = (int*)malloc(N * sizeof(int));
    int *array4 = (int*)malloc(N * sizeof(int));
    int *array5 = (int*)malloc(N * sizeof(int));
    struct data_t data;
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        array1[i] = (double)i / N;
        array2[i] = (double)(N - i) / N;
        array3[i] = i;
        array4[i] = i * 2;
        array5[i] = i % 10;
        data.values[i] = (double)i;
        data.indices[i] = i;
    }
    data.sum = 0.0;
    
    printf("Starting OpenMP tests...\n");
    
    /* Call all test functions to trigger various OpenMP constructs */
    scan_test(array1, N);
    
    enter_data_test(&data);
    
    nested_loop_test(array3, array4, array5, N/10);
    
    simd_cond_test(array2, N);
    
    scan_complex_test(array1, array2, N/2);
    
    /* Final reduction and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
                if(N > 100) schedule(guided)
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + array3[i] + array4[i] + array5[i];
    }
    
    final_sum += data.sum;
    
    printf("Final result: %f\n", final_sum);
    printf("Test completed successfully.\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    free(array5);
    
    return 0;
}

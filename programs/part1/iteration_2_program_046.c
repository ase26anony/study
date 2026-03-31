/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1000
#define M 100

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double result;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(int n, double *arr) {
    double partial_sum = 0.0;
    
    #pragma omp parallel for reduction(+:partial_sum)
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
    }
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        
        #pragma omp for reduction(+:partial_sum) nowait
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
            
            /* This should generate OMP_CLAUSE_INCLUSIVE */
            #pragma omp scan inclusive(local_sum)
            arr[i] = local_sum;
        }
        
        /* Nested with exclusive scan */
        #pragma omp single
        {
            double exclusive_scan = 0.0;
            for (int i = 0; i < 10; i++) {
                /* This should generate OMP_CLAUSE_EXCLUSIVE */
                #pragma omp scan exclusive(exclusive_scan)
                exclusive_scan += i * 0.1;
            }
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *data) {
    /* This should generate OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: data->values[0:N]) \
                           map(to: data->indices) \
                           map(alloc: data->result)
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: data->values[0:N])
    for (int i = 0; i < N; i++) {
        data->values[i] = data->values[i] * 2.0 + i;
    }
    
    #pragma omp exit data map(from: data->result) \
                          map(release: data->values)
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temporaries(int n, double *a, double *b, double *c) {
    double tmp = 0.0;
    int last_val = 0;
    
    /* Complex parallel loop with multiple clauses - may generate _LOOPTEMP_ */
    #pragma omp parallel for reduction(+:tmp) lastprivate(last_val) \
                linear(i:1) collapse(2) if(n > 500)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < M; j++) {
            a[i * M + j] = i * j * 0.1;
            b[i * M + j] = a[i * M + j] * 2.0;
            tmp += a[i * M + j];
            last_val = i * M + j;
        }
    }
    
    /* SIMD loop with conditionals - may generate _CONDTEMP_ */
    #pragma omp simd reduction(+:tmp) linear(i:1) \
                simdlen(8) if(n > 100)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
        if (c[i] > 100.0) {
            tmp += 1.0;
        }
    }
    
    /* Nested reduction - may generate _REDUCTEMP_ */
    #pragma omp parallel reduction(+:tmp)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < 10; j++) {
                tmp += a[i] * j;
            }
        }
    }
}

/* Function 4: Scan directive in loop - may generate _SCANTEMP_ */
void test_scan_directive(int n, double *arr) {
    double scan_var = 0.0;
    
    #pragma omp parallel
    {
        #pragma omp for ordered(1)
        for (int i = 0; i < n; i++) {
            double local = arr[i] * i;
            
            /* Scan directive that may generate internal temporaries */
            #pragma omp ordered depend(sink: i-1)
            scan_var += local;
            arr[i] = scan_var;
            #pragma omp ordered depend(source)
            
            /* Additional scan with inclusive */
            #pragma omp scan inclusive(scan_var)
            scan_var += 0.1;
        }
    }
}

/* Function 5: Target regions with complex data environment */
void test_target_regions(struct DataBlock *data, int n) {
    /* Enter data with structured block */
    #pragma omp target enter data map(to: data[0:2])
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: data[0].values[0:n]) \
                reduction(+: data[0].result)
    for (int i = 0; i < n; i++) {
        data[0].values[i] = data[0].values[i] * 3.14;
        data[0].result += data[0].values[i];
    }
    
    /* Nested target region */
    #pragma omp target map(tofrom: data[1].result) \
                       map(to: data[1].values[0:n])
    {
        #pragma omp parallel for reduction(+:data[1].result)
        for (int i = 0; i < n; i++) {
            data[1].values[i] = data[0].values[i] * 2.0;
            data[1].result += data[1].values[i];
        }
    }
    
    #pragma omp target exit data map(from: data[0].result) \
                                 map(release: data[0].values)
}

int main() {
    /* Allocate and initialize test data */
    double *array1 = (double*)malloc(N * M * sizeof(double));
    double *array2 = (double*)malloc(N * M * sizeof(double));
    double *array3 = (double*)malloc(N * sizeof(double));
    
    struct DataBlock *data = (struct DataBlock*)malloc(2 * sizeof(struct DataBlock));
    
    /* Initialize arrays */
    for (int i = 0; i < N * M; i++) {
        array1[i] = 0.0;
        array2[i] = (double)i;
    }
    
    for (int i = 0; i < N; i++) {
        array3[i] = i * 0.25;
    }
    
    /* Call functions with different OpenMP constructs */
    test_scan_clauses(N, array3);
    
    test_enter_data(&data[0]);
    
    test_internal_temporaries(N, array1, array2, array3);
    
    test_scan_directive(N, array3);
    
    test_target_regions(data, N);
    
    /* Final computation with reduction */
    double final_result = 0.0;
    #pragma omp parallel for reduction(+:final_result) \
                if(N > 100) schedule(dynamic, 16)
    for (int i = 0; i < N; i++) {
        final_result += array3[i] + data[0].values[i % N];
    }
    
    printf("Final result: %f\n", final_result);
    printf("Data[0].result: %f\n", data[0].result);
    printf("Data[1].result: %f\n", data[1].result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(data);
    
    return 0;
}

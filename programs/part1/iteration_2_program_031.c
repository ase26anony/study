/* Test program to trigger uncovered OpenMP clause pretty-printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1000
#define CHUNK 64

/* Structure to test complex data mapping */
struct DataBlock {
    double values[N];
    int indices[N];
    double sum;
};

/* Function 1: Uses scan inclusive/exclusive clauses */
void test_scan_clauses(double *arr, int n) {
    double partial_sum = 0.0;
    double prefix_sum[N];
    
    #pragma omp parallel for reduction(+:partial_sum) schedule(static, CHUNK)
    for (int i = 0; i < n; i++) {
        partial_sum += arr[i];
    }
    
    /* This should trigger OMP_CLAUSE_INCLUSIVE and OMP_CLAUSE_EXCLUSIVE */
    double scan_temp = 0.0;
    #pragma omp parallel for reduction(+:scan_temp)
    for (int i = 0; i < n; i++) {
        #pragma omp scan inclusive(scan_temp)
        scan_temp += arr[i];
        prefix_sum[i] = scan_temp;
        
        if (i % 2 == 0) {
            double temp = 0.0;
            #pragma omp scan exclusive(temp)
            temp = prefix_sum[i];
            arr[i] = temp;
        }
    }
}

/* Function 2: Uses enter data with to mapper */
void test_enter_data(struct DataBlock *block) {
    /* This should trigger OMP_CLAUSE_ENTER with OMP_CLAUSE_ENTER_TO */
    #pragma omp enter data map(to: block[:1]) \
        map(to: block->values[:N]) \
        map(to: block->indices[:N])
    
    #pragma omp target teams distribute parallel for \
        map(tofrom: block->values[:N]) \
        map(to: block->indices[:N])
    for (int i = 0; i < N; i++) {
        block->values[i] = block->indices[i] * 1.5;
    }
    
    #pragma omp exit data map(from: block->sum) \
        map(release: block->values[:N], block->indices[:N])
}

/* Function 3: Complex loops to generate internal temporary clauses */
void test_internal_temps(double *a, double *b, double *c, int n) {
    double reduction_temp = 0.0;
    int last_val = 0;
    
    /* Complex loop with multiple clauses - may generate _LOOPTEMP_, _REDUCTEMP_ */
    #pragma omp parallel for simd \
        reduction(+:reduction_temp) \
        lastprivate(last_val) \
        linear(i:1) \
        schedule(static) \
        collapse(1) \
        aligned(a, b, c: 32)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
        reduction_temp += c[i];
        last_val = i;
        
        /* Nested conditional to potentially generate _CONDTEMP_ */
        if (i % 3 == 0) {
            #pragma omp simd reduction(+:reduction_temp)
            for (int j = 0; j < 4; j++) {
                c[i] += j * 0.1;
            }
        }
    }
    
    /* Another complex construct */
    #pragma omp target teams distribute parallel for simd \
        map(to: a[:n], b[:n]) \
        map(from: c[:n]) \
        reduction(max:reduction_temp) \
        lastprivate(last_val)
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
        if (c[i] > reduction_temp) {
            reduction_temp = c[i];
        }
        last_val = i;
    }
}

/* Function 4: Scan with conditionals for _SCANTEMP_ generation */
void test_scan_with_conditionals(int *data, int n) {
    int scan_var = 0;
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        /* Multiple scan directives with conditionals */
        if (data[i] > 0) {
            #pragma omp scan inclusive(scan_var)
            scan_var += data[i];
            data[i] = scan_var;
        } else {
            #pragma omp scan exclusive(scan_var)
            data[i] = scan_var;
            scan_var -= data[i];
        }
    }
}

/* Function 5: Mixed constructs with if clauses */
void test_mixed_constructs(struct DataBlock *block, int threshold) {
    /* Conditional parallel region */
    #pragma omp parallel if(block->sum > threshold) \
        default(none) \
        shared(block) \
        firstprivate(threshold)
    {
        #pragma omp for nowait \
            reduction(+:block->sum) \
            lastprivate(threshold)
        for (int i = 0; i < N; i++) {
            block->sum += block->values[i];
        }
        
        #pragma omp single
        {
            /* Nested scan */
            double local_scan = 0.0;
            #pragma omp simd
            for (int i = 0; i < 10; i++) {
                #pragma omp scan inclusive(local_scan)
                local_scan += block->values[i];
            }
        }
    }
}

int main() {
    /* Initialize data */
    double *a = (double*)malloc(N * sizeof(double));
    double *b = (double*)malloc(N * sizeof(double));
    double *c = (double*)malloc(N * sizeof(double));
    int *data = (int*)malloc(N * sizeof(int));
    
    struct DataBlock block;
    
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.0;
        b[i] = i * 0.5;
        c[i] = 0.0;
        data[i] = (i % 2 == 0) ? i : -i;
        block.values[i] = i * 2.0;
        block.indices[i] = i;
    }
    block.sum = 0.0;
    
    /* Call functions to trigger various OpenMP constructs */
    test_scan_clauses(a, N);
    test_enter_data(&block);
    test_internal_temps(a, b, c, N);
    test_scan_with_conditionals(data, N);
    test_mixed_constructs(&block, 500);
    
    /* Final computation and output */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        if(N > 100)  /* Conditional to affect gimplification */
    for (int i = 0; i < N; i++) {
        final_sum += c[i] + data[i];
    }
    
    printf("Final result: %f\n", final_sum);
    printf("Block sum: %f\n", block.sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(data);
    
    return 0;
}

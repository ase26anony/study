/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all test_omp_internal_clauses.c -o test_omp_internal_clauses */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd: int: omp_out += omp_in) \
    initializer(omp_priv = 0)

/* Initialize target data */
void init_data(int *arr, int n) {
    #pragma omp target teams distribute parallel for simd if(n > 1000)
    for (int i = 0; i < n; i++) {
        arr[i] = i % 100;
    }
}

int main(int argc, char *argv[]) {
    int i, n = 1000;
    volatile int sink; /* Prevent optimizations */
    
    /* Use argc for runtime-dependent behavior */
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:n])
    
    int *data = (int *)malloc(n * sizeof(int));
    for (i = 0; i < n; i++) {
        data[i] = (i * 3) % 97;
    }
    
    /* 1. Complex reduction with multiple variables - may generate _reductemp_ */
    int sum1 = 0, sum2 = 0, sum3 = 0;
    volatile int cond = (argc > 2); /* Volatile condition */
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2, sum3) \
        if(cond) /* May generate _condtemp_ */
    for (i = 0; i < n; i++) {
        sum1 += data[i];
        sum2 += data[i] * 2;
        sum3 += data[i] / 2;
    }
    
    sink = sum1 + sum2 + sum3;
    printf("Reduction sums: %d %d %d\n", sum1, sum2, sum3);
    
    /* 2. Array reduction - more likely to generate temporaries */
    int arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (i = 0; i < n; i++) {
        arr_sum[i % 10] += data[i];
    }
    
    /* 3. Scan directive - should generate _scantemp_ */
    int scan_sum = 0;
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < n; i++) {
        #pragma omp scan exclusive(scan_sum)
        scan_sum += data[i];
    }
    
    sink = scan_sum;
    printf("Scan sum: %d\n", scan_sum);
    
    /* 4. Nested reductions with custom reduction */
    int custom_sum = 0;
    int max_val = -1000000;
    int min_val = 1000000;
    
    #pragma omp parallel sections reduction(myadd:custom_sum) \
        reduction(max:max_val) reduction(min:min_val)
    {
        #pragma omp section
        {
            for (i = 0; i < n/2; i++) {
                custom_sum += data[i];
                if (data[i] > max_val) max_val = data[i];
                if (data[i] < min_val) min_val = data[i];
            }
        }
        #pragma omp section
        {
            for (i = n/2; i < n; i++) {
                custom_sum += data[i];
                if (data[i] > max_val) max_val = data[i];
                if (data[i] < min_val) min_val = data[i];
            }
        }
    }
    
    printf("Custom reduction: %d, Min: %d, Max: %d\n", custom_sum, min_val, max_val);
    
    /* 5. Complex condition with function call */
    int cond_sum = 0;
    #pragma omp parallel for reduction(+:cond_sum) \
        if(omp_get_num_threads() > 1 || argc > 1)
    for (i = 0; i < n; i++) {
        cond_sum += (data[i] > 50) ? 1 : 0;
    }
    
    sink = cond_sum;
    
    /* 6. Target region with enter/exit */
    int target_sum = 0;
    #pragma omp target teams distribute parallel for reduction(+:target_sum) \
        map(tofrom:target_sum)
    for (i = 0; i < n; i++) {
        target_sum += target_data[i];
    }
    
    printf("Target sum: %d\n", target_sum);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:n])
    
    free(data);
    
    /* Force output to prevent dead code elimination */
    printf("Final sink: %d\n", sink);
    
    return 0;
}

/* test_omp_internal_clauses.c */
/* Compile with: g++ -O3 -fopenmp -fopenmp-version=51 -fdump-tree-all-details -fdump-tree-omplower test_omp_internal_clauses.c -o test_omp_internal_clauses */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd : int : omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create non-trivial condition */
int check_threshold(volatile int *val) {
    return *val > 100;
}

int main(int argc, char **argv) {
    volatile int use_parallel = 1; /* Prevent optimization */
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    
    /* Initialize array with non-constant values */
    for (i = 0; i < N; i++) {
        target_data[i] = i * (argc > 1 ? atoi(argv[1]) : 1);
    }
    
    /* Enter data to target - triggers OMP_CLAUSE_ENTER */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex target region with reduction and condition - may generate _reductemp_ and _condtemp_ */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(argc > 2) /* Non-trivial condition */ \
        map(tofrom: sum, max_val)
    for (i = 0; i < N; i++) {
        sum += target_data[i];
        if (target_data[i] > max_val) {
            max_val = target_data[i];
        }
    }
    
    /* Print to prevent elimination */
    printf("Sum: %d, Max: %d\n", sum, max_val);
    
    /* Nested reductions with custom reduction */
    #pragma omp parallel for reduction(myadd:sum) reduction(min:min_val) \
        if(check_threshold(&use_parallel)) /* Volatile function call in condition */
    for (i = 0; i < N; i++) {
        sum += i;
        if (i < min_val) min_val = i;
    }
    
    printf("After custom reduction - Sum: %d, Min: %d\n", sum, min_val);
    
    /* Scan directive - triggers _scantemp_ */
    int partial_sums[1000];
    scan_sum = 0;
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        schedule(dynamic) /* Dynamic scheduling creates more complex lowering */
    for (i = 0; i < N; i++) {
        /* Exclusive scan */
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] = scan_sum;
        scan_sum += i % 10; /* Non-trivial increment */
    }
    
    printf("Scan sum: %d\n", scan_sum);
    
    /* Array reduction - may generate additional temporaries */
    int arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (i = 0; i < N; i++) {
        arr_sum[i % 10] += i;
    }
    
    /* Nowait clause creates async execution */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum)
        for (i = 0; i < 100; i++) {
            sum += i * 2;
        }
        
        #pragma omp for reduction(max:max_val)
        for (i = 0; i < 100; i++) {
            if (i > max_val) max_val = i;
        }
    }
    
    /* Final with non-trivial condition */
    #pragma omp parallel final(argc > 3) /* May generate condition temporaries */
    {
        #pragma omp for reduction(+:sum)
        for (i = 0; i < 50; i++) {
            sum += i * 3;
        }
    }
    
    printf("Final sum: %d\n", sum);
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    return 0;
}

/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all-details test.c */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int global_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd: int: omp_out = omp_out + omp_in) \
    initializer(omp_priv = 0)

/* Function to create runtime-dependent conditions */
int get_threshold(int argc) {
    volatile int threshold = 100; /* volatile to prevent optimization */
    return argc > 1 ? threshold : 50;
}

int main(int argc, char *argv[]) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    volatile int sink; /* Prevent dead code elimination */
    
    /* Initialize array on host */
    for (i = 0; i < N; i++) {
        global_data[i] = i % 100;
    }
    
    /* Enter data to device - triggers OMP_CLAUSE_ENTER */
    #pragma omp target enter data map(to: global_data[0:N])
    
    /* Complex target region with reduction and if clause 
       May generate _reductemp_ and _condtemp_ */
    int threshold = get_threshold(argc);
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) reduction(max:max_val) \
        if(target: argc > 1) map(tofrom: sum, max_val)
    for (i = 0; i < N; i++) {
        sum += global_data[i];
        if (global_data[i] > max_val) {
            max_val = global_data[i];
        }
    }
    
    sink = sum + max_val; /* Use results */
    
    /* Nested reductions with custom reduction operator
       May generate additional _reductemp_ clauses */
    int sum2 = 0, sum3 = 0;
    #pragma omp parallel for reduction(myadd:sum2) reduction(+:sum3) \
        if(parallel: threshold > 50) schedule(dynamic, 16)
    for (i = 0; i < N; i++) {
        sum2 += i;
        sum3 += global_data[i % N];
    }
    
    sink += sum2 + sum3;
    
    /* Scan directive - triggers OMP_CLAUSE__SCANTEMP_ 
       Requires OpenMP 5.0+ */
    int partial_sums[1000];
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < N; i++) {
        /* Exclusive scan */
        #pragma omp scan exclusive(scan_sum)
        partial_sums[i] = scan_sum;
        scan_sum += global_data[i];
    }
    
    sink += scan_sum + partial_sums[N-1];
    
    /* Array reduction - may generate complex temporaries */
    int arr_sum[10] = {0};
    #pragma omp parallel for reduction(+:arr_sum[:10])
    for (i = 0; i < N; i++) {
        arr_sum[i % 10] += global_data[i];
    }
    
    for (i = 0; i < 10; i++) {
        sink += arr_sum[i];
    }
    
    /* Final conditional parallel region */
    #pragma omp parallel if(threshold > 10) reduction(min:min_val)
    {
        #pragma omp for nowait
        for (i = 0; i < N; i++) {
            if (global_data[i] < min_val) {
                min_val = global_data[i];
            }
        }
    }
    
    sink += min_val;
    
    /* Exit data from device */
    #pragma omp target exit data map(from: global_data[0:N])
    
    printf("Results: sum=%d, max=%d, min=%d, scan_sum=%d, sink=%d\n",
           sum, max_val, min_val, scan_sum, sink);
    
    return 0;
}

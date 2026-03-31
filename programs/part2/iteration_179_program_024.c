/* test_omp_clauses.c - Test program for OpenMP internal clause pretty-printing */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Global data for declare target enter clause */
int global_data[1000];
#pragma omp declare target enter(global_data)

/* Custom reduction for complex cases */
#pragma omp declare reduction(myadd: int: omp_out += omp_in) \
    initializer(omp_priv = 0)

/* Function to create runtime-dependent conditions */
int get_threshold(int argc) {
    volatile int base = 100; /* volatile to prevent optimization */
    return base + argc * 50;
}

int main(int argc, char *argv[]) {
    int i, sum = 0, max_val = -1000, min_val = 1000;
    int scan_sum = 0;
    int N = 1000;
    volatile int sink; /* volatile sink to prevent dead code elimination */
    
    /* Initialize arrays with runtime-dependent values */
    for (i = 0; i < N; i++) {
        global_data[i] = (i + argc) % 100;
    }
    
    /* ============================================
       Test 1: Target teams with reduction and if clause
       Should generate _reductemp_ and _condtemp_ clauses
       ============================================ */
    int target_sum = 0;
    int threshold = get_threshold(argc);
    
    #pragma omp target teams distribute parallel for simd \
        reduction(+:target_sum) \
        if(target: argc > 1) \
        map(tofrom: target_sum) \
        map(to: global_data[0:N])
    for (i = 0; i < N; i++) {
        target_sum += global_data[i];
    }
    
    sink = target_sum; /* Prevent optimization */
    printf("Target sum: %d\n", target_sum);
    
    /* ============================================
       Test 2: Parallel for with multiple reductions
       Should generate multiple _reductemp_ clauses
       ============================================ */
    int sum1 = 0, sum2 = 0;
    
    #pragma omp parallel for reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        schedule(dynamic, 16) \
        if(parallel: argc > 2)
    for (i = 0; i < N; i++) {
        int val = global_data[i];
        sum1 += val;
        sum2 += val * 2;
        if (val > max_val) max_val = val;
        if (val < min_val) min_val = val;
    }
    
    printf("Sum1: %d, Sum2: %d, Max: %d, Min: %d\n", sum1, sum2, max_val, min_val);
    
    /* ============================================
       Test 3: SIMD with inscan reduction (OpenMP 5.1)
       Should generate _scantemp_ clauses
       ============================================ */
    int scan_array[100];
    for (i = 0; i < 100; i++) {
        scan_array[i] = i + 1;
    }
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        scan(inscan:scan_sum)
    for (i = 0; i < 100; i++) {
        scan_sum += scan_array[i];
        #pragma omp scan inclusive(scan_sum)
        scan_array[i] = scan_sum; /* Use scan result */
    }
    
    printf("Scan sum: %d\n", scan_sum);
    
    /* ============================================
       Test 4: Nested parallel with custom reduction
       Complex case that may generate additional temporaries
       ============================================ */
    int custom_sum = 0;
    
    #pragma omp parallel reduction(myadd:custom_sum) \
        if(omp_get_max_threads() > 1)
    {
        #pragma omp for nowait
        for (i = 0; i < N; i++) {
            custom_sum += global_data[i] % 10;
        }
    }
    
    printf("Custom reduction sum: %d\n", custom_sum);
    
    /* ============================================
       Test 5: Array reduction (OpenMP 5.1)
       May generate complex reduction temporaries
       ============================================ */
    int arr_sum[5] = {0, 0, 0, 0, 0};
    
    #pragma omp parallel for reduction(+:arr_sum[:5]) \
        if(argc > 3)
    for (i = 0; i < N; i++) {
        int idx = global_data[i] % 5;
        arr_sum[idx] += 1;
    }
    
    printf("Array reduction: ");
    for (i = 0; i < 5; i++) {
        printf("%d ", arr_sum[i]);
    }
    printf("\n");
    
    /* ============================================
       Test 6: Taskloop with final clause
       May generate condition temporaries
       ============================================ */
    int task_sum = 0;
    volatile int final_condition = (argc > 4);
    
    #pragma omp parallel
    #pragma omp single
    #pragma omp taskloop reduction(+:task_sum) \
        final(final_condition > 0) \
        grainsize(10)
    for (i = 0; i < N; i++) {
        task_sum += global_data[i] / 2;
    }
    
    printf("Task sum: %d\n", task_sum);
    
    return 0;
}

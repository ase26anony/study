/* test_omp_internal_clauses.c */
/* Compile with: gcc -O2 -fopenmp -fopenmp-version=51 -fdump-tree-omplower -fdump-tree-all test_omp_internal_clauses.c -o test_omp */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* For OMP_CLAUSE_ENTER */
#pragma omp declare target
int target_data[1000];
#pragma omp end declare target

/* Custom reduction for complex cases */
#pragma omp declare reduction(complex_add : double : omp_out += omp_in + 1.0) \
    initializer(omp_priv = 0.0)

int main(int argc, char **argv) {
    int i;
    volatile int use_argc = argc; /* Prevent optimization */
    int N = 1000;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        target_data[i] = i;
    }
    
    /* Enter data to target (triggers OMP_CLAUSE_ENTER) */
    #pragma omp target enter data map(to: target_data[0:N])
    
    /* Complex reduction with multiple variables (may trigger _reductemp_) */
    double sum1 = 0.0, sum2 = 0.0;
    double max_val = -1e30, min_val = 1e30;
    
    /* Combined construct with reduction and if clause (may trigger _condtemp_) */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum1, sum2) \
        reduction(max:max_val) reduction(min:min_val) \
        if(use_argc > 1) map(tofrom: target_data[0:N])
    for (i = 0; i < N; i++) {
        target_data[i] += 1;
        sum1 += target_data[i];
        sum2 += target_data[i] * 0.5;
        if (target_data[i] > max_val) max_val = target_data[i];
        if (target_data[i] < min_val) min_val = target_data[i];
    }
    
    /* Use volatile to prevent optimization */
    volatile double sink1 = sum1;
    volatile double sink2 = sum2;
    
    /* Scan directive (triggers _scantemp_) - OpenMP 5.1 feature */
    double scan_sum = 0.0;
    double scan_array[N];
    
    #pragma omp parallel for simd reduction(inscan, +:scan_sum) \
        private(i) schedule(static, 16)
    for (i = 0; i < N; i++) {
        scan_sum += i;
        #pragma omp scan inclusive(scan_sum)
        scan_array[i] = scan_sum;
    }
    
    /* Nested reduction with custom operator */
    double custom_reduce = 0.0;
    #pragma omp parallel for reduction(complex_add:custom_reduce) \
        if(use_argc > 2)  /* Another condition */
    for (i = 0; i < N; i++) {
        custom_reduce += i * 0.1;
    }
    
    /* Array reduction (OpenMP 5.1) - may generate additional temporaries */
    double arr_reduce[10] = {0};
    #pragma omp parallel for reduction(+:arr_reduce[:10])
    for (i = 0; i < N; i++) {
        arr_reduce[i % 10] += i * 0.01;
    }
    
    /* Nowait clause creates complex scheduling */
    #pragma omp parallel
    {
        #pragma omp for nowait reduction(+:sum1)
        for (i = 0; i < N/2; i++) {
            sum1 += i;
        }
        
        #pragma omp for reduction(+:sum2)
        for (i = N/2; i < N; i++) {
            sum2 += i;
        }
    }
    
    /* Exit data from target */
    #pragma omp target exit data map(from: target_data[0:N])
    
    /* Print results to prevent dead code elimination */
    printf("Results: sum1=%.2f, sum2=%.2f, max=%.2f, min=%.2f\n", 
           sum1, sum2, max_val, min_val);
    printf("Scan sum=%.2f, custom=%.2f\n", scan_sum, custom_reduce);
    printf("Array reduce[0]=%.2f\n", arr_reduce[0]);
    
    return 0;
}

/* Compile with: g++ -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -o omp_test tree-pretty-print-test.cc */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#ifdef DUMP_OMP
/* Dummy function to hint compiler about OpenMP clause types */
void __attribute__((noinline)) dump_omp_clause(int clause_type) {
    fprintf(stderr, "Clause type: %d\n", clause_type);
}
#endif

/* Function to be used with declare target enter */
void __attribute__((noinline)) vector_add(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Declare target with enter clause and to mapper - triggers OMP_CLAUSE_ENTER */
#pragma omp declare target enter(vector_add) to(map_to_from: vector_add)

int main(int argc, char *argv[]) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Prevent optimization with volatile and runtime values */
    volatile int vol_size = 100 + (rand() % 100);
    int n = vol_size;
    int m = 50 + (rand() % 50);
    
    /* Allocate arrays */
    double *array1 = (double*)malloc(n * sizeof(double));
    double *array2 = (double*)malloc(n * sizeof(double));
    int *arr_int1 = (int*)malloc(n * sizeof(int));
    int *arr_int2 = (int*)malloc(n * sizeof(int));
    int *arr_int3 = (int*)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        array1[i] = i * 1.5;
        array2[i] = 0.0;
        arr_int1[i] = i % 10;
        arr_int2[i] = (i % 7) + 1;
    }
    
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* 1. OpenMP parallel for simd with reduction and scan - may generate _reductemp_ and _scantemp_ */
    #pragma omp parallel for simd reduction(+:sum) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = array1[i];
        
        /* Inscan phase for prefix sum */
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        /* Reduction */
        sum += val;
        
        /* Data-dependent operation */
        if (prefix_sum > 100.0) {
            array2[i] = prefix_sum / (i + 1);
        } else {
            array2[i] = val;
        }
    }
    
    /* 2. Nested loop with collapse and volatile bound - may generate _condtemp_ */
    volatile int outer_bound = m;
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < outer_bound; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < n) {
                /* Complex condition that might require condition temporaries */
                if ((i > j) && (omp_get_thread_num() % 2 == 0)) {
                    arr_int1[idx] *= 2;
                }
            }
        }
    }
    
    /* 3. Target region with entered function */
    #pragma omp target map(to: arr_int1[0:n], arr_int2[0:n]) map(from: arr_int3[0:n])
    {
        /* Call the function that was entered via declare target */
        vector_add(arr_int1, arr_int2, arr_int3, n);
        
        /* Additional computation in target region */
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += arr_int3[i] * 0.01;
        }
    }
    
    /* 4. Teams distribute with reduction */
    #pragma omp target teams distribute parallel for reduction(+:sum) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        sum += array2[i] * 0.5;
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < n; i += 7) {
        checksum += array2[i] + arr_int3[i % n];
    }
    
    printf("Checksum: %f, Sum: %f\n", checksum, sum);
    
    /* Conditional compilation for debugging */
    #ifdef DUMP_OMP
    /* These calls hint at the clause types but don't directly call pretty-printer */
    dump_omp_clause(0);  /* Placeholder for clause types */
    #endif
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(arr_int1);
    free(arr_int2);
    free(arr_int3);
    
    return 0;
}

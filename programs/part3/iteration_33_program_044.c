/* Compile with: g++ -O1 -fopenmp -fdump-tree-omplower -fdump-tree-all -foffload=disable tree-pretty-print-test.cc -o test */
/* Also try: g++ -O0 -fopenmp -foffload-abi=lp64 -fdump-tree-original tree-pretty-print-test.cc -o test */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#ifdef DUMP_OMP
/* Dummy function to hint compiler about OpenMP clause types */
void __attribute__((noinline)) dump_omp_clause(int clause_type) {
    volatile int x = clause_type;
    printf("Clause type hint: %d\n", x);
}
#endif

/* Function to be used with declare target enter */
void __attribute__((noinline)) vector_add(int *a, int *b, int *c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Declare target with enter clause and to mapper - triggers OMP_CLAUSE_ENTER */
#pragma omp declare target enter(vector_add) to(map_to_from:vector_add)

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int base_size = 100 + (argc * 17) % 50;
    
    /* Volatile variables to prevent optimization */
    volatile int vol_bound = base_size;
    volatile int seed = argc;
    
    /* Allocate arrays with runtime sizes */
    int size1 = base_size * 2;
    int size2 = base_size * 3;
    
    double *array1 = (double*)malloc(size1 * sizeof(double));
    int *array2 = (int*)malloc(size2 * sizeof(int));
    int *array3 = (int*)malloc(size2 * sizeof(int));
    int *result = (int*)malloc(size2 * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < size1; i++) {
        array1[i] = i * 1.5;
    }
    
    for (int i = 0; i < size2; i++) {
        array2[i] = i % 10;
        array3[i] = (i + 3) % 7;
    }
    
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* OpenMP parallel for simd with reduction and scan - may generate _reductemp_ and _scantemp_ */
    #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) if(size1 > 50) \
            scan(inscan:prefix_sum)
    for (int i = 0; i < size1; i++) {
        double val = array1[i];
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        
        sum += val;
        
        /* Data-dependent operation to create complex control flow */
        if (i % (omp_get_thread_num() + 2) == 0) {
            array1[i] = val * 2.0;
        } else {
            array1[i] = val / 2.0;
        }
    }
    
    /* Nested OpenMP loop with collapse and volatile bound - may generate _condtemp_ */
    int rows = vol_bound;
    int cols = base_size / 2 + 1;
    
    #pragma omp parallel for collapse(2) \
            if(rows > 10 && cols > 10) \
            schedule(dynamic, 4)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            if (idx < size2) {
                /* Complex expression that might need condition temporaries */
                array2[idx] = (i * j + (i > j ? i : j)) % 100;
            }
        }
    }
    
    /* Target region using the function with declare target enter */
    #pragma omp target map(to: array2[0:size2], array3[0:size2]) \
                     map(from: result[0:size2]) \
                     if(size2 > 50)
    {
        /* Call the function that was entered via declare target */
        vector_add(array2, array3, result, size2);
        
        /* Additional computation in target region */
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < size2; i++) {
            result[i] += omp_get_thread_num();
            sum += result[i] * 0.01;
        }
    }
    
    /* Teams distribute parallel for with reduction - complex nesting */
    #pragma omp target teams distribute parallel for \
            map(tofrom: array1[0:size1]) \
            reduction(+:sum) \
            num_teams(2) thread_limit(32) \
            if(size1 > 30)
    for (int i = 0; i < size1; i++) {
        array1[i] = array1[i] * (1.0 + 0.01 * omp_get_team_num());
        sum += array1[i];
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < size1 && i < 20; i++) {
        checksum += array1[i];
    }
    for (int i = 0; i < size2 && i < 20; i++) {
        checksum += result[i];
    }
    
    printf("Checksum: %f, Final sum: %f\n", checksum, sum);
    
    #ifdef DUMP_OMP
    /* Hint compiler about various OpenMP clause types */
    dump_omp_clause(0);  /* Placeholder for clause types */
    #endif
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(result);
    
    return 0;
}

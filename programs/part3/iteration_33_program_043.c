/* Compile with: g++ -O1 -fopenmp -fdump-tree-omplower -fdump-tree-ompexp -foffload=disable tree-coverage.cc -o tree-coverage */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#ifdef DUMP_OMP
/* Dummy function to hint compiler about OpenMP clause types */
void __attribute__((noinline)) dump_omp_clause(int clause_type) {
    volatile int x = clause_type;
    (void)x;
}
#endif

/* Function to be used with declare target enter */
void __attribute__((noinline)) vector_add(int *a, int *b, int *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Declare target with enter clause and to mapper */
#pragma omp declare target enter(vector_add) to(map_to_from: vector_add)

int main(int argc, char **argv) {
    /* Use argc for pseudo-random but reproducible sizes */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    srand(seed);
    
    /* Prevent optimization with volatile and runtime values */
    volatile int base_size = 100 + (rand() % 50);
    int n = base_size;
    int m = 50 + (rand() % 30);
    
    /* Allocate arrays */
    double *array1 = (double*)malloc(n * sizeof(double));
    double *array2 = (double*)malloc(n * sizeof(double));
    int *int_array1 = (int*)malloc(n * sizeof(int));
    int *int_array2 = (int*)malloc(n * sizeof(int));
    int *int_array3 = (int*)malloc(n * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        array1[i] = i * 1.5;
        array2[i] = 0.0;
        int_array1[i] = i % 10;
        int_array2[i] = (i + 3) % 7;
    }
    
    double sum = 0.0;
    double prefix_sum = 0.0;
    
    /* 1. SIMD with reduction and scan - targets _reductemp_ and _scantemp_ */
    #pragma omp parallel for simd reduction(+:sum) \
            simdlen(4) linear(prefix_sum:1) scan(inscan:prefix_sum)
    for (int i = 0; i < n; i++) {
        double val = array1[i];
        sum += val;
        
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        array2[i] = prefix_sum;
    }
    
    /* 2. Nested loop with collapse and volatile bound - may generate _condtemp_ */
    volatile int outer_bound = m;
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int i = 0; i < outer_bound; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < n) {
                /* Data-dependent operation with thread ID */
                if (omp_get_thread_num() % 2 == 0) {
                    int_array1[idx] += j;
                } else {
                    int_array1[idx] -= j;
                }
            }
        }
    }
    
    /* 3. Complex reduction with nested parallelism */
    double nested_sum = 0.0;
    #pragma omp parallel reduction(+:nested_sum)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            double local_sum = 0.0;
            /* Inner loop to create reduction temporary */
            for (int j = 0; j < 5; j++) {
                local_sum += array1[i] * j;
            }
            nested_sum += local_sum;
        }
        
        /* Additional work to keep parallel region alive */
        #pragma omp for
        for (int i = 0; i < n/2; i++) {
            array2[i] *= 1.01;
        }
    }
    
    /* 4. Target region with entered function */
    #pragma omp target map(to: int_array1[0:n], int_array2[0:n]) \
                      map(from: int_array3[0:n]) if(n > 50)
    {
        #pragma omp teams distribute parallel for simd \
                reduction(+:sum) num_teams(2)
        for (int i = 0; i < n; i++) {
            int_array3[i] = int_array1[i] * int_array2[i];
        }
        
        /* Call the entered function */
        vector_add(int_array1, int_array2, int_array3, n/2);
    }
    
    /* 5. Task with depend clause (may generate additional temporaries) */
    double task_sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: task_sum) shared(task_sum)
            {
                for (int i = 0; i < n; i++) {
                    task_sum += array2[i];
                }
            }
            
            #pragma omp task depend(in: task_sum) shared(task_sum)
            {
                task_sum *= 0.5;
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = sum + nested_sum + task_sum + prefix_sum;
    for (int i = 0; i < n; i++) {
        checksum += int_array3[i] * 0.001;
    }
    
    printf("Checksum: %f\n", checksum);
    
    #ifdef DUMP_OMP
    /* Hint compiler about clause types */
    dump_omp_clause(0);  /* OMP_CLAUSE__REDUCTEMP_ */
    dump_omp_clause(1);  /* OMP_CLAUSE__CONDTEMP_ */
    dump_omp_clause(2);  /* OMP_CLAUSE__SCANTEMP_ */
    dump_omp_clause(3);  /* OMP_CLAUSE_ENTER */
    #endif
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(int_array1);
    free(int_array2);
    free(int_array3);
    
    return 0;
}

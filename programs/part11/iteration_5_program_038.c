/* test_openmp_clauses.c
 * Targets uncovered lines in tree-pretty-print.cc:1434-1445
 * OMP_CLAUSE_FOR, OMP_CLAUSE_PARALLEL, OMP_CLAUSE_SECTIONS, OMP_CLAUSE_TASKGROUP
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_data(int n, double *data) {
    double sum = 0.0;
    double max_val = -1e30;
    
    /* 1. OMP_CLAUSE_FOR - in combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(sum) if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            data[i] = i * 0.1 + j * 0.01;
        }
    }
    
    /* Force diagnostic with clause name "for" */
    #pragma omp error severity(warning) message("Testing clause: for")
    
    /* 2. OMP_CLAUSE_PARALLEL - combined with for clause */
    #pragma omp parallel for schedule(dynamic) num_threads(4) \
        reduction(+:sum) if(omp_get_num_procs() > 1)
    for (int i = 0; i < n; i++) {
        sum += data[i];
        /* Runtime call inside parallel region */
        if (omp_get_thread_num() == 0) {
            data[i] *= 1.001;
        }
    }
    
    /* 3. OMP_CLAUSE_SECTIONS - combined with parallel clause */
    #pragma omp parallel sections private(max_val) shared(data, n) \
        reduction(max:max_val)
    {
        #pragma omp section
        {
            max_val = data[0];
            for (int i = 1; i < n/2; i++) {
                if (data[i] > max_val) max_val = data[i];
            }
            /* Nested diagnostic */
            #pragma omp error severity(message) message("In sections clause region")
        }
        
        #pragma omp section
        {
            double local_max = data[n/2];
            for (int i = n/2 + 1; i < n; i++) {
                if (data[i] > local_max) local_max = data[i];
            }
            #pragma omp critical
            {
                if (local_max > max_val) max_val = local_max;
            }
        }
    }
    
    /* 4. OMP_CLAUSE_TASKGROUP - with task_reduction argument */
    double task_sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int i = 0; i < 8; i++) {
                    #pragma omp task in_reduction(+:task_sum) shared(data, n)
                    {
                        int start = (n * i) / 8;
                        int end = (n * (i + 1)) / 8;
                        double local_sum = 0.0;
                        for (int j = start; j < end; j++) {
                            local_sum += data[j];
                        }
                        task_sum += local_sum;
                        
                        /* Complex macro with _Pragma to test pretty-printing */
                        #define EMIT_CLAUES _Pragma("omp error severity(warning) message(\"taskgroup clause active\")")
                        EMIT_CLAUES;
                    }
                }
            }
        }
    }
    
    printf("Processed: sum=%.2f, max=%.2f, task_sum=%.2f\n", 
           sum, max_val, task_sum);
}

/* Another function with mixed control flow and OpenMP */
void __attribute__((optimize("O0"))) complex_nesting(int size) {
    int *arr = (int*)malloc(size * sizeof(int));
    if (!arr) return;
    
    /* Switch statement containing OpenMP directives */
    for (int iter = 0; iter < 3; iter++) {
        switch (iter) {
            case 0:
                /* OMP_CLAUSE_FOR with multiple clauses */
                #pragma omp parallel for ordered schedule(guided) \
                    linear(i:1) lastprivate(arr)
                for (int i = 0; i < size; i++) {
                    arr[i] = i * iter;
                    #pragma omp ordered
                    {
                        if (i % 100 == 0) arr[i] *= 2;
                    }
                }
                break;
                
            case 1:
                /* OMP_CLAUSE_SECTIONS inside loop */
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        for (int i = 0; i < size/2; i++) {
                            arr[i] += omp_get_thread_num();
                        }
                    }
                    #pragma omp section
                    {
                        for (int i = size/2; i < size; i++) {
                            arr[i] -= omp_get_thread_num();
                        }
                    }
                }
                break;
                
            case 2:
                /* OMP_CLAUSE_TASKGROUP with nested tasks */
                int total = 0;
                #pragma omp parallel
                {
                    #pragma omp single
                    {
                        #pragma omp taskgroup task_reduction(+:total)
                        {
                            #pragma omp task in_reduction(+:total)
                            { total += 1; }
                            #pragma omp task in_reduction(+:total)
                            { total += 2; }
                            #pragma omp task in_reduction(+:total)
                            { total += 3; }
                        }
                    }
                }
                arr[0] = total;
                break;
        }
        
        /* Force diagnostic for each clause type */
        if (iter == 0) {
            #pragma omp error severity(warning) message("for clause was used")
        } else if (iter == 1) {
            #pragma omp error severity(warning) message("sections clause was used")
        } else {
            #pragma omp error severity(warning) message("taskgroup clause was used")
        }
    }
    
    /* Combined directive testing OMP_CLAUSE_PARALLEL and OMP_CLAUSE_FOR */
    #pragma omp parallel for simd schedule(nonmonotonic:dynamic) \
        aligned(arr:64) if(size > 10000)
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] % 100;
    }
    
    free(arr);
}

/* Main function with multiple OpenMP regions */
int main() {
    const int N = 10000;
    double *data = (double*)malloc(N * sizeof(double));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize OpenMP */
    omp_set_num_threads(4);
    
    /* Process data with all target clauses */
    process_data(N, data);
    
    /* Additional test with complex nesting */
    complex_nesting(5000);
    
    /* Final checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) \
        if(N > 100)  /* OMP_CLAUSE_FOR with if clause */
    for (int i = 0; i < N; i++) {
        checksum += data[i];
    }
    
    printf("Final checksum: %.6f\n", checksum);
    
    free(data);
    return 0;
}

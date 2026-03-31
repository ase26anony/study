/* test_omp_clauses.c - Targeting uncovered pretty-print lines for OMP clauses */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"), noinline)) 
process_with_omp_clauses(int n, double *arr, double *results) {
    double sum = 0.0, max_val = -1e30;
    int i, j;
    
    /* 1. Use 'for' clause in combined directive with arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
        private(i, j) shared(arr) reduction(+:sum) if(n > 1000)
    for (i = 0; i < n; i++) {
        for (j = 0; j < 10; j++) {
            arr[i * 10 + j] = (i + j) * 0.5;
            sum += arr[i * 10 + j];
        }
    }
    results[0] = sum;
    
    /* 2. Use 'parallel' clause in combined directive */
    #pragma omp parallel default(none) shared(arr, results, n) \
        reduction(max:max_val) num_threads(4)
    {
        int tid = omp_get_thread_num();
        #pragma omp for schedule(dynamic) nowait
        for (i = 0; i < n * 10; i++) {
            if (arr[i] > max_val) max_val = arr[i];
            arr[i] += tid * 0.01;  /* Small thread-specific modification */
        }
        
        /* Nested directive with 'parallel' clause */
        #pragma omp parallel if(omp_get_num_threads() > 1)
        {
            /* Empty but forces pretty-printing of 'parallel' clause */
        }
    }
    results[1] = max_val;
    
    /* 3. Use 'sections' clause in combined directive */
    double section_sum = 0.0, section_product = 1.0;
    #pragma omp parallel sections private(i) shared(arr, n) \
        reduction(+:section_sum) reduction(*:section_product)
    {
        #pragma omp section
        {
            /* First section computes sum of even indices */
            for (i = 0; i < n * 10; i += 2) {
                section_sum += arr[i];
            }
        }
        
        #pragma omp section  
        {
            /* Second section computes product of odd indices */
            for (i = 1; i < n * 10 && i < 100; i += 2) {
                section_product *= (arr[i] + 1.0);
            }
        }
        
        #pragma omp section
        {
            /* Third section with nested directive */
            #pragma omp parallel sections if(0)
            {
                #pragma omp section
                { /* Empty but forces sections clause representation */ }
            }
        }
    }
    results[2] = section_sum;
    results[3] = section_product;
    
    /* 4. Use 'taskgroup' clause with task_reduction */
    double task_reduction_sum = 0.0;
    #pragma omp parallel master
    {
        #pragma omp taskgroup task_reduction(+:task_reduction_sum)
        {
            for (i = 0; i < 8; i++) {
                #pragma omp task in_reduction(+:task_reduction_sum) \
                    firstprivate(i) shared(arr, n)
                {
                    double local_sum = 0.0;
                    int start = i * (n * 10 / 8);
                    int end = (i + 1) * (n * 10 / 8);
                    if (end > n * 10) end = n * 10;
                    
                    for (int k = start; k < end; k++) {
                        local_sum += arr[k];
                    }
                    task_reduction_sum += local_sum;
                    
                    /* Force diagnostic with clause name in message */
                    if (i == 3) {
                        /* This should trigger pretty-printing of 'for' clause */
                        #pragma omp error message("Task encountered for clause context")
                    }
                }
            }
        }
        
        /* Nested taskgroup */
        #pragma omp taskgroup
        {
            #pragma omp task
            {
                /* Empty task but ensures taskgroup clause is represented */
            }
        }
    }
    results[4] = task_reduction_sum;
}

/* Complex control flow with OpenMP directives */
void __attribute__((optimize("O0")))
complex_flow_with_clauses(int mode) {
    switch (mode) {
        case 1: {
            /* 'for' clause in switch case */
            double arr[100];
            #pragma omp parallel for simd schedule(guided)
            for (int i = 0; i < 100; i++) {
                arr[i] = i * 0.1;
            }
            
            /* Use _Pragma to create complex pattern */
            #define OMP_FOR_CLAUSE _Pragma("omp parallel for schedule(static)")
            OMP_FOR_CLAUSE
            for (int i = 0; i < 50; i++) {
                arr[i] *= 2.0;
            }
            break;
        }
        
        case 2: {
            /* 'sections' clause in nested loop */
            for (int outer = 0; outer < 3; outer++) {
                #pragma omp parallel sections if(outer > 0)
                {
                    #pragma omp section
                    { int x = outer * 2; }
                    
                    #pragma omp section
                    { int y = outer * 3; }
                }
            }
            break;
        }
            
        case 3: {
            /* 'taskgroup' clause in conditional */
            if (omp_get_max_threads() > 1) {
                #pragma omp taskgroup
                {
                    #pragma omp task
                    { /* Task work */ }
                }
            }
            break;
        }
    }
}

/* Function that forces diagnostics with clause names */
void __attribute__((optimize("O0")))
trigger_clause_diagnostics(void) {
    /* These should generate warnings with pretty-printed clause names */
    
    /* Diagnostic for 'for' clause */
    #pragma omp error severity(warning) message("Testing for clause pretty-printing")
    
    /* Diagnostic for 'parallel' clause */
    #pragma omp parallel
    {
        #pragma omp error severity(message) message("In parallel region")
    }
    
    /* Diagnostic for 'sections' clause */
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            #pragma omp error severity(warning) message("In sections construct")
        }
    }
    
    /* Diagnostic for 'taskgroup' clause */
    #pragma omp taskgroup
    {
        #pragma omp task
        {
            #pragma omp error severity(message) message("In taskgroup context")
        }
    }
}

int main(int argc, char *argv[]) {
    int n = 5000;
    if (argc > 1) n = atoi(argv[1]);
    
    double *arr = (double *)malloc(n * 10 * sizeof(double));
    double results[5] = {0};
    
    /* Process with all target OpenMP clauses */
    process_with_omp_clauses(n, arr, results);
    
    /* Exercise complex flow patterns */
    for (int mode = 1; mode <= 3; mode++) {
        complex_flow_with_clauses(mode);
    }
    
    /* Trigger diagnostics */
    trigger_clause_diagnostics();
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < 5; i++) {
        checksum += results[i];
    }
    
    /* Use OpenMP runtime calls in conditional directives */
    #pragma omp parallel if(omp_get_max_threads() > 1)
    {
        #pragma omp single
        {
            printf("OpenMP max threads: %d\n", omp_get_max_threads());
            printf("Results checksum: %f\n", checksum);
        }
    }
    
    free(arr);
    return 0;
}

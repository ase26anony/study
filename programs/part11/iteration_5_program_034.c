/* test_omp_clauses.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 1000
#define NUM_ITERATIONS 100

/* Function with optimization disabled to preserve OpenMP constructs */
void __attribute__((optimize("O0"))) process_array(double *arr, int n) {
    double sum = 0.0;
    double max_val = arr[0];
    
    /* 1. Combined parallel for clause - triggers 'parallel' and 'for' pretty-printing */
    #pragma omp parallel for schedule(dynamic) reduction(+:sum) \
        if(n > 100) num_threads(omp_get_max_threads())
    for (int i = 0; i < n; i++) {
        arr[i] = (double)i * 1.5;
        sum += arr[i];
        
        /* Nested directive with for clause in complex context */
        if (i % 100 == 0) {
            /* Complex directive with for clause and arguments */
            #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
                private(i) /* This will trigger pretty-print of 'for' clause */
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 10; k++) {
                    /* Dummy computation */
                    arr[(j*10 + k) % n] += 0.001;
                }
            }
        }
    }
    
    /* 2. Combined parallel sections clause - triggers 'parallel' and 'sections' */
    #pragma omp parallel sections reduction(max:max_val) \
        default(shared) private(sum)
    {
        /* First section */
        #pragma omp section
        {
            max_val = arr[0];
            for (int i = 1; i < n/2; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
            
            /* Nested sections directive */
            #pragma omp sections
            {
                #pragma omp section
                { sum = arr[0]; }
                
                #pragma omp section  
                { sum = arr[1]; }
            }
        }
        
        /* Second section */
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
            
            /* Trigger diagnostic with sections clause name */
            #pragma omp error severity(warning) message("Processing sections clause")
        }
    }
    
    /* 3. Taskgroup clause with task_reduction */
    double task_sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum) \
                /* This should trigger pretty-print of 'taskgroup' clause */
            {
                for (int i = 0; i < 10; i++) {
                    #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                    {
                        double local_sum = 0.0;
                        for (int j = 0; j < n/10; j++) {
                            local_sum += arr[(i * n/10 + j) % n];
                        }
                        task_sum += local_sum;
                        
                        /* Trigger diagnostic with for clause name */
                        if (i == 5) {
                            _Pragma("omp error severity(message) message(\"for clause encountered in task\")")
                        }
                    }
                }
            }
        }
    }
    
    /* Force compiler to retain all constructs by using results */
    printf("Array processed: sum=%.2f, max=%.2f, task_sum=%.2f\n", 
           sum, max_val, task_sum);
}

/* Another function with mixed OpenMP and complex C constructs */
void __attribute__((optimize("O0"))) nested_omp_constructs() {
    int switch_var = 2;
    
    switch (switch_var) {
        case 1: {
            /* Parallel for in switch case */
            #pragma omp parallel for ordered
            for (int i = 0; i < 10; i++) {
                #pragma omp ordered
                printf("Ordered iteration %d\n", i);
            }
            break;
        }
        case 2: {
            /* Parallel sections with nested for */
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    #pragma omp for nowait
                    for (int i = 0; i < 5; i++) {
                        /* Empty */
                    }
                }
                #pragma omp section
                {
                    /* Trigger parallel clause pretty-print */
                    _Pragma("omp parallel num_threads(2)")
                    {
                        printf("Nested parallel region\n");
                    }
                }
            }
            break;
        }
    }
    
    /* Macro expansion with _Pragma for complex clause patterns */
    #define CREATE_TASKGROUP(id) \
        _Pragma("omp taskgroup task_reduction(+:id##_sum)") \
        { \
            _Pragma("omp task in_reduction(+:id##_sum)") \
            { id##_sum += 1.0; } \
        }
    
    double my_sum = 0.0;
    CREATE_TASKGROUP(my)
    
    /* Use result to prevent optimization */
    printf("Taskgroup result: %.2f\n", my_sum);
}

int main() {
    double *array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    if (!array) return 1;
    
    /* Initialize OpenMP runtime */
    omp_set_dynamic(0);
    omp_set_num_threads(4);
    
    /* Process array with OpenMP directives */
    process_array(array, ARRAY_SIZE);
    
    /* Execute nested constructs */
    nested_omp_constructs();
    
    /* Additional complex directive combining target clauses */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom:array[0:ARRAY_SIZE]) schedule(static) collapse(1)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] *= 1.01;
    }
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    #pragma omp parallel for reduction(+:checksum) \
        if(ARRAY_SIZE > 500) schedule(guided)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array[i];
        
        /* Nested task with error directive */
        if (i == ARRAY_SIZE/2) {
            #pragma omp task untied
            {
                /* Force pretty-print of sections clause */
                #pragma omp error severity(warning) \
                    message("Midpoint reached: sections clause may be printed")
            }
        }
    }
    
    printf("Final checksum: %.6f\n", checksum);
    
    free(array);
    return 0;
}

/* test_openmp_clauses.c - Targeting uncovered lines in tree-pretty-print.cc */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 10000
#define NUM_ITERATIONS 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"))) process_with_openmp(double *data, int n, double *result_sum, double *result_max) {
    double local_sum = 0.0;
    double local_max = -INFINITY;
    
    /* 1. Use 'for' clause with schedule and collapse arguments */
    #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
                reduction(+:local_sum) reduction(max:local_max) \
                if(n > 1000)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            double val = data[i] * (j + 1);
            local_sum += val;
            if (val > local_max) local_max = val;
        }
    }
    
    *result_sum = local_sum;
    *result_max = local_max;
}

/* Another function using sections clause */
void __attribute__((optimize("O0"))) compute_with_sections(double *data, int n, double *sum1, double *sum2) {
    double section1_sum = 0.0;
    double section2_sum = 0.0;
    
    /* 2. Use 'sections' clause with multiple section blocks */
    #pragma omp parallel sections reduction(+:section1_sum, section2_sum) \
                num_threads(4)
    {
        #pragma omp section
        {
            for (int i = 0; i < n/2; i++) {
                section1_sum += sqrt(fabs(data[i]));
            }
        }
        
        #pragma omp section
        {
            for (int i = n/2; i < n; i++) {
                section2_sum += log(fabs(data[i]) + 1.0);
            }
        }
        
        /* Additional section to ensure sections clause is fully utilized */
        #pragma omp section
        {
            /* Empty section but still valid */
        }
    }
    
    *sum1 = section1_sum;
    *sum2 = section2_sum;
}

/* Function using taskgroup clause */
void __attribute__((optimize("O0"))) process_with_taskgroup(double *data, int n, double *final_result) {
    double task_sum = 0.0;
    
    /* 3. Use 'taskgroup' clause with task_reduction */
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskgroup task_reduction(+:task_sum)
        {
            for (int i = 0; i < n; i += n/10) {
                #pragma omp task in_reduction(+:task_sum) firstprivate(i)
                {
                    double chunk_sum = 0.0;
                    int end = (i + n/10) < n ? (i + n/10) : n;
                    for (int j = i; j < end; j++) {
                        chunk_sum += sin(data[j]) * cos(data[j]);
                    }
                    task_sum += chunk_sum;
                    
                    /* Nested task with error directive containing clause name */
                    #pragma omp task
                    {
                        /* Force diagnostic with clause name in message */
                        #pragma omp error message("Processing with for clause simulation")
                        double temp = 0.0;
                        for (int k = 0; k < 10; k++) {
                            temp += data[0] * k;
                        }
                    }
                }
            }
        }
    }
    
    *final_result = task_sum;
}

/* Complex control flow with mixed OpenMP directives */
void __attribute__((optimize("O0"))) complex_control_flow(double *data, int n) {
    int thread_count = 0;
    
    /* Switch statement with OpenMP inside cases */
    for (int iteration = 0; iteration < 3; iteration++) {
        switch (iteration) {
            case 0:
                /* Combined parallel for directive */
                #pragma omp parallel for schedule(dynamic) \
                            if(omp_get_max_threads() > 1)
                for (int i = 0; i < n; i++) {
                    data[i] = i * 0.01;
                }
                break;
                
            case 1:
                /* Parallel sections with runtime calls */
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        thread_count = omp_get_num_threads();
                    }
                    #pragma omp section
                    {
                        /* Use _Pragma for complex pattern */
                        _Pragma("omp critical")
                        {
                            printf("Thread %d in section\n", omp_get_thread_num());
                        }
                    }
                }
                break;
                
            case 2:
                /* Taskgroup with nested tasks */
                #pragma omp parallel
                #pragma omp single
                {
                    #pragma omp taskgroup
                    {
                        #pragma omp task
                        {
                            /* Another error directive with clause reference */
                            #pragma omp error severity(warning) message("sections clause used previously")
                        }
                    }
                }
                break;
        }
    }
}

int main() {
    double *data = (double *)malloc(ARRAY_SIZE * sizeof(double));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    double checksum = 0.0;
    
    /* Initialize data with parallel for */
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i % 100) * 0.5;
    }
    
    /* Process with various OpenMP constructs */
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        double sum_result, max_result;
        double section_sum1, section_sum2;
        double taskgroup_result;
        
        /* Call functions with different OpenMP clauses */
        process_with_openmp(data, ARRAY_SIZE, &sum_result, &max_result);
        compute_with_sections(data, ARRAY_SIZE, &section_sum1, &section_sum2);
        process_with_taskgroup(data, ARRAY_SIZE, &taskgroup_result);
        complex_control_flow(data, ARRAY_SIZE);
        
        /* Accumulate checksum to prevent dead code elimination */
        checksum += sum_result + max_result + section_sum1 + section_sum2 + taskgroup_result;
        
        /* Modify data slightly for next iteration */
        #pragma omp parallel for simd
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] += 0.001 * sin(i * 0.01);
        }
    }
    
    /* Final parallel region with combined clauses */
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = fmod(data[i], 100.0);
        }
        
        #pragma omp sections
        {
            #pragma omp section
            {
                /* Empty but ensures sections clause is present */
            }
            #pragma omp section
            {
                /* Use error directive one more time */
                #pragma omp error message("taskgroup clause was used")
            }
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Using %d OpenMP threads maximum\n", omp_get_max_threads());
    
    free(data);
    return 0;
}

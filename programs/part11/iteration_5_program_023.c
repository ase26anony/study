/* test_openmp_clauses.c - Targeting uncovered pretty-print lines for OMP clauses */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>

#define ARRAY_SIZE 1000
#define NUM_ITERATIONS 100

/* Function with optimization attribute to prevent directive removal */
void __attribute__((optimize("O0"), noinline)) 
process_with_openmp(double *data, int n, double *results) {
    double sum = 0.0;
    double max_val = -INFINITY;
    
    /* 1. PARALLEL FOR clause - triggers OMP_CLAUSE_FOR and OMP_CLAUSE_PARALLEL */
    #pragma omp parallel for schedule(dynamic) reduction(+:sum) if(n > 100)
    for (int i = 0; i < n; i++) {
        data[i] = sin(i * 0.01) + cos(i * 0.005);
        sum += data[i];
        
        /* Nested OpenMP construct with FOR clause in complex context */
        if (i % 100 == 0) {
            /* DISTRIBUTE PARALLEL FOR SIMD with explicit arguments */
            #pragma omp distribute parallel for simd schedule(static, 4) collapse(2) \
                private(i) if(omp_get_num_threads() > 1)
            for (int j = 0; j < 10; j++) {
                for (int k = 0; k < 10; k++) {
                    /* Force diagnostic with clause name in message */
                    #pragma omp error severity(warning) message("Processing for clause iteration")
                    data[j * 10 + k] *= 1.0001;
                }
            }
        }
    }
    results[0] = sum;
    
    /* 2. PARALLEL SECTIONS clause - triggers OMP_CLAUSE_PARALLEL and OMP_CLAUSE_SECTIONS */
    #pragma omp parallel sections private(sum) reduction(max:max_val) \
        num_threads(omp_get_max_threads() / 2 + 1)
    {
        #pragma omp section
        {
            sum = 0.0;
            #pragma omp parallel for simd reduction(+:sum) if(n > 500)
            for (int i = 0; i < n/2; i++) {
                sum += sqrt(fabs(data[i]));
            }
            max_val = sum;
            
            /* Macro expansion with _Pragma for complex pretty-printing */
            #define EMIT_SECTION_WARNING() \
                _Pragma("omp error severity(message) message(\"In sections clause region\")")
            
            EMIT_SECTION_WARNING();
        }
        
        #pragma omp section
        {
            double local_max = -INFINITY;
            #pragma omp parallel for reduction(max:local_max) schedule(guided)
            for (int i = n/2; i < n; i++) {
                if (data[i] > local_max) local_max = data[i];
                /* Complex control flow with OpenMP */
                switch (i % 4) {
                    case 0:
                        #pragma omp atomic
                        data[i] += 0.001;
                        break;
                    case 1:
                        #pragma omp critical
                        data[i] = fmod(data[i], 10.0);
                        break;
                }
            }
            if (local_max > max_val) max_val = local_max;
        }
    }
    results[1] = max_val;
    
    /* 3. TASKGROUP clause with task_reduction argument */
    double task_sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskgroup task_reduction(+:task_sum)
            {
                for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
                    #pragma omp task in_reduction(+:task_sum) \
                        depend(out: data[iter % n]) if(iter % 10 == 0)
                    {
                        int idx = iter % n;
                        double val = data[idx] * (iter + 1);
                        task_sum += val;
                        
                        /* Force pretty-printing of taskgroup clause in diagnostic */
                        if (val > 1000.0) {
                            #pragma omp error severity(warning) \
                                message("Large value in taskgroup clause reduction")
                        }
                    }
                }
                
                /* Additional task with nested directive */
                #pragma omp task untied mergeable
                {
                    /* Nested SECTIONS clause inside taskgroup */
                    #pragma omp sections
                    {
                        #pragma omp section
                        task_sum += 1.0;
                        #pragma omp section  
                        task_sum += 2.0;
                    }
                }
            }
        }
    }
    results[2] = task_sum;
    
    /* 4. Mixed directives in complex control flow */
    for (int phase = 0; phase < 3; phase++) {
        switch (phase) {
            case 0:
                /* Combined PARALLEL FOR with multiple clauses */
                #pragma omp parallel for ordered schedule(nonmonotonic:dynamic) \
                    linear(i:1) lastprivate(sum)
                for (int i = 0; i < n; i += 10) {
                    #pragma omp ordered
                    data[i] = phase * 100.0 + i;
                }
                break;
                
            case 1:
                /* SECTIONS clause with explicit section blocks */
                #pragma omp parallel sections
                {
                    #pragma omp section
                    {
                        /* Macro with _Pragma for FOR clause */
                        #define PROCESS_CHUNK(start, end) \
                            _Pragma("omp parallel for schedule(static)") \
                            for (int i = (start); i < (end); i++) { \
                                data[i] = log(fabs(data[i]) + 1.0); \
                            }
                        
                        PROCESS_CHUNK(0, n/3)
                    }
                    
                    #pragma omp section
                    {
                        #pragma omp taskloop grainsize(64) nogroup
                        for (int i = n/3; i < 2*n/3; i++) {
                            data[i] = exp(data[i] * 0.01);
                        }
                    }
                }
                break;
                
            case 2:
                /* TASKGROUP clause with explicit taskwait */
                #pragma omp taskgroup
                {
                    #pragma omp task
                    {
                        results[3] = data[0];
                    }
                    #pragma omp task
                    {
                        results[4] = data[n-1];
                    }
                    #pragma omp taskwait
                    
                    /* Error directive with clause names in message */
                    #pragma omp error severity(message) \
                        message("Processed for, parallel, sections, and taskgroup clauses")
                }
                break;
        }
    }
}

/* Recursive function with OpenMP directives */
double __attribute__((optimize("O0")))
recursive_processing(double *data, int start, int end, int depth) {
    if (end - start <= 1 || depth >= 3) {
        return data[start];
    }
    
    double left_result, right_result;
    int mid = (start + end) / 2;
    
    /* PARALLEL SECTIONS in recursive context */
    #pragma omp parallel sections if(depth == 0)
    {
        #pragma omp section
        {
            left_result = recursive_processing(data, start, mid, depth + 1);
        }
        
        #pragma omp section
        {
            right_result = recursive_processing(data, mid, end, depth + 1);
        }
    }
    
    /* TASKGROUP with task_reduction in conditional block */
    if (depth == 0) {
        double reduction_sum = 0.0;
        #pragma omp parallel
        {
            #pragma omp single
            {
                #pragma omp taskgroup task_reduction(+:reduction_sum)
                {
                    for (int i = start; i < end; i += (end - start)/4) {
                        #pragma omp task in_reduction(+:reduction_sum)
                        {
                            reduction_sum += data[i];
                        }
                    }
                }
            }
        }
        return left_result + right_result + reduction_sum;
    }
    
    return left_result + right_result;
}

int main() {
    double *data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    double results[5] = {0};
    
    /* Initialize with OpenMP PARALLEL FOR */
    #pragma omp parallel for simd schedule(static, 16) \
        if(ARRAY_SIZE > 100) num_threads(omp_get_max_threads())
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 0.01;
    }
    
    /* Process with various OpenMP clauses */
    process_with_openmp(data, ARRAY_SIZE, results);
    
    /* Recursive processing with OpenMP */
    double recursive_result = recursive_processing(data, 0, ARRAY_SIZE, 0);
    
    /* Final computation with combined clauses */
    double final_sum = 0.0;
    #pragma omp parallel for reduction(+:final_sum) \
        schedule(dynamic, 8) collapse(1)
    for (int i = 0; i < ARRAY_SIZE; i++) {
        final_sum += data[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results: sum=%.4f, max=%.4f, task_sum=%.4f, recursive=%.4f, final=%.4f\n",
           results[0], results[1], results[2], recursive_result, final_sum);
    
    /* Force diagnostic at end with clause reference */
    #pragma omp error severity(warning) \
        message("Compilation covered for, parallel, sections, and taskgroup clause printing")
    
    free(data);
    return 0;
}

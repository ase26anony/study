#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

extern volatile int g_volatile_cond;

/* Function 1: Complex reduction patterns */
void test_reductions(int n, int *array, float *farray, int *checksum) {
    int sum = 0;
    float fsum = 0.0f;
    long prod = 1;
    
    /* Multiple reduction variables in parallel region */
    #pragma omp parallel reduction(+:sum, fsum) reduction(*:prod) if(g_volatile_cond > 0)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            sum += array[i];
            fsum += farray[i];
        }
        
        #pragma omp for reduction(*:prod)
        for (int i = 1; i <= n; i++) {
            prod *= (i % 10) + 1;
        }
        
        /* Nested reduction with taskloop */
        #pragma omp taskloop reduction(+:sum)
        for (int i = 0; i < n/2; i++) {
            sum += array[i] * 2;
        }
    }
    
    /* Combined parallel for simd with reduction */
    #pragma omp parallel for simd reduction(+:sum) simdlen(4)
    for (int i = 0; i < n; i++) {
        sum += array[i] * 3;
    }
    
    *checksum += sum + (int)fsum + (int)prod;
}

/* Function 2: Scan operations */
void test_scans(int n, int *array, int *checksum) {
    int prefix_sum = 0;
    int scan_sum = 0;
    
    /* SIMD with inscan reduction */
    #pragma omp simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        prefix_sum += array[i];
        #pragma omp scan exclusive(prefix_sum)
        array[i] = prefix_sum;
        scan_sum += prefix_sum;
    }
    
    /* Parallel for with scan directive */
    #pragma omp parallel for reduction(+:scan_sum)
    for (int i = 0; i < n; i++) {
        int local = array[i];
        #pragma omp scan exclusive(local)
        scan_sum += local;
    }
    
    *checksum += scan_sum;
}

/* Function 3: Conditional temporaries with volatile */
void test_conditionals(int n, int *array, volatile int cond_var, int *checksum) {
    int result = 0;
    
    /* Multiple if clauses with volatile conditions */
    #pragma omp parallel if(cond_var > 0) if(cond_var < n) reduction(+:result)
    {
        #pragma omp for
        for (int i = 0; i < n; i++) {
            result += array[i];
        }
    }
    
    /* Nested parallel with different condition */
    #pragma omp parallel if(cond_var % 2 == 0)
    {
        #pragma omp for reduction(+:result)
        for (int i = 0; i < n; i++) {
            result += array[i] * 2;
        }
        
        /* Task with if clause */
        #pragma omp task if(cond_var > 10)
        {
            result += 100;
        }
    }
    
    *checksum += result;
}

/* Function 4: Enter data with to mapper */
void test_enter_data(int n, int **array_ptr, int *checksum) {
    int *local_array = *array_ptr;
    
    /* Multiple enter data directives with to mapper */
    #pragma omp enter data map(to: local_array[0:n])
    
    #pragma omp target enter data map(to: local_array[0:n]) if(n > 100)
    
    /* Use the data */
    #pragma omp target teams distribute parallel for simd
    for (int i = 0; i < n; i++) {
        local_array[i] *= 2;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: local_array[0:n])
    #pragma omp exit data map(release: local_array)
    
    /* Calculate checksum */
    for (int i = 0; i < n; i++) {
        *checksum += local_array[i];
    }
}

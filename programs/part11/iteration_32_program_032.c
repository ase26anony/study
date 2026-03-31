/* test_omp_simt.c - Program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Mixed data types and vector types */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE 2

/* Function with conditional SIMD execution */
void conditional_simd_computation(float *data, int *indices, int use_simd, int n, float *sum_out) {
    float sum = 0.0f;
    
    /* Conditional SIMD execution path - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* Complex loop with reduction and data-dependent access */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:n]) map(to: indices[0:n]) \
            reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < n; i++) {
            /* Non-contiguous memory access with indirect indexing */
            int idx = indices[i];
            if (idx < n) {
                /* Data-dependent operation */
                data[idx] = data[idx] * 2.0f + 1.0f;
                sum += data[idx];
                
                /* Conditional break - adds complexity */
                if (sum > 10000.0f) {
                    /* This may affect SIMT lane management */
                    data[idx] = data[idx] / 2.0f;
                }
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < n; i++) {
            int idx = indices[i];
            if (idx < n) {
                data[idx] = data[idx] * 2.0f + 1.0f;
                sum += data[idx];
            }
        }
    }
    
    *sum_out = sum;
}

/* Function with nested SIMD loops and mixed data types */
void nested_simd_with_mixed_types(float *fdata, double *ddata, int n) {
    /* Outer loop with teams distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: fdata[0:n], ddata[0:n]) \
        simdlen(8) safelen(16)
    for (int i = 0; i < n; i++) {
        /* Mixed precision computation */
        double temp = (double)fdata[i];
        
        /* Inner SIMD-style computation */
        #pragma omp simd linear(j:1) aligned(fdata, ddata:32)
        for (int j = 0; j < 4; j++) {
            if (i + j < n) {
                /* Strided access pattern */
                fdata[i + j * STRIDE] = fdata[i + j * STRIDE] * 3.0f;
                ddata[i] += (double)fdata[i + j * STRIDE];
            }
        }
        
        /* Vector type operation */
        v4sf vec_data = {fdata[i], fdata[i+1], fdata[i+2], fdata[i+3]};
        v4sf vec_mul = {2.0f, 2.0f, 2.0f, 2.0f};
        vec_data = vec_data * vec_mul;
        
        /* Store back */
        if (i + 3 < n) {
            fdata[i] = vec_data[0];
            fdata[i+1] = vec_data[1];
            fdata[i+2] = vec_data[2];
            fdata[i+3] = vec_data[3];
        }
    }
}

/* Function with SIMD loop containing complex conditions */
void simd_with_conditions(float *a, float *b, int *mask, int n) {
    float threshold = 0.5f;
    
    /* SIMD loop with safelen clause and conditional execution */
    #pragma omp simd safelen(8) linear(i:1) \
        reduction(+:threshold) if(simd: n > 100)
    for (int i = 0; i < n; i += STRIDE) {  /* Non-unit stride */
        /* Complex conditional logic */
        if (mask[i % 16]) {
            a[i] = a[i] * b[i] + threshold;
            
            /* Data-dependent operation that may cause divergence */
            if (a[i] > 100.0f) {
                a[i] = 100.0f;
                threshold += 0.1f;  /* Affects reduction variable */
            }
        } else {
            a[i] = a[i] / b[i] - threshold;
        }
        
        /* Early exit condition - challenging for SIMT */
        if (i > 0 && a[i] < 0.0f && a[i-1] < 0.0f) {
            /* This creates control flow divergence */
            b[i] = -b[i];
        }
    }
}

int main(int argc, char *argv[]) {
    const int n = SIZE;
    float *data = (float*)aligned_alloc(32, n * sizeof(float));
    double *ddata = (double*)aligned_alloc(32, n * sizeof(double));
    int *indices = (int*)malloc(n * sizeof(int));
    int *mask = (int*)malloc(16 * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        data[i] = (float)i / n;
        ddata[i] = (double)i / n;
        indices[i] = (i * 3) % n;  /* Create non-linear access pattern */
    }
    
    for (int i = 0; i < 16; i++) {
        mask[i] = i % 2;  /* Alternating mask pattern */
    }
    
    /* Runtime condition for conditional SIMD execution */
    int use_simd = (argc > 1) ? atoi(argv[1]) : 0;
    float sum = 0.0f;
    
    printf("Starting computation (use_simd = %d)...\n", use_simd);
    
    /* Call 1: Conditional SIMD with target offloading */
    conditional_simd_computation(data, indices, use_simd, n, &sum);
    
    /* Call 2: Always execute - nested SIMD with mixed types */
    nested_simd_with_mixed_types(data, ddata, n);
    
    /* Call 3: SIMD with complex conditions */
    simd_with_conditions(data, data + n/2, mask, n/2);
    
    /* Prevent dead code elimination */
    printf("Results:\n");
    printf("  Sum from conditional computation: %f\n", sum);
    printf("  Sample values: data[0]=%f, data[100]=%f, data[500]=%f\n",
           data[0], data[100], data[500]);
    printf("  Double data sample: ddata[50]=%f\n", ddata[50]);
    
    /* Cleanup */
    free(data);
    free(ddata);
    free(indices);
    free(mask);
    
    return 0;
}

/* test_omp_simt.c - Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Mixed data types and vector types to stress SIMT lowering */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

#define SIZE 1024
#define STRIDE_SIZE 512

int main(int argc, char *argv[]) {
    /* Runtime condition to control SIMD execution path */
    int use_simd = argc > 1;  /* Enable SIMD if any argument given */
    
    /* Arrays with different access patterns */
    float data[SIZE];
    double dbl_data[SIZE];
    int indices[SIZE];
    v4sf vec_data[SIZE/4];
    
    /* Reduction variable */
    double sum = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)(i % 100) * 0.1f;
        dbl_data[i] = (double)(i % 100) * 0.2;
        indices[i] = (i * 3) % SIZE;
    }
    for (int i = 0; i < SIZE/4; i++) {
        vec_data[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    /* =========================================== */
    /* 1. Conditional SIMD execution with offloading */
    /* This may trigger the IFN_GOMP_USE_SIMT path */
    /* =========================================== */
    if (use_simd) {
        /* Target offloading directive - often uses SIMT model */
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:SIZE]) map(to: indices[0:SIZE]) \
            reduction(+:sum) if(target: use_simd)
        for (int i = 0; i < SIZE; i++) {
            /* Complex data-dependent condition inside SIMD loop */
            if (data[i] > 50.0f) {
                /* Early exit - creates control flow divergence */
                data[i] = 0.0f;
            } else {
                /* Mixed data type operations */
                float temp = data[i] * 2.0f + (float)dbl_data[i];
                
                /* Indirect memory access pattern */
                int idx = indices[i];
                data[i] = temp + data[idx] * 0.5f;
                
                /* Reduction with data-dependent weighting */
                sum += (data[i] > 10.0f) ? data[i] * 0.1 : data[i] * 0.01;
            }
        }
        
        /* Nested loop with inner SIMD - another candidate for SIMT */
        #pragma omp target teams distribute parallel for simd \
            collapse(2) map(tofrom: dbl_data[0:SIZE])
        for (int i = 0; i < 32; i++) {
            for (int j = 0; j < 32; j++) {
                int idx = i * 32 + j;
                if (idx < SIZE) {
                    /* Data-dependent computation */
                    dbl_data[idx] = dbl_data[idx] * (1.0 + (i % 3) * 0.1);
                }
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < SIZE; i++) {
            if (data[i] > 50.0f) {
                data[i] = 0.0f;
            } else {
                float temp = data[i] * 2.0f + (float)dbl_data[i];
                int idx = indices[i];
                data[i] = temp + data[idx] * 0.5f;
                sum += (data[i] > 10.0f) ? data[i] * 0.1 : data[i] * 0.01;
            }
        }
    }
    
    /* =========================================== */
    /* 2. Unconditional SIMD with complex patterns */
    /* Always present to ensure SIMD parsing */
    /* =========================================== */
    
    /* SIMD with safelen clause and non-unit stride */
    #pragma omp simd safelen(8) aligned(data: 16) linear(i:1)
    for (int i = 0; i < STRIDE_SIZE; i++) {
        /* Non-contiguous memory access with stride */
        data[i*2] = data[i*2] * 3.0f + (float)i * 0.01f;
        
        /* Conditional within SIMD loop */
        if (data[i*2] > 100.0f) {
            data[i*2] = 100.0f;
        }
    }
    
    /* SIMD reduction with mixed types */
    float float_sum = 0.0f;
    #pragma omp simd reduction(+:float_sum) private(indices)
    for (int i = 0; i < SIZE; i += 4) {
        /* Vector type operations within SIMD loop */
        v4sf vec = *(v4sf*)&data[i];
        vec = vec * (v4sf){1.1f, 1.2f, 1.3f, 1.4f};
        *(v4sf*)&data[i] = vec;
        
        /* Reduction on vector elements */
        float_sum += vec[0] + vec[1] + vec[2] + vec[3];
    }
    
    /* SIMD loop with linear clause and complex index */
    #pragma omp simd linear(k:2)
    for (int i = 0, k = 0; i < SIZE/2; i++, k += 2) {
        /* Access with compile-time unknown stride */
        data[k] = data[k] + dbl_data[i];
    }
    
    /* =========================================== */
    /* 3. Additional SIMD constructs for coverage */
    /* =========================================== */
    
    /* SIMD with if clause (runtime decision) */
    int simd_enabled = 1;
    #pragma omp simd if(simd_enabled)
    for (int i = 0; i < SIZE; i++) {
        dbl_data[i] = dbl_data[i] / (1.0 + i * 0.001);
    }
    
    /* Nested parallelism with SIMD */
    #pragma omp parallel for simd schedule(static)
    for (int i = 0; i < SIZE; i++) {
        /* Use thread-specific computations */
        int tid = omp_get_thread_num();
        data[i] = data[i] + tid * 0.001f;
    }
    
    /* Prevent dead code elimination */
    printf("Results: sum=%f, float_sum=%f\n", sum, float_sum);
    printf("Sample data[0]=%f, data[100]=%f, data[500]=%f\n", 
           data[0], data[100], data[500]);
    printf("SIMD was %s\n", use_simd ? "enabled" : "disabled");
    
    return 0;
}

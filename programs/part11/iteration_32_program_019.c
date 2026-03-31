/* test_omp_simt.c - Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define STRIDE 2

/* Mixed data types to complicate vectorization */
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char **argv) {
    /* Runtime condition to control SIMD path */
    int use_simd = argc > 1;
    int use_offload = argc > 2;
    
    /* Arrays with different access patterns */
    float data[N];
    double dbl_data[N];
    int indices[N];
    v4sf vec_data[N/4];
    
    /* Reduction variable */
    double sum = 0.0;
    float vec_sum = 0.0f;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        data[i] = (float)(i % 100) * 0.1f;
        dbl_data[i] = (double)(i % 50) * 0.2;
        indices[i] = (i * 3) % N;
    }
    
    for (int i = 0; i < N/4; i++) {
        vec_data[i] = (v4sf){i*0.1f, i*0.2f, i*0.3f, i*0.4f};
    }
    
    /* Conditional SIMD execution - may trigger IFN_GOMP_USE_SIMT */
    if (use_simd) {
        /* SIMD loop with reduction and complex access pattern */
        #pragma omp simd reduction(+:sum) safelen(16) linear(i:1) aligned(data:32)
        for (int i = 0; i < N; i++) {
            /* Data-dependent condition inside SIMD loop */
            if (data[i] > 50.0f) {
                /* Early exit - complicates SIMD transformation */
                if (i > N/2) break;
            }
            
            /* Mixed data type operations */
            data[i] = data[i] * 2.0f + (float)dbl_data[i];
            sum += data[i];
            
            /* Non-contiguous access */
            if (i % STRIDE == 0 && i + STRIDE < N) {
                data[i + STRIDE] = data[i] * 0.5f;
            }
        }
        
        /* Nested loop with inner SIMD */
        for (int outer = 0; outer < 10; outer++) {
            #pragma omp simd reduction(+:vec_sum)
            for (int i = 0; i < N; i += 4) {
                /* Vector type operations */
                v4sf temp = *(v4sf*)&data[i];
                temp = temp * 1.5f;
                *(v4sf*)&data[i] = temp;
                vec_sum += data[i] + data[i+1];
            }
        }
    } else {
        /* Sequential fallback */
        for (int i = 0; i < N; i++) {
            if (data[i] > 50.0f && i > N/2) break;
            data[i] = data[i] * 2.0f + (float)dbl_data[i];
            sum += data[i];
            if (i % STRIDE == 0 && i + STRIDE < N) {
                data[i + STRIDE] = data[i] * 0.5f;
            }
        }
    }
    
    /* Unconditional SIMD loop with stride - always present */
    #pragma omp simd safelen(8)
    for (int i = 0; i < N/STRIDE; i++) {
        /* Non-unit stride access */
        data[i * STRIDE] = data[i * STRIDE] * 3.0f;
        
        /* Indirect indexing */
        int idx = indices[i];
        if (idx < N) {
            dbl_data[idx] = dbl_data[idx] * 1.1;
        }
    }
    
    /* GPU offloading with SIMD - may trigger SIMT transformation */
    if (use_offload) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: data[0:N]) map(to: indices[0:N]) reduction(+:sum)
        for (int i = 0; i < N; i++) {
            /* Complex computation with conditional */
            float val = data[i];
            if (val > 25.0f) {
                val = sqrtf(val);
            } else {
                val = val * val;
            }
            
            /* Indirect memory access */
            int idx = indices[i];
            if (idx >= 0 && idx < N) {
                val += data[idx] * 0.1f;
            }
            
            data[i] = val;
            sum += val;
        }
    }
    
    /* Additional SIMD construct with mixed directives */
    #pragma omp parallel
    {
        #pragma omp for simd nowait
        for (int i = 0; i < N; i++) {
            data[i] = data[i] + 1.0f;
        }
        
        #pragma omp single
        {
            /* Nested SIMD in single region */
            #pragma omp simd
            for (int i = 0; i < N/2; i++) {
                dbl_data[i] = dbl_data[i] * 2.0;
            }
        }
    }
    
    /* Prevent dead code elimination */
    printf("Results: sum=%.2f, vec_sum=%.2f\n", sum, vec_sum);
    printf("Sample data[0]=%.2f, data[100]=%.2f, data[500]=%.2f\n", 
           data[0], data[100], data[500]);
    
    return 0;
}

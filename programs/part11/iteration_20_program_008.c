/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization of helper functions */
__attribute__((noinline)) 
int simt_test(int n, int threshold, int iter) {
    volatile int size = TOTAL; /* volatile to prevent optimization */
    int i, j;
    int result = 0;
    
    /* Arrays with volatile elements to prevent dead code elimination */
    volatile int a[TOTAL];
    volatile int b[TOTAL];
    volatile int c[TOTAL];
    
    /* Initialize arrays with non-constant values */
    for (i = 0; i < TOTAL; i++) {
        a[i] = (i + iter) % 100;
        b[i] = (i * 2 + iter) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) shared(a, b, c, size, n, threshold)
    {
        int local_sum = 0;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(n > threshold) \
                map(tofrom: a, b, c) \
                reduction(+:local_sum) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Force basic block split with conditional */
                if (iter % 2 == 0) {
                    c[idx] = a[idx] + b[idx] + n;
                } else {
                    c[idx] = a[idx] - b[idx] + n;
                }
                
                /* Additional conditional to create more basic blocks */
                if (c[idx] > 150) {
                    /* Early exit-like behavior to force label generation */
                    c[idx] = 150;
                    local_sum += 1; /* Count overflow cases */
                } else if (c[idx] < 0) {
                    c[idx] = 0;
                    local_sum -= 1; /* Count underflow cases */
                } else {
                    local_sum += c[idx] % 10;
                }
                
                /* Force another basic block with dummy operation */
                volatile int dummy = c[idx];
                (void)dummy; /* Suppress unused warning */
            }
        }
        
        #pragma omp atomic
        result += local_sum;
    }
    
    /* Additional computation to prevent loop elimination */
    volatile int checksum = 0;
    for (i = 0; i < TOTAL; i++) {
        checksum += c[i];
    }
    
    return result + (checksum % 1000);
}

/* Another test function with different loop structure */
__attribute__((noinline))
int simt_test2(int n, int flag) {
    volatile int size = 256;
    int x[256], y[256], z[256];
    int i, j, sum = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i * 3;
        y[i] = i * 2;
    }
    
    /* Nested OpenMP regions */
    #pragma omp parallel for private(i) reduction(+:sum)
    for (i = 0; i < 4; i++) {
        #pragma omp target teams distribute parallel for simd \
                if(flag) \
                map(tofrom: x, y, z) \
                num_teams(2)
        for (j = i * 64; j < (i + 1) * 64; j++) {
            z[j] = x[j] + y[j] + n;
            
            /* Complex conditional to encourage SIMT transformation */
            if (z[j] % 3 == 0) {
                z[j] *= 2;
                if (z[j] > 500) {
                    z[j] = z[j] % 500;
                    sum += 1;
                }
            } else if (z[j] % 5 == 0) {
                z[j] /= 2;
                sum += 2;
            }
            
            /* Use of __builtin_assume_aligned to create interesting GIMPLE */
            int *ptr = &z[j];
            ptr = (int*)__builtin_assume_aligned(ptr, 16);
            *ptr += j % 8;
        }
    }
    
    return sum;
}

int main() {
    int i, total_result = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying parameters to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int result1 = simt_test(i, threshold, i);
        int result2 = simt_test2(i, i % 2);
        
        total_result += result1 + result2;
        
        printf("Iteration %d: result1=%d, result2=%d\n", 
               i, result1, result2);
    }
    
    printf("Total result: %d\n", total_result);
    
    /* Additional test with dynamic allocation */
    #pragma omp parallel
    {
        #pragma omp single
        {
            int *dyn_arr = (int*)malloc(1000 * sizeof(int));
            if (dyn_arr) {
                #pragma omp target teams distribute parallel for simd \
                        map(tofrom: dyn_arr[0:1000]) \
                        if(1)  /* Always true but forces conditional path */
                for (int k = 0; k < 1000; k++) {
                    dyn_arr[k] = k * k;
                    /* Force multiple basic blocks */
                    if (dyn_arr[k] % 7 == 0) {
                        dyn_arr[k] = -dyn_arr[k];
                    }
                }
                free(dyn_arr);
            }
        }
    }
    
    return total_result != 0 ? 0 : 1;
}

/* Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization of test function */
__attribute__((noinline)) 
int simt_test(int n, int threshold, volatile int flag) {
    int i, j;
    int result = 0;
    
    /* Use volatile to prevent loop optimization */
    volatile int bound1 = n > 0 ? n : N;
    volatile int bound2 = M;
    
    /* Arrays that will be mapped to/from device */
    int a[TOTAL], b[TOTAL], c[TOTAL];
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i % 100;
        b[i] = (i * 2) % 100;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) firstprivate(bound1, bound2)
    {
        int local_flag = flag;
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:TOTAL], b[0:TOTAL]) \
                map(from: c[0:TOTAL]) \
                num_teams(4) thread_limit(64)
        for (i = 0; i < bound1; i++) {
            for (j = 0; j < bound2; j++) {
                int idx = i * bound2 + j;
                
                /* Main computation - simple arithmetic */
                c[idx] = a[idx] + b[idx] + local_flag;
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 200 && local_flag > 0) {
                    /* Early exit creates extra control flow */
                    c[idx] = 200;
                    /* This creates additional basic blocks and labels */
                    if (i > bound1/2) {
                        /* Nested condition for more complex CFG */
                        c[idx] = 199;
                    }
                }
                
                /* Another condition to split basic blocks further */
                if (j % 16 == 0 && local_flag < 0) {
                    c[idx] = c[idx] * 2;
                }
            }
        }
        
        /* Reduction in host after target region */
        #pragma omp for reduction(+:result)
        for (i = 0; i < bound1; i++) {
            for (j = 0; j < bound2; j++) {
                int idx = i * bound2 + j;
                result += c[idx];
                
                /* Additional condition to prevent optimization */
                if (result > 1000000) {
                    result = result % 1000000;
                }
            }
        }
    }
    
    return result;
}

/* Another test function with different structure */
__attribute__((noinline))
int simt_test2(int size, int use_device) {
    int x[256], y[256], z[256];
    int i, j, sum = 0;
    
    for (i = 0; i < 256; i++) {
        x[i] = i;
        y[i] = 255 - i;
    }
    
    /* Nested OpenMP with target inside parallel */
    #pragma omp parallel for private(i, j)
    for (int outer = 0; outer < 4; outer++) {
        #pragma omp target teams distribute parallel for simd \
                if(use_device > 0) \
                map(to: x[0:256], y[0:256]) \
                map(from: z[outer*64:64]) \
                num_teams(2)
        for (i = 0; i < 64; i++) {
            int idx = outer * 64 + i;
            z[idx] = x[idx] * y[idx];
            
            /* Complex conditional to generate labels */
            if (z[idx] % 7 == 0) {
                z[idx] += outer;
                if (z[idx] > 10000) {
                    z[idx] = 10000;
                }
            }
        }
    }
    
    /* Final reduction with collapse */
    #pragma omp parallel for simd collapse(2) reduction(+:sum)
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            sum += z[i*16 + j];
        }
    }
    
    return sum;
}

int main() {
    int i, total = 0;
    volatile int flag = 1; /* Volatile to prevent constant propagation */
    
    printf("Starting SIMT transformation test...\n");
    
    /* Call test function with varying parameters to prevent optimization */
    for (i = 1; i <= 10; i++) {
        int threshold = i % 3; /* Varying threshold */
        int result = simt_test(i * 16, threshold, flag);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
        
        /* Alternate flag to change control flow */
        flag = -flag;
    }
    
    /* Second test with different pattern */
    for (i = 0; i < 5; i++) {
        int result = simt_test2(256, i % 2);
        total += result;
        printf("Test2 iteration %d: result = %d\n", i, result);
    }
    
    printf("Total sum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}

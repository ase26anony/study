/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization of helper function */
__attribute__((noinline))
int simt_test(int n, int threshold, volatile int *result) {
    volatile int i, j;
    int sum = 0;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Initialize arrays */
    for (int idx = 0; idx < SIZE; idx++) {
        a[idx] = idx % 100;
        b[idx] = (idx * 2) % 100;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) shared(a, b, c)
    {
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(n > threshold) \
                map(tofrom: a[0:SIZE], b[0:SIZE], c[0:SIZE]) \
                private(i, j)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Simple arithmetic operation */
                c[idx] = a[idx] + b[idx] + n;
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 200) {
                    /* Dummy operation that can't be optimized away */
                    c[idx] = c[idx] % 100;
                }
                
                /* Another condition to create more control flow */
                if (i > j && c[idx] < 50) {
                    c[idx] = c[idx] + 100;
                }
            }
        }
        
        /* Barrier to ensure target region completes */
        #pragma omp barrier
        
        /* Sequential region to compute sum */
        #pragma omp single
        {
            sum = 0;
            for (int idx = 0; idx < SIZE; idx++) {
                sum += c[idx];
            }
            *result = sum;
        }
    }
    
    return sum;
}

/* Another test function with different structure */
__attribute__((noinline))
int simt_test2(int n, volatile int *arr) {
    volatile int x, y;
    int total = 0;
    
    #pragma omp parallel
    {
        /* Nested target with teams and distribute */
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr[0:256]) \
                private(x, y) \
                reduction(+:total) \
                if(n % 2 == 0)
        for (x = 0; x < 16; x++) {
            for (y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                
                /* Complex expression to prevent optimization */
                arr[idx] = (arr[idx] * n + x * y) % 256;
                
                /* Early return simulation with conditional */
                if (arr[idx] == 0) {
                    arr[idx] = 1;  /* Prevents zero */
                }
                
                total += arr[idx];
            }
        }
    }
    
    return total;
}

int main(void) {
    volatile int results[10];
    volatile int test_array[256];
    int total_sum = 0;
    
    /* Initialize test array */
    for (int i = 0; i < 256; i++) {
        test_array[i] = i;
    }
    
    /* Call test functions with varying parameters to prevent
       constant propagation and optimization */
    for (int iter = 0; iter < 10; iter++) {
        int threshold = 5;
        int n = iter + 1;
        
        /* First test function */
        results[iter] = 0;
        int r1 = simt_test(n, threshold, &results[iter]);
        
        /* Second test function */
        int r2 = simt_test2(n, test_array);
        
        total_sum += r1 + r2;
        
        /* Print intermediate results to prevent dead code elimination */
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               iter, results[iter], r2);
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test with dynamic allocation */
    volatile int *dyn_arr = (volatile int*)malloc(1024 * sizeof(int));
    if (dyn_arr) {
        #pragma omp parallel
        {
            #pragma omp target teams distribute parallel for simd \
                    map(tofrom: dyn_arr[0:1024]) \
                    if(total_sum > 1000)
            for (int i = 0; i < 1024; i++) {
                dyn_arr[i] = i * total_sum;
                if (dyn_arr[i] > 1000000) {
                    dyn_arr[i] = 1000000;
                }
            }
        }
        
        /* Verify some values */
        int check = 0;
        for (int i = 0; i < 1024; i += 128) {
            check += dyn_arr[i];
        }
        printf("Dynamic array check sum: %d\n", check);
        
        free((void*)dyn_arr);
    }
    
    return 0;
}

/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Use volatile to prevent optimization */
volatile int global_threshold = 50;

/* Helper function marked noinline to preserve structure */
__attribute__((noinline, noipa))
int simt_test(int n, int threshold) {
    /* Volatile variables to prevent optimization */
    volatile int i, j;
    int sum = 0;
    
    /* Arrays with map clauses to force offloading */
    int a[TOTAL], b[TOTAL], c[TOTAL];
    
    /* Initialize arrays */
    for (int idx = 0; idx < TOTAL; idx++) {
        a[idx] = idx % 100;
        b[idx] = (idx * 2) % 100;
        c[idx] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel private(i, j) reduction(+:sum)
    {
        /* Target region with if clause for conditional offloading */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(n > threshold) \
                map(to: a[0:TOTAL], b[0:TOTAL]) \
                map(from: c[0:TOTAL]) \
                num_teams(4) thread_limit(64)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Simple arithmetic operation */
                c[idx] = a[idx] + b[idx] + n;
                
                /* Conditional to create multiple basic blocks */
                if (c[idx] > 150) {
                    /* Early exit simulation - creates extra basic blocks */
                    if (c[idx] > 200 && i > N/2) {
                        /* Dummy operation that can't be optimized away */
                        c[idx] = c[idx] % 100;
                    } else {
                        /* Another branch for more complexity */
                        c[idx] = c[idx] + 1;
                    }
                } else {
                    /* Alternative path */
                    c[idx] = c[idx] - 1;
                }
                
                /* Additional condition to force label generation */
                if (j == M-1 && c[idx] < 0) {
                    /* This creates another control flow edge */
                    c[idx] = -c[idx];
                }
            }
        }
        
        /* Reduction after target region */
        #pragma omp for
        for (int idx = 0; idx < TOTAL; idx++) {
            sum += c[idx];
        }
    }
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, noipa))
int simt_test2(int size, int flag) {
    volatile int x, y;
    int result = 0;
    int arr1[256], arr2[256], arr3[256];
    
    for (int i = 0; i < 256; i++) {
        arr1[i] = i;
        arr2[i] = 256 - i;
        arr3[i] = 0;
    }
    
    /* Different nesting pattern */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp target teams distribute parallel for simd \
                    if(flag) map(tofrom: arr3) map(to: arr1, arr2) \
                    num_teams(2)
            for (x = 0; x < 16; x++) {
                for (y = 0; y < 16; y++) {
                    int idx = x * 16 + y;
                    arr3[idx] = arr1[idx] * arr2[idx];
                    
                    /* Complex conditional structure */
                    switch (arr3[idx] % 4) {
                        case 0:
                            arr3[idx] += size;
                            break;
                        case 1:
                            arr3[idx] -= size;
                            /* Nested if inside switch */
                            if (arr3[idx] < 0) {
                                arr3[idx] = 0;
                            }
                            break;
                        default:
                            arr3[idx] = arr3[idx] / 2;
                            /* Another conditional */
                            if (x > y) {
                                arr3[idx] += 100;
                            }
                    }
                }
            }
        }
        
        #pragma omp for reduction(+:result)
        for (int i = 0; i < 256; i++) {
            result += arr3[i];
        }
    }
    
    return result;
}

int main() {
    int total_sum = 0;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (int iter = 0; iter < 10; iter++) {
        int n = iter * 15;
        int threshold = global_threshold + iter;
        
        /* Call test function with varying parameters */
        int result = simt_test(n, threshold);
        total_sum += result;
        
        printf("Iteration %d: n=%d, threshold=%d, result=%d\n", 
               iter, n, threshold, result);
        
        /* Second test with different parameters */
        if (iter % 2 == 0) {
            int result2 = simt_test2(iter, iter > 5);
            total_sum += result2;
            printf("  Second test result: %d\n", result2);
        }
    }
    
    printf("Total sum: %d\n", total_sum);
    
    /* Additional test with dynamic teams */
    #pragma omp parallel
    {
        int local_arr[100];
        
        #pragma omp for
        for (int i = 0; i < 100; i++) {
            local_arr[i] = i;
        }
        
        #pragma omp target teams distribute parallel for simd \
                map(tofrom: local_arr) if(1) \
                num_teams(omp_get_num_threads())
        for (int i = 0; i < 100; i++) {
            local_arr[i] = local_arr[i] * 2;
            /* Multiple basic blocks */
            if (local_arr[i] > 100) {
                local_arr[i] = 100;
                if (i % 3 == 0) {
                    local_arr[i] += i;
                }
            }
        }
    }
    
    return total_sum > 0 ? 0 : 1;
}

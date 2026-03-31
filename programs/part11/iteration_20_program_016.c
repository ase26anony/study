/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 32
#define M 32
#define SIZE (N * M)

/* Prevent optimization and inlining */
volatile int g_volatile_counter = 0;
static volatile int s_volatile_bound = SIZE;

/* Helper function marked noinline to preserve structure */
__attribute__((noinline, target("noipa")))
int simt_test(int n, int threshold, int iter) {
    volatile int local_volatile = iter;
    int i, j;
    
    /* Arrays with volatile elements to prevent optimization */
    volatile int a[SIZE];
    volatile int b[SIZE];
    volatile int c[SIZE];
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i + local_volatile;
        b[i] = i * 2 + local_volatile;
        c[i] = 0;
    }
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel num_threads(2) private(i, j)
    {
        int thread_id = omp_get_thread_num();
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) \
                if(n > threshold) \
                map(tofrom: a, b, c) \
                num_teams(2) \
                thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Force multiple basic blocks with conditional */
                if (thread_id == 0) {
                    /* First basic block path */
                    c[idx] = a[idx] + b[idx] + local_volatile;
                    
                    /* Early exit condition to create label jumps */
                    if (c[idx] > 1000 && j % 8 == 0) {
                        /* Dummy operation that can't be optimized away */
                        c[idx] = c[idx] % 256;
                        /* Continue to encourage multiple labels */
                        if (c[idx] == 0) {
                            c[idx] = 1;
                        }
                    }
                } else {
                    /* Second basic block path */
                    c[idx] = a[idx] - b[idx] - local_volatile;
                    
                    /* Another conditional to create more labels */
                    if (c[idx] < -100 && i % 4 == 0) {
                        c[idx] = -c[idx];
                        if (j == M-1) {
                            c[idx] = 0;
                        }
                    }
                }
                
                /* Additional complexity with volatile */
                if (g_volatile_counter > 10000) {
                    c[idx] = 0; /* This path unlikely but prevents optimization */
                }
            }
        }
        
        /* Additional computation outside target to maintain context */
        #pragma omp for simd
        for (i = 0; i < SIZE; i++) {
            if (c[i] > 100) {
                c[i] = 100;
            } else if (c[i] < -100) {
                c[i] = -100;
            }
        }
    }
    
    /* Compute reduction sum to prevent dead code elimination */
    int sum = 0;
    volatile int* volatile_ptr = c;
    for (i = 0; i < SIZE; i++) {
        sum += volatile_ptr[i];
    }
    
    /* Use volatile to prevent optimization */
    g_volatile_counter += sum;
    
    return sum;
}

/* Another test function with different construct nesting */
__attribute__((noinline, target("noipa")))
int simt_test2(int n, int flag) {
    volatile int arr[256];
    int i, j;
    
    for (i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    
    /* Different nesting pattern */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                #pragma omp target teams distribute parallel for simd \
                        if(flag) \
                        map(tofrom: arr) \
                        collapse(2)
                for (i = 0; i < 16; i++) {
                    for (j = 0; j < 16; j++) {
                        int idx = i * 16 + j;
                        
                        /* Complex conditional structure */
                        switch (idx % 4) {
                            case 0:
                                arr[idx] += n;
                                if (arr[idx] > 1000) arr[idx] = 1000;
                                break;
                            case 1:
                                arr[idx] -= n;
                                if (arr[idx] < 0) arr[idx] = 0;
                                break;
                            case 2:
                                arr[idx] *= 2;
                                /* Force basic block split */
                                if (arr[idx] % 3 == 0) {
                                    arr[idx] /= 3;
                                }
                                break;
                            default:
                                arr[idx] = arr[idx] / 2;
                                /* Early return simulation */
                                if (arr[idx] == 0 && i > 8) {
                                    arr[idx] = 1;
                                }
                        }
                    }
                }
            }
        }
    }
    
    int sum = 0;
    for (i = 0; i < 256; i++) {
        sum += arr[i];
    }
    
    return sum;
}

int main() {
    int total = 0;
    int i;
    
    printf("Starting SIMT transformation test...\n");
    
    /* Varying arguments to prevent constant propagation */
    for (i = 1; i <= 10; i++) {
        int threshold = 5;
        int result1 = simt_test(i, threshold, i * 10);
        int result2 = simt_test2(i, i % 2);
        
        total += result1 + result2;
        
        printf("Iteration %d: result1 = %d, result2 = %d, total = %d\n",
               i, result1, result2, total);
    }
    
    /* Additional test with dynamic thread count */
    omp_set_dynamic(0);
    omp_set_num_threads(4);
    
    #pragma omp parallel for simd
    for (i = 0; i < 100; i++) {
        g_volatile_counter += i;
    }
    
    printf("Final volatile counter: %d\n", g_volatile_counter);
    printf("Total sum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}

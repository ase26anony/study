/* Compile with: -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower */
/* For AMD GPUs: -foffload=amdgcn-amdhsa */

#include <stdio.h>
#include <stdlib.h>

#define N 128
#define M 64
#define TOTAL (N * M)

/* Prevent optimization and inlining */
volatile int global_seed = 42;
static int __attribute__((noinline)) use_result(int sum) {
    volatile int tmp = sum;
    return tmp + 1;
}

/* Main test function with nested OpenMP constructs */
__attribute__((noinline))
int simt_test(int n, int threshold) {
    /* Volatile to prevent optimization */
    volatile int vn = n;
    int a[TOTAL], b[TOTAL], c[TOTAL];
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < TOTAL; i++) {
        a[i] = i + global_seed;
        b[i] = i * 2 - global_seed;
        c[i] = 0;
    }
    
    int result = 0;
    
    /* Outer parallel region to create nesting context */
    #pragma omp parallel shared(a, b, c, result) firstprivate(vn, threshold)
    {
        int tid = omp_get_thread_num();
        
        /* Target region with conditional offloading and SIMD */
        #pragma omp target teams distribute parallel for simd \
                collapse(2) if(vn > threshold) \
                map(tofrom: a[0:TOTAL], b[0:TOTAL], c[0:TOTAL]) \
                num_teams(4) thread_limit(128)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                int idx = i * M + j;
                
                /* Main computation */
                c[idx] = a[idx] + b[idx] + tid;
                
                /* Conditional break to create multiple basic blocks */
                if (c[idx] > 1000 && idx % 32 == 0) {
                    /* Early exit creates additional control flow */
                    c[idx] = 999;
                    /* This creates additional basic blocks and labels */
                }
                
                /* Another condition to encourage label generation */
                if (idx % 16 == 0) {
                    c[idx] += (vn > 5) ? 1 : 0;
                }
            }
        }
        
        /* Reduction inside parallel region */
        #pragma omp for reduction(+:result) nowait
        for (i = 0; i < TOTAL; i++) {
            result += c[i];
        }
    }
    
    return use_result(result);
}

/* Secondary test with different construct nesting */
__attribute__((noinline))
int simt_test2(int n, int flag) {
    volatile int vflag = flag;
    int x[N][M], y[N][M], z[N][M];
    int i, j;
    
    /* Initialize with volatile to prevent constant propagation */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            x[i][j] = (i * j + global_seed) % 100;
            y[i][j] = (i + j * 2) % 100;
        }
    }
    
    int sum = 0;
    
    /* Different nesting pattern */
    #pragma omp parallel if(vflag > 0)
    {
        #pragma omp single
        {
            /* Teams construct with distribute */
            #pragma omp target teams distribute parallel for simd \
                    map(to: x, y) map(from: z) \
                    if(omp_get_num_threads() > 1) \
                    collapse(2)
            for (i = 0; i < N; i++) {
                for (j = 0; j < M; j++) {
                    z[i][j] = x[i][j] * y[i][j];
                    
                    /* Complex condition with early continue */
                    if (z[i][j] < 0) {
                        z[i][j] = 0;
                        continue;
                    }
                    
                    /* Another condition for basic block splitting */
                    if (j % 8 == 0 && i > N/2) {
                        z[i][j] += 1000;
                    }
                }
            }
        }
        
        /* Reduction with atomic to create more IR complexity */
        #pragma omp for
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                #pragma omp atomic
                sum += z[i][j];
            }
        }
    }
    
    return sum;
}

int main() {
    int total = 0;
    int i;
    
    /* Varying parameters to prevent constant folding */
    for (i = 1; i <= 10; i++) {
        int threshold = (i % 3) + 2;  /* Vary threshold */
        int flag = i % 2;
        
        /* Call both test functions with varying args */
        int res1 = simt_test(i * 10, threshold);
        int res2 = simt_test2(i * 5, flag);
        
        total += res1 + res2;
        
        /* Print to prevent dead code elimination */
        printf("Iteration %d: res1=%d, res2=%d\n", i, res1, res2);
    }
    
    printf("Total sum: %d\n", total);
    
    /* Additional test with explicit SIMD clause */
    {
        int arr[1000];
        #pragma omp parallel for simd simdlen(8)
        for (i = 0; i < 1000; i++) {
            arr[i] = i * i;
        }
        
        int check = 0;
        for (i = 0; i < 1000; i++) {
            check += arr[i];
        }
        printf("SIMD test check: %d\n", check);
    }
    
    return 0;
}

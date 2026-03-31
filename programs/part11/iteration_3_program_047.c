/* test_ddg_coverage.c
 * Complex loop nests with various dependencies to trigger DDG edge creation
 * Compile with: gcc -O2 -fmodulo-sched -fdump-ddg test_ddg_coverage.c -o test_ddg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

/* Prevent inlining to ensure separate DDG construction for each kernel */
__attribute__((noinline)) 
void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    volatile int sink = 0; /* Prevent optimization */
    
    /* Triple-nested loop with flow dependencies across all dimensions */
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* Complex strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+1] + arr1[i][j-1] * 2 - arr1[i-2][j];
                arr2[i][j] = arr1[i][j] + arr2[i-1][j+2] - arr2[i][j-2];
                
                /* Cross-dimensional dependency */
                if (k % 3 == 0) {
                    arr1[i][j] += arr2[i-1][j] * arr1[i][j-1];
                } else {
                    arr1[i][j] -= arr2[i][j+1] / (arr1[i-1][j] + 1);
                }
                
                /* Memory barrier to preserve dependencies */
                asm volatile("" ::: "memory");
            }
        }
    }
}

__attribute__((noinline))
void kernel2_aliasing_dependencies(int* arr, int* brr) {
    /* Create aliasing pointers */
    int* p = &arr[0];
    int* q = &arr[1];  /* q aliases with p+1 */
    int* r = &brr[0];
    int* s = &brr[10]; /* s aliases with r+10 */
    
    volatile int accumulator = 0;
    
    /* Loop with anti-dependencies (WAR) and output dependencies (WAW) */
    for (int i = 10; i < N-10; i++) {
        /* Read through p (will create anti-dependency with write through q) */
        int temp = *p + accumulator;
        
        /* Write through q (aliases p+1) - creates WAR */
        *q = temp * 2 + i;
        
        /* Output dependency (WAW) on same location through different pointers */
        if (i % 4 == 0) {
            arr[i/2] = *r + *s;
        } else {
            arr[i/2] = *r - *s;
        }
        
        /* Read after write to create flow dependency */
        *r = arr[i/2] + *q;
        
        /* Pointer arithmetic to change aliasing pattern */
        p = &arr[i % 16];
        q = &arr[(i + 1) % 16];
        
        /* Varying dependency distances */
        if (i % 5 == 0) {
            s = &brr[i-3];  /* Distance 3 */
        } else if (i % 3 == 0) {
            s = &brr[i-2];  /* Distance 2 */
        } else {
            s = &brr[i-1];  /* Distance 1 */
        }
        
        accumulator += *p;
    }
}

__attribute__((noinline))
void kernel3_restrict_pointers(double* restrict a, double* restrict b, 
                               double* restrict c, double* c2) {
    /* restrict pointers allow better dependency analysis */
    /* but c2 aliases with c to create output dependencies */
    
    for (int i = 2; i < N-2; i++) {
        /* Flow dependencies with restrict */
        a[i] = b[i-1] * c[i-2] + a[i-1];
        b[i] = a[i] / (c[i-1] + 1.0);
        
        /* Output dependency through aliasing pointer */
        c2[i] = a[i] + b[i];  /* c2 aliases c, so WAW with c[i] */
        
        /* Complex index calculations for non-linear access */
        int idx = (i * 7) % N;
        int idx2 = (i * 13) % N;
        
        /* Cross-iteration dependencies with varying distances */
        if (i % 7 == 0) {
            c[idx] = a[idx2] - b[idx] + c[idx-4];  /* Distance 4 */
        } else if (i % 3 == 0) {
            c[idx] = a[idx2] * b[idx] + c[idx-2];  /* Distance 2 */
        } else {
            c[idx] = a[idx2] + b[idx] - c[idx-1];  /* Distance 1 */
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
}

__attribute__((noinline))
void kernel4_mixed_types(volatile int* vi, float* vf, double* vd, char* vc) {
    union mixed {
        int i;
        float f;
        char bytes[4];
    } u;
    
    /* Mixed data type dependencies */
    for (int i = 4; i < N-4; i++) {
        /* Type casting creates data dependencies */
        u.i = vi[i-1];
        vf[i] = (float)u.i * 0.5f + vf[i-2];
        
        /* Bitwise operations */
        u.i = (vi[i-3] << 2) | (vi[i-4] >> 3);
        vd[i] = (double)u.i / 3.14 + vd[i-1];
        
        /* Char array access with memcpy dependency */
        memcpy(&vc[i*4], &u.bytes[0], 4);
        
        /* Dependency through union */
        u.f = vf[i-1];
        vi[i] = u.i ^ vi[i-2];
        
        /* Volatile access creates hard dependency */
        vi[i] += *vi;  /* *vi is volatile qualified */
        
        /* Inline assembly with memory clobber */
        asm volatile("" : "+r"(vi[i]) : : "memory");
        
        /* Mixed precision operations */
        if (i % 8 == 0) {
            vd[i] = (double)vf[i] * vd[i-4] - (double)vi[i-2];
        } else {
            vf[i] = (float)vd[i] + vf[i-1] * (float)vi[i-3];
        }
    }
}

/* Simple PRNG for initialization to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main(void) {
    /* Allocate multi-dimensional arrays with different sizes */
    int arr1[N][M];
    int arr2[N][M];
    int linear_arr[N*2];
    double darr1[N];
    double darr2[N];
    double darr3[N];
    float farr[N];
    char carr[N*4];
    volatile int volatile_arr[N];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
        linear_arr[i] = (int)lcg_rand() % 1000;
        darr1[i] = (double)(lcg_rand() % 1000) / 3.0;
        darr2[i] = (double)(lcg_rand() % 1000) / 7.0;
        darr3[i] = (double)(lcg_rand() % 1000) / 11.0;
        farr[i] = (float)(lcg_rand() % 1000) / 5.0f;
        volatile_arr[i] = (int)lcg_rand() % 100;
    }
    
    /* Initialize char array */
    for (int i = 0; i < N*4; i++) {
        carr[i] = (char)(lcg_rand() % 256);
    }
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_dependencies(arr1, arr2);
    
    /* Modify data between kernels using volatile to prevent optimization */
    for (int i = 0; i < 10; i++) {
        volatile_arr[i] = arr1[i][0] + arr2[i][0];
    }
    asm volatile("" ::: "memory");
    
    kernel2_aliasing_dependencies(linear_arr, &linear_arr[N]);
    
    /* More volatile operations between kernels */
    volatile_arr[10] = linear_arr[50] * 2;
    asm volatile("" ::: "memory");
    
    /* darr3 aliases with darr2 for output dependencies */
    kernel3_restrict_pointers(darr1, darr2, darr3, darr3);
    
    volatile_arr[20] = (int)darr1[30] + (int)darr2[40];
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(volatile_arr, farr, darr1, carr);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
        checksum += linear_arr[i] + (int)darr1[i] + (int)darr2[i] + 
                   (int)darr3[i] + (int)farr[i] + volatile_arr[i];
    }
    
    for (int i = 0; i < N*4; i++) {
        checksum += carr[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}

/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128
#define ITERS 10

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0;
    
    /* Complex 3-level nested loop with true dependencies (RAW) */
    for (int i = 2; i < N-2; i++) {
        for (int j = 3; j < M-3; j += 2) {  /* Non-unit stride */
            for (int k = 1; k < P; k++) {
                /* Flow dependencies with varying distances */
                arr1[i][j] = arr1[i-2][j+1] + arr1[i-1][j-1];  /* Distance 2 and 1 */
                arr2[i][j] = arr2[i][j-3] * 2 + arr1[i][j];    /* Anti-dependency on arr2 */
                
                /* Conditional dependency distance */
                if ((i + j) % 4 == 0) {
                    arr1[i-1][j+2] = arr1[i][j] + arr2[i-2][j];  /* Complex dependency web */
                } else if ((i + j) % 3 == 0) {
                    arr2[i][j] = arr1[i-3][j+1] - arr2[i][j-2];  /* Different distance */
                }
                
                /* Memory barrier to prevent optimization */
                asm volatile("" ::: "memory");
            }
        }
        barrier++;  /* Volatile operation creates artificial dependency */
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_pointer_aliasing(double* arr3, float* arr4, int size) {
    /* Create aliasing pointers */
    double* p1 = &arr3[0];
    double* p2 = &arr3[1];  /* p2 aliases p1+1 */
    double* p3 = &arr3[size/2];
    float* f1 = &arr4[0];
    float* f2 = &arr4[1];   /* f2 aliases f1+1 */
    
    volatile double v = 0.0;
    
    for (int i = 2; i < size - 5; i++) {
        /* Anti-dependencies through aliasing pointers */
        double temp = *p1;           /* Read p1 */
        *p2 = temp * 3.14159;        /* Write p2 (aliases p1+1) - WAR */
        
        /* Output dependencies through different pointers */
        *p3 = *p1 + *p2;             /* WAW potential with p1/p2 */
        
        /* Pointer arithmetic creates complex access patterns */
        p1 = &arr3[i];
        p2 = &arr3[i+1];
        p3 = &arr3[(i * 17) % (size-1)];  /* Non-linear access */
        
        /* Mixed-type dependencies */
        *f1 = (float)(*p1) * 2.0f;
        *f2 = *f1 + (float)(*p2);    /* Flow dependency on f1 */
        
        f1 = &arr4[i % 128];
        f2 = &arr4[(i + 37) % 128];
        
        /* Volatile forces dependency */
        v += *p1;
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_restrict_pointers(int* restrict r1, int* restrict r2, 
                                      int* restrict r3, int len) {
    /* restrict qualifier allows aggressive optimization but still creates DDG edges */
    volatile int sync = 0;
    
    for (int i = 4; i < len - 4; i += 3) {  /* Non-unit stride */
        /* Output dependencies within restrict pointers */
        r1[i] = r2[i-2] + r3[i-1];      /* Flow: RAW on r2 and r3 */
        r2[i] = r1[i] * r1[i-1];        /* Flow: RAW on r1 */
        r3[i] = r2[i] - r3[i-3];        /* Flow: RAW on r2, r3 with distance 3 */
        
        /* WAW dependencies */
        if (i % 5 == 0) {
            r1[i] = r1[i-4] + r2[i-2];  /* WAW on r1[i] */
        } else {
            r1[i] = r3[i-1] * 2;        /* Alternative WAW on r1[i] */
        }
        
        /* Loop-carried dependency with varying distance */
        int dep_dist = (i % 7) + 1;
        r2[i + dep_dist] = r1[i] + r2[i];
        
        sync = i;  /* Volatile write */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* cbuf, int* ibuf, float* fbuf, 
                                double* dbuf, int size) {
    union mixed_data {
        int i;
        float f;
        char bytes[4];
    } u;
    
    volatile union mixed_data vu;
    
    for (int i = 8; i < size - 8; i++) {
        /* Type-punning through union creates dependencies */
        u.i = ibuf[i-2];
        fbuf[i] = u.f * 1.5f;           /* Dependency through union */
        
        /* Bitwise operations with type casting */
        ibuf[i] = (int)(fbuf[i-1]) ^ ibuf[i-4];  /* Mixed-type dependency */
        
        /* Memory operations create dependencies */
        memcpy(&cbuf[i], &cbuf[i-1], 4);  /* memcpy creates flow dependency */
        
        /* Inline assembly with memory clobber */
        asm volatile(
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m" (ibuf[i+1])          /* Output operand */
            : "m" (ibuf[i-1])           /* Input operand */
            : "%eax", "memory"
        );
        
        /* Double/float mixing */
        dbuf[i] = (double)fbuf[i] + dbuf[i-2];
        
        /* Char/int mixing with pointer casting */
        *(int*)(&cbuf[(i*3) % size]) = ibuf[i-3];
        
        vu.i = i;  /* Volatile union access */
        asm volatile("" ::: "memory");
    }
}

/* Main function orchestrating all kernels */
int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int arr1[N][M];
    int arr2[N][M];
    double arr3[1024];
    float arr4[512];
    char cbuf[2048];
    int ibuf[1024];
    float fbuf[1024];
    double dbuf[1024];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        arr3[i] = (double)(lcg_rand() % 10000) / 100.0;
        ibuf[i] = lcg_rand() % 1000;
        fbuf[i] = (float)(lcg_rand() % 1000) / 10.0f;
        dbuf[i] = (double)(lcg_rand() % 10000) / 50.0;
    }
    
    for (int i = 0; i < 512; i++) {
        arr4[i] = (float)(lcg_rand() % 1000) / 5.0f;
    }
    
    for (int i = 0; i < 2048; i++) {
        cbuf[i] = (char)(lcg_rand() % 256);
    }
    
    volatile int iteration_barrier = 0;
    
    /* Execute kernels multiple times to ensure DDG construction */
    for (int iter = 0; iter < ITERS; iter++) {
        kernel1_flow_deps(arr1, arr2);
        iteration_barrier++;
        
        kernel2_pointer_aliasing(arr3, arr4, 1024);
        iteration_barrier++;
        
        kernel3_restrict_pointers(ibuf, &arr1[0][0], &arr2[0][0], 1024);
        iteration_barrier++;
        
        kernel4_mixed_types(cbuf, ibuf, fbuf, dbuf, 1024);
        iteration_barrier++;
        
        /* Modify arrays between iterations to prevent cross-iteration optimization */
        for (int i = 0; i < 16; i++) {
            arr1[i][i] = lcg_rand() % 100;
            arr3[i] += 1.0;
            ibuf[i] ^= 0xFF;
        }
        asm volatile("" ::: "memory");
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (long long)arr3[i] + ibuf[i] + (long long)fbuf[i] + (long long)dbuf[i];
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (long long)arr4[i];
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += cbuf[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 512
#define Q 1024

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile variable to prevent cross-kernel optimization */
static volatile int volatile_barrier = 0;

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* Flow dependencies (RAW) across i dimension */
    for (i = 1; i < N - 1; i++) {
        for (j = 1; j < M - 1; j++) {
            for (k = 1; k < 8; k++) {
                /* Multi-dimensional strided access with flow dependency */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j] * k;
                /* Additional dependency chain */
                arr2[i][j] = arr1[i][j-1] + arr2[i-2][j+1];
            }
        }
    }
    
    /* Loop-carried dependency with varying distance */
    for (i = 2; i < N - 2; i++) {
        if (i % 3 == 0) {
            arr1[i][0] = arr1[i-2][0] + arr1[i-1][0];  /* Distance 2 */
        } else if (i % 5 == 0) {
            arr1[i][0] = arr1[i-3][0] * 2;  /* Distance 3 */
        } else {
            arr1[i][0] = arr1[i-1][0] + 1;  /* Distance 1 */
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* arr, int* brr) {
    int i;
    int *p = &arr[0];
    int *q = &arr[1];  /* q aliases with p+1 */
    int *r = &brr[0];
    
    /* Anti-dependencies through aliasing pointers */
    for (i = 1; i < P - 1; i++) {
        int temp = *p;          /* Read from p */
        *q = temp + r[i];       /* Write to q (aliases p+1) - WAR */
        p = &arr[i];            /* Move p */
        q = &arr[i+1];          /* Move q */
        
        /* Additional anti-dependency chain */
        r[i] = arr[i-1] * 2;    /* Read arr[i-1], write r[i] */
        arr[i] = r[i] + 1;      /* Read r[i], write arr[i] - WAR */
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict a, double* restrict b, 
                                double* restrict c) {
    int i, j;
    
    /* Output dependencies (WAW) with restrict */
    for (i = 0; i < Q/2; i++) {
        a[i] = b[i] * c[i];
        a[i] = a[i] + 1.0;      /* WAW on a[i] */
        
        /* Strided access with output dependency */
        if (i % 4 == 0) {
            b[i*2] = c[i] * 2.0;
            b[i*2] = b[i*2] + a[i];  /* WAW on b[i*2] */
        }
    }
    
    /* Nested loop with mixed dependencies */
    for (i = 1; i < 64; i++) {
        for (j = 1; j < 64; j++) {
            int idx = i * 8 + j;
            c[idx] = a[idx-1] + b[idx-8];  /* Flow dependency */
            a[idx] = c[idx] * 0.5;         /* Flow dependency */
            c[idx] = a[idx] + b[idx];      /* WAW on c[idx] */
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(float* farr, double* darr, char* carr, 
                                int* iarr) {
    int i;
    union {
        int i;
        float f;
        char bytes[4];
    } converter;
    
    /* Mixed type dependency chain */
    for (i = 1; i < P - 4; i++) {
        /* Float operations with dependency */
        farr[i] = farr[i-1] * 1.5f + (float)iarr[i];
        
        /* Type casting creates implicit dependencies */
        converter.f = farr[i];
        iarr[i] = converter.i;
        
        /* Inline assembly memory barrier */
        asm volatile("" ::: "memory");
        
        /* Char array with dependency through memcpy */
        carr[i] = (char)(iarr[i] & 0xFF);
        memcpy(&carr[i+1], &carr[i], 1);  /* Creates dependency */
        
        /* Double with volatile read */
        darr[i] = (double)volatile_barrier + darr[i-1] * 0.9;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* Bitwise operations with dependency */
        iarr[i+1] = (iarr[i] << 3) | (iarr[i-1] & 0x7);
    }
    
    /* Loop with varying dependency distances */
    for (i = 4; i < P - 4; i++) {
        switch (i % 4) {
            case 0:
                farr[i] = farr[i-4] * 2.0f;  /* Distance 4 */
                break;
            case 1:
                farr[i] = farr[i-2] + farr[i-1];  /* Distances 1 and 2 */
                break;
            case 2:
                farr[i] = farr[i-3] - farr[i-1];  /* Distances 1 and 3 */
                break;
            case 3:
                farr[i] = farr[i-1] * farr[i-2];  /* Distances 1 and 2 */
                break;
        }
    }
}

/* Kernel 5: Complex pointer aliasing with overlapping regions */
__attribute__((noinline))
static void kernel5_complex_aliasing(int* base, int size) {
    int* p1 = base;
    int* p2 = base + size/4;
    int* p3 = base + size/2;
    int* p4 = base + 3*size/4;
    int i;
    
    /* All pointers may alias due to unknown size */
    for (i = 0; i < size/8; i++) {
        /* Complex web of potential aliasing dependencies */
        int val1 = p1[i];
        p2[i] = val1 + i;
        
        int val2 = p3[i];
        p4[i] = val2 * 2;
        
        /* p1 and p3 might alias if size is small */
        p1[i+1] = p3[i] + p4[i];
        
        /* p2 and p4 might alias */
        p3[i] = p2[i] - p1[i];
        
        /* Insert barrier to prevent optimization */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    float farr[P];
    double darr[Q];
    char carr[P];
    int iarr[P];
    int* dynamic_arr = malloc(Q * sizeof(int));
    
    /* Initialize with pseudo-random values using LCG */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < P; i++) {
        farr[i] = (float)(lcg_rand() % 1000) / 10.0f;
        carr[i] = (char)(lcg_rand() % 256);
        iarr[i] = lcg_rand() % 10000;
    }
    
    for (int i = 0; i < Q; i++) {
        darr[i] = (double)(lcg_rand() % 1000) / 3.0;
        if (dynamic_arr) dynamic_arr[i] = lcg_rand() % 5000;
    }
    
    /* Execute kernels with volatile barriers between them */
    kernel1_flow_deps(arr1, arr2);
    volatile_barrier = arr1[0][0];
    
    kernel2_anti_deps(&arr1[0][0], iarr);
    volatile_barrier = iarr[0];
    
    kernel3_output_deps(darr, darr + Q/2, darr + Q/4);
    volatile_barrier = (int)darr[0];
    
    kernel4_mixed_types(farr, darr, carr, iarr);
    volatile_barrier = (int)farr[0];
    
    if (dynamic_arr) {
        kernel5_complex_aliasing(dynamic_arr, Q);
        volatile_barrier = dynamic_arr[0];
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (unsigned)arr1[i][j];
            checksum += (unsigned)arr2[i][j];
        }
    }
    
    for (int i = 0; i < P; i++) {
        checksum += (unsigned)farr[i];
        checksum += (unsigned)carr[i];
        checksum += (unsigned)iarr[i];
    }
    
    for (int i = 0; i < Q; i++) {
        checksum += (unsigned)darr[i];
        if (dynamic_arr) checksum += (unsigned)dynamic_arr[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    if (dynamic_arr) free(dynamic_arr);
    return 0;
}

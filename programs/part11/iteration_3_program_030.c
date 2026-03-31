#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile sink to prevent optimization */
volatile int volatile_sink;

/* ========== KERNEL 1: Triple-nested loop with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M], double arr3[P]) {
    int i, j, k;
    
    /* Flow dependencies across all dimensions */
    for (i = 1; i < N; i++) {
        for (j = 1; j < M - 2; j++) {
            /* RAW dependency: read after write with varying distances */
            arr1[i][j] = arr1[i-1][j+2] + arr2[i][j];
            
            /* Additional dependency with non-unit stride */
            if (j % 3 == 0) {
                arr2[i][j] = arr1[i][j-3] * 2;
            } else {
                arr2[i][j] = arr1[i][j-1] + arr1[i-1][j];
            }
        }
    }
    
    /* Third dimension with loop-carried dependency */
    for (k = 2; k < P; k++) {
        /* Mixed distance dependencies */
        if (k % 4 == 0) {
            arr3[k] = arr3[k-2] * 1.5;
        } else if (k % 4 == 1) {
            arr3[k] = arr3[k-1] + arr3[k-3];
        } else {
            arr3[k] = arr3[k-1] * 0.75;
        }
        
        /* Cross-array dependency */
        arr3[k] += (double)arr1[k % N][k % M];
    }
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* q aliases p+1 */
    int *r = &base_arr[size/2]; /* Potential aliasing */
    
    int i;
    
    /* WAR (anti-dependency) patterns */
    for (i = 1; i < size - 1; i++) {
        int temp = p[i];      /* Read from p[i] */
        p[i-1] = temp + i;    /* Write to p[i-1] - anti-dep with next iteration */
        
        /* Complex aliasing pattern */
        q[i] = p[i] * 2;      /* q[i] aliases p[i+1], creating WAR/WAW */
        
        /* Conditional aliasing */
        if (i % 8 == 0) {
            r[i % (size/2)] = p[i] + q[i-1];
        }
    }
    
    /* Additional loop with overlapping regions */
    int *alias1 = &base_arr[10];
    int *alias2 = &base_arr[15]; /* Overlap: alias2 = alias1 + 5 */
    
    for (i = 0; i < size - 20; i++) {
        /* WAR: Read then write to overlapping region */
        int val1 = alias1[i+5];      /* Reads what alias2[i] points to */
        alias2[i] = val1 * 3;        /* Writes to same location read in future iterations */
        
        /* WAW: Multiple writes to same location */
        if (i % 3 == 0) {
            alias1[i+5] = i;         /* Same location as alias2[i] */
        }
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_output_deps(int* restrict res1, int* restrict res2, 
                                const int* restrict src1, const int* restrict src2, int len) {
    int i;
    
    /* WAW (output dependency) patterns */
    for (i = 0; i < len; i++) {
        /* Multiple writes to same output */
        res1[i] = src1[i] + src2[i];
        res1[i] = res1[i] * 2;        /* WAW on res1[i] */
        
        /* Chain of WAW dependencies */
        res2[i] = res1[i] - i;
        res2[i] = res2[i] * res2[i];  /* WAW on res2[i] */
    }
    
    /* Loop-carried output dependencies */
    for (i = 1; i < len; i++) {
        /* res1[i] depends on previous res1[i-1] calculation */
        res1[i] = res1[i-1] + res2[i];
        
        /* Conditional WAW */
        if (i % 5 == 0) {
            res1[i] = res1[i] >> 2;   /* Another write to res1[i] */
        }
    }
}

/* ========== KERNEL 4: Mixed data types and assembly barriers ========== */
__attribute__((noinline))
static void kernel4_mixed_types(char* cbuf, int* ibuf, float* fbuf, double* dbuf, int size) {
    int i;
    volatile int vol_var = 0;
    
    /* Mixed type dependency chain */
    for (i = 1; i < size; i++) {
        /* char -> int -> float -> double chain */
        int ival = (int)cbuf[i] + (int)cbuf[i-1];
        ibuf[i] = ival * 2;
        
        /* Memory barrier to enforce dependency */
        asm volatile("" ::: "memory");
        
        float fval = (float)ibuf[i] / 3.0f;
        fbuf[i] = fval + fbuf[i-1];
        
        /* Another barrier */
        asm volatile("" ::: "memory");
        
        dbuf[i] = (double)fbuf[i] * 1.5;
        
        /* Volatile operation to prevent optimization */
        vol_var = i;
        volatile_sink = vol_var;
    }
    
    /* Union-based type punning for additional dependencies */
    union {
        int i;
        float f;
        char bytes[4];
    } converter;
    
    for (i = 0; i < size - 4; i++) {
        /* Type-punning dependencies */
        converter.i = ibuf[i];
        fbuf[i] = converter.f;  /* RAW through union */
        
        converter.f = fbuf[i] * 2.0f;
        ibuf[i+1] = converter.i; /* WAR through union */
        
        /* Memory function with dependencies */
        if (i % 16 == 0) {
            memcpy(&cbuf[i], &cbuf[i+4], 4); /* Creates RAW/WAW dependencies */
        }
    }
}

/* ========== KERNEL 5: Complex strided multi-dimensional access ========== */
__attribute__((noinline))
static void kernel5_strided_access(int arr4d[8][16][32][64]) {
    int i, j, k, l;
    
    /* 4-level nested loop with complex strided access */
    for (i = 1; i < 8; i++) {
        for (j = 2; j < 16; j += 2) {  /* Stride 2 */
            for (k = 1; k < 32; k += 3) {  /* Stride 3 */
                for (l = 4; l < 60; l += 4) {  /* Stride 4 */
                    /* Multi-dimensional flow dependency */
                    arr4d[i][j][k][l] = 
                        arr4d[i-1][j][k][l+1] +      /* Different i, different l */
                        arr4d[i][j-2][k+1][l-2] +    /* Different j, different k, different l */
                        arr4d[i][j][k-1][l];         /* Different k */
                    
                    /* Anti-dependency in inner loop */
                    int temp = arr4d[i][j][k][l-1];
                    arr4d[i][j][k][l] += temp;
                    arr4d[i][j][k][l-2] = temp * 2;  /* WAR */
                }
                
                /* Output dependency within k-loop */
                arr4d[i][j][k][10] = arr4d[i][j][k-1][10] + 1;
                arr4d[i][j][k][10] = arr4d[i][j][k][10] * 3;  /* WAW */
            }
        }
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int arr1[N][M];
    int arr2[N][M];
    double arr3[P];
    int base_arr[N * M];
    int res1[N], res2[N];
    int src1[N], src2[N];
    char cbuf[1024];
    int ibuf[1024];
    float fbuf[1024];
    double dbuf[1024];
    int arr4d[8][16][32][64];
    
    int i, j, k, l;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (i = 0; i < P; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    for (i = 0; i < N * M; i++) {
        base_arr[i] = lcg_rand() % 1000;
    }
    
    for (i = 0; i < N; i++) {
        src1[i] = lcg_rand() % 1000;
        src2[i] = lcg_rand() % 1000;
    }
    
    for (i = 0; i < 1024; i++) {
        cbuf[i] = (char)(lcg_rand() % 256);
        ibuf[i] = lcg_rand() % 1000;
        fbuf[i] = (float)(lcg_rand() % 1000) / 10.0f;
        dbuf[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 16; j++) {
            for (k = 0; k < 32; k++) {
                for (l = 0; l < 64; l++) {
                    arr4d[i][j][k][l] = lcg_rand() % 1000;
                }
            }
        }
    }
    
    /* Execute kernels with volatile operations between them */
    kernel1_flow_deps(arr1, arr2, arr3);
    
    volatile_sink = 1;
    asm volatile("" ::: "memory");
    
    kernel2_anti_deps(base_arr, N * M);
    
    volatile_sink = 2;
    asm volatile("" ::: "memory");
    
    kernel3_output_deps(res1, res2, src1, src2, N);
    
    volatile_sink = 3;
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(cbuf, ibuf, fbuf, dbuf, 1024);
    
    volatile_sink = 4;
    asm volatile("" ::: "memory");
    
    kernel5_strided_access(arr4d);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += (unsigned int)arr1[i][j];
            checksum += (unsigned int)arr2[i][j];
        }
    }
    
    for (i = 0; i < P; i++) {
        checksum += (unsigned long long)arr3[i];
    }
    
    for (i = 0; i < N * M; i++) {
        checksum += (unsigned int)base_arr[i];
    }
    
    for (i = 0; i < N; i++) {
        checksum += res1[i] + res2[i] + src1[i] + src2[i];
    }
    
    for (i = 0; i < 1024; i++) {
        checksum += (unsigned char)cbuf[i];
        checksum += ibuf[i];
        checksum += (unsigned int)fbuf[i];
        checksum += (unsigned long long)dbuf[i];
    }
    
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 16; j++) {
            for (k = 0; k < 32; k++) {
                for (l = 0; l < 64; l++) {
                    checksum += arr4d[i][j][k][l];
                }
            }
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}

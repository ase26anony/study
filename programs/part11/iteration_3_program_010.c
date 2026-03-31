/* test_ddg_coverage.c
 * Complex loop nests to trigger GCC's Data Dependency Graph edge creation
 * Specifically targets ddg.cc lines 749-757
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128
#define ITER 10

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile sink to prevent optimization */
static volatile int volatile_sink;

/* ========== KERNEL 1: Triple-nested loop with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M], double arr3[N]) {
    int i, j, k;
    
    /* Triple nested loop with flow dependencies across dimensions */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            /* Flow dependency (RAW) with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j] + arr2[i][j];
            
            /* Flow dependency with non-unit stride in j dimension */
            if (j % 3 == 0) {
                arr2[i][j] = arr1[i][j-2] * 2;
            } else {
                arr2[i][j] = arr1[i][j-1] + 1;
            }
            
            /* Additional dependency chain with varying distance */
            for (k = 0; k < P; k++) {
                /* Loop-carried dependency with distance 2 */
                if (k >= 2) {
                    arr1[i][j] += arr1[i][j] - arr2[i-1][j] + (k % 4);
                }
                /* Anti-dependency (WAR) */
                int temp = arr2[i][j];
                arr2[i][j] = temp + k;
                arr1[i][j] = arr2[i][j] * 3;
            }
        }
        
        /* Output dependency (WAW) with loop-carried distance */
        arr3[i] = (double)arr1[i][M/2];
        if (i >= 3) {
            arr3[i-2] = arr3[i] * 0.5;
        }
    }
    
    /* Cross-dimensional flow dependency */
    for (i = 2; i < N; i += 2) {
        for (j = 2; j < M; j += 2) {
            arr1[i][j] = arr1[i-2][j-1] + arr1[i-1][j-2];
        }
    }
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_aliasing_deps(int* base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* q aliases with p+1 */
    int *r = &base_arr[size/2];
    int *s = &base_arr[size/2 + 1];  /* s aliases with r+1 */
    
    int i;
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (i = 0; i < size - 10; i++) {
        int read_val = p[i];          /* Read from p[i] */
        q[i] = read_val * 2;          /* Write to q[i] which is p[i+1] */
        
        /* Creates WAR: read p[i+1] then write to it through q[i] */
        read_val = p[i+1];
        q[i] = read_val + i;
        
        /* Complex aliasing pattern */
        if (i % 4 == 0) {
            r[i/2] = s[i/2] * 3;      /* s[i/2] is r[i/2+1] */
            s[i/2] = r[i/2] / 2;
        }
    }
    
    /* Output dependencies (WAW) with pointer aliasing */
    int *alias1 = &base_arr[100];
    int *alias2 = &base_arr[101];
    
    for (i = 0; i < 50; i++) {
        /* Both pointers may alias the same location */
        alias1[i] = i * i;
        alias2[i-1] = alias1[i] + 5;  /* WAW if alias2[i-1] == alias1[i] */
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_restrict_deps(int* restrict r1, int* restrict r2, 
                                  int* restrict r3, int len) {
    int i;
    
    /* Output dependencies (WAW) within same restrict-qualified array */
    for (i = 2; i < len; i++) {
        r1[i] = r1[i-1] + r1[i-2];    /* Flow dependency */
        r1[i-1] = r1[i] * 2;          /* Output dependency on previous iteration */
    }
    
    /* Multiple restrict pointers with cross-dependencies */
    for (i = 0; i < len - 4; i++) {
        r2[i] = r3[i+1] + 1;          /* Flow between different restrict arrays */
        r3[i] = r2[i+2] * 3;          /* Anti-dependency */
        
        /* Varying dependency distances */
        if (i % 3 == 0 && i >= 3) {
            r1[i] = r1[i-3] + r2[i-1];
        } else if (i % 5 == 0 && i >= 5) {
            r1[i] = r1[i-5] * r3[i-2];
        }
    }
    
    /* Strided access pattern */
    for (i = 4; i < len; i += 2) {
        r2[i] = r2[i-4] + r3[i-2];
        r3[i-2] = r2[i] - 1;
    }
}

/* ========== KERNEL 4: Mixed data types and assembly barriers ========== */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, float* f_arr, double* d_arr, 
                                int* i_arr, int size) {
    int i;
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile int vol_var = 0;
    
    /* Mixed type dependency chain */
    for (i = 1; i < size; i++) {
        /* int -> float -> double -> char dependency chain */
        int int_val = i_arr[i-1] + 1;
        
        /* Memory barrier to enforce dependency */
        asm volatile("" ::: "memory");
        
        float float_val = (float)int_val * 1.5f;
        f_arr[i] = float_val + f_arr[i-1];
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        double double_val = (double)float_val * 2.0;
        d_arr[i] = double_val + d_arr[i-1];
        
        /* Type punning through union creates dependencies */
        u.i = int_val;
        c_arr[i] = u.c[0] + (char)float_val;
        
        /* Volatile operation prevents optimization */
        vol_var = int_val;
        volatile_sink = vol_var;
    }
    
    /* Bitwise operations with dependencies */
    for (i = 2; i < size; i++) {
        i_arr[i] = (i_arr[i-1] << 2) | (i_arr[i-2] & 0xFF);
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&c_arr[i-7], &c_arr[i-8], 8);
        }
    }
    
    /* memset with dependency on previous values */
    for (i = 0; i < size - 8; i += 8) {
        int pattern = i_arr[i] & 0xFF;
        memset(&c_arr[i], pattern, 8);
        
        /* Dependency on memset result */
        i_arr[i+4] = c_arr[i] + c_arr[i+1];
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Allocate multi-dimensional arrays with different data types */
    int (*arr1)[M] = malloc(N * sizeof(*arr1));
    int (*arr2)[M] = malloc(N * sizeof(*arr2));
    double *arr3 = malloc(N * sizeof(double));
    int *flat_arr = malloc(N * M * sizeof(int));
    char *char_arr = malloc(N * sizeof(char));
    float *float_arr = malloc(N * sizeof(float));
    double *double_arr = malloc(N * sizeof(double));
    int *int_arr = malloc(N * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !flat_arr || 
        !char_arr || !float_arr || !double_arr || !int_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
        }
        arr3[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < N * M; i++) {
        flat_arr[i] = lcg_rand() % 1000;
    }
    
    for (int i = 0; i < N; i++) {
        char_arr[i] = lcg_rand() % 256;
        float_arr[i] = (float)(lcg_rand() % 1000) / 10.0f;
        double_arr[i] = (double)(lcg_rand() % 1000) / 10.0;
        int_arr[i] = lcg_rand() % 1000;
    }
    
    /* Execute kernels multiple times to ensure DDG construction */
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify array contents with volatile operations between kernels */
        volatile_sink = iter;
        
        /* Kernel 1: Complex nested loops with flow dependencies */
        kernel1_flow_deps(arr1, arr2, arr3);
        
        /* Volatile operation to prevent cross-kernel optimization */
        asm volatile("" ::: "memory");
        
        /* Kernel 2: Pointer aliasing dependencies */
        kernel2_aliasing_deps(flat_arr, N * M);
        
        /* Volatile operation */
        volatile_sink = flat_arr[0];
        
        /* Kernel 3: Restrict pointers */
        kernel3_restrict_deps(&flat_arr[0], &flat_arr[N*M/4], 
                             &flat_arr[N*M/2], N*M/4);
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Kernel 4: Mixed data types */
        kernel4_mixed_types(char_arr, float_arr, double_arr, int_arr, N);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (unsigned)arr1[i][j];
            checksum += (unsigned)arr2[i][j];
        }
        checksum += (unsigned long long)arr3[i];
    }
    
    for (int i = 0; i < N * M; i++) {
        checksum += (unsigned)flat_arr[i];
    }
    
    for (int i = 0; i < N; i++) {
        checksum += (unsigned char)char_arr[i];
        checksum += (unsigned)float_arr[i];
        checksum += (unsigned long long)double_arr[i];
        checksum += (unsigned)int_arr[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(flat_arr);
    free(char_arr);
    free(float_arr);
    free(double_arr);
    free(int_arr);
    
    return 0;
}

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

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0;
    
    /* Flow dependencies (RAW) across i, j, k dimensions */
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j-1];
                arr2[i][j] = arr1[i][j-1] * 2 - arr2[i-2][j];
                
                /* Loop-carried dependency with varying distance */
                if (i % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[i-1][j+1];
                } else if (i % 5 == 0) {
                    arr1[i][j] = arr1[i-3][j] * arr2[i-2][j-1];
                } else {
                    arr1[i][j] = arr1[i-1][j] + arr2[i][j];
                }
            }
        }
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        barrier++;
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    volatile int* vp = &base_arr[size/4];
    
    /* Anti-dependencies through aliasing pointers */
    for (int i = 2; i < size - 2; i++) {
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (aliases p+1) - WAR */
        
        /* More complex aliasing pattern */
        if (i % 4 == 0) {
            *r = *p + *q;       /* Read from p and q, write to r */
            p = &base_arr[i];   /* Change pointer */
        } else if (i % 7 == 0) {
            q = &base_arr[i-1]; /* Change aliasing relationship */
            *p = *q * 3;        /* WAR dependency */
        }
        
        /* Volatile access creates additional dependencies */
        *vp = i;
        asm volatile("" ::: "memory");
        
        /* Output dependency (WAW) */
        base_arr[i] = base_arr[i-1] + 1;
        base_arr[i] = base_arr[i-2] * 2;  /* WAW on base_arr[i] */
    }
}

/* Kernel 3: Restrict pointers with output dependencies */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict d1, double* restrict d2, 
                                double* restrict d3, int len) {
    /* Output dependencies (WAW) with restrict qualification */
    for (int i = 2; i < len - 2; i++) {
        /* Chain of output dependencies */
        d1[i] = d2[i-1] + d3[i+1];
        d1[i] = d1[i] * 1.5;           /* WAW on d1[i] */
        
        d2[i] = d1[i-2] * 0.5;
        d2[i] = d2[i] + d3[i-1];       /* WAW on d2[i] */
        
        /* Loop-carried output dependency with distance 3 */
        if (i % 3 == 0) {
            d3[i] = d1[i] + d2[i];
            d3[i] = d3[i-3] * 2.0;     /* WAW with distance 3 */
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 4: Mixed data types and operations */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, int* i_arr, float* f_arr, 
                                double* d_arr, int size) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile union mixed_union vu;
    
    /* Mixed type dependency chain */
    for (int i = 1; i < size - 1; i++) {
        /* Type casting creates data type conversions in DDG */
        float f_temp = (float)i_arr[i-1] * 1.5f;
        i_arr[i] = (int)f_temp + (int)c_arr[i] * 2;
        
        /* Bitwise operations */
        i_arr[i] = (i_arr[i] << 2) | (i_arr[i-1] >> 1);
        
        /* Mixed type access through union */
        u.i = i_arr[i];
        f_arr[i] = u.f * 0.5f;
        u.f = f_arr[i-1];
        c_arr[i] = u.c[0] + 1;
        
        /* Double precision operations */
        d_arr[i] = (double)i_arr[i] / 3.14159;
        d_arr[i] = d_arr[i-1] * d_arr[i];
        
        /* Memory function calls create dependencies */
        if (i % 8 == 0) {
            memcpy(&c_arr[i-4], &c_arr[i], 4);
            memset(&i_arr[i-2], 0, sizeof(int) * 2);
        }
        
        /* Volatile union access */
        vu.i = i;
        asm volatile("" ::: "memory");
    }
}

/* Complex loop nest with all dependency types */
__attribute__((noinline))
static void kernel5_complex_nest(int arr3d[N][M][8]) {
    /* Triple nested loop with complex indexing */
    for (int i = 2; i < N-2; i++) {
        for (int j = 2; j < M-2; j++) {
            for (int k = 1; k < 7; k++) {
                /* Flow dependency across all dimensions */
                arr3d[i][j][k] = arr3d[i-1][j+1][k-1] + arr3d[i][j-1][k+1];
                
                /* Anti-dependency with pointer-like access */
                int temp = arr3d[i][j][k];
                arr3d[i-1][j][k] = temp * 2;
                
                /* Output dependency */
                arr3d[i][j][k] = arr3d[i][j][k] + 1;
                arr3d[i][j][k] = arr3d[i][j][k] * 3;
                
                /* Dependency with varying distance based on conditions */
                if ((i + j) % 4 == 0) {
                    arr3d[i][j][k] = arr3d[i-2][j][k] + arr3d[i][j-2][k];
                } else if ((i + j) % 4 == 1) {
                    arr3d[i][j][k] = arr3d[i-3][j+1][k] - arr3d[i+1][j-3][k];
                }
            }
            
            /* Inline assembly barrier every inner loop iteration */
            asm volatile("" ::: "memory");
        }
        
        /* Volatile operation every outer loop iteration */
        volatile int sync = i;
        (void)sync;
    }
}

int main(void) {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    int arr3d[N][M][8];
    int linear_arr[N * M * 2];
    double dbl_arr[1024];
    float flt_arr[512];
    char char_arr[1024];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
            for (int k = 0; k < 8; k++) {
                arr3d[i][j][k] = lcg_rand() % 1000;
            }
        }
    }
    
    for (int i = 0; i < N * M * 2; i++) {
        linear_arr[i] = lcg_rand() % 1000;
    }
    
    for (int i = 0; i < 1024; i++) {
        dbl_arr[i] = (lcg_rand() % 1000) / 10.0;
        if (i < 512) flt_arr[i] = (lcg_rand() % 1000) / 10.0f;
        char_arr[i] = lcg_rand() % 256;
    }
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_deps(arr1, arr2);
    
    /* Modify array contents between kernels */
    volatile int modify = 0;
    for (int i = 0; i < 100; i++) {
        arr1[i%N][i%M] ^= 0x55;
        modify++;
    }
    asm volatile("" ::: "memory");
    
    kernel2_anti_deps(linear_arr, N * M * 2);
    
    /* More modifications */
    for (int i = 0; i < 50; i++) {
        dbl_arr[i] += 1.0;
    }
    asm volatile("" ::: "memory");
    
    kernel3_output_deps(dbl_arr, &dbl_arr[256], &dbl_arr[512], 768);
    
    kernel4_mixed_types(char_arr, linear_arr, flt_arr, dbl_arr, 512);
    
    /* Final complex kernel */
    kernel5_complex_nest(arr3d);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
            for (int k = 0; k < 8; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    for (int i = 0; i < N * M * 2; i++) {
        checksum += linear_arr[i];
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (unsigned long long)dbl_arr[i];
        if (i < 512) checksum += (unsigned long long)flt_arr[i];
        checksum += char_arr[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}

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

/* Kernel 1: Triple-nested loop with flow dependencies */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0;
    
    /* Flow dependencies across all dimensions */
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* RAW: Read after write with varying distances */
                arr1[i][j] = arr1[i-1][j+1] + arr1[i][j-1] * 2;
                
                /* Cross-array dependency */
                arr2[i][j] = arr1[i][j] + arr2[i-2][j+2] - arr2[i][j-3];
                
                /* Loop-carried dependency with distance 2 */
                if (k % 3 == 0) {
                    arr1[i][j] = arr1[i][j] + arr1[i-2][j] + 1;
                } else {
                    arr1[i][j] = arr1[i][j] + arr1[i-1][j] * 2;
                }
            }
        }
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        barrier++;
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[2];
    volatile int* vp = &base_arr[size/2];
    
    /* WAR (Write after Read) anti-dependencies */
    for (int i = 3; i < size - 3; i++) {
        int temp = *p + *q;      /* Read from p and q */
        *p = temp * 2;           /* Write to p - anti-dep with next iteration */
        *q = *r + i;             /* Write to q */
        
        /* Pointer arithmetic creates overlapping accesses */
        p = &base_arr[i];
        q = &base_arr[i+1];
        r = &base_arr[i-1];
        
        /* Volatile access creates memory barrier */
        *vp = *vp + 1;
        
        /* Output dependency (WAW) */
        base_arr[i] = base_arr[i-2] + base_arr[i-1];
        base_arr[i+1] = base_arr[i] * 3;
    }
}

/* Kernel 3: Restrict pointers with output dependencies */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict dbl_arr1, 
                                 double* restrict dbl_arr2,
                                 int len) {
    /* restrict allows better optimization but still creates dependencies */
    for (int i = 2; i < len - 2; i++) {
        /* WAW: Output dependencies */
        dbl_arr1[i] = dbl_arr2[i-1] + dbl_arr2[i-2];
        dbl_arr1[i] = dbl_arr1[i] * 1.5;  /* Overwrite - output dep */
        
        /* Flow dependency chain */
        dbl_arr2[i] = dbl_arr1[i-1] + dbl_arr1[i-2];
        dbl_arr2[i+1] = dbl_arr2[i] * 0.75;
        
        /* Conditional dependency distance */
        if (i % 4 == 0) {
            dbl_arr1[i] = dbl_arr1[i-3] + 2.0;
        } else if (i % 4 == 1) {
            dbl_arr1[i] = dbl_arr1[i-2] * 1.1;
        } else {
            dbl_arr1[i] = dbl_arr1[i-1] - 0.5;
        }
    }
    
    /* Memory clobber to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Kernel 4: Mixed data types and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(char* char_arr, float* float_arr, 
                                 int* int_arr, int size) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile float vf = 0.0f;
    
    for (int i = 4; i < size - 4; i++) {
        /* Type casting creates data type dependencies */
        u.i = int_arr[i-1];
        float_arr[i] = u.f + float_arr[i-2];
        
        /* Bitwise operations with dependencies */
        int_arr[i] = (int_arr[i-1] << 2) | (int_arr[i-2] >> 1);
        int_arr[i] = int_arr[i] ^ int_arr[i-3];
        
        /* Char array with memcpy dependency */
        char temp[4];
        memcpy(temp, &char_arr[i-1], 4);
        char_arr[i] = temp[0] + temp[1] - temp[2];
        
        /* Inline assembly barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
            vf = float_arr[i] + 1.0f;
        }
        
        /* Mixed type dependency chain */
        u.f = float_arr[i];
        char_arr[i+1] = u.c[0] + char_arr[i-1];
        int_arr[i+1] = (int)u.f + int_arr[i];
    }
}

/* Complex strided access pattern */
__attribute__((noinline))
static void kernel5_strided_access(int arr3d[32][32][32]) {
    /* Non-unit stride in all dimensions */
    for (int i = 2; i < 30; i += 2) {
        for (int j = 3; j < 29; j += 3) {
            for (int k = 4; k < 28; k += 4) {
                /* Complex strided dependency pattern */
                arr3d[i][j][k] = arr3d[i-2][j+3][k-4] 
                               + arr3d[i][j-3][k+4] 
                               - arr3d[i+2][j][k-2];
                
                /* Cross-iteration dependency with varying distance */
                int dep_dist = (i * j) % 5 + 1;
                arr3d[i][j][k] += arr3d[i-dep_dist][j][k] * 2;
                
                /* Anti-dependency with stride */
                int temp = arr3d[i][j][k-1];
                arr3d[i][j][k] = temp + arr3d[i][j][k+1];
                arr3d[i][j][k-1] = temp * 3;
            }
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    int linear_arr[N*M];
    double dbl_arr1[1024];
    double dbl_arr2[1024];
    char char_arr[2048];
    float float_arr[2048];
    int int_arr[2048];
    int arr3d[32][32][32];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
            linear_arr[i*M + j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        dbl_arr1[i] = (lcg_rand() % 1000) / 10.0;
        dbl_arr2[i] = (lcg_rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 2048; i++) {
        char_arr[i] = lcg_rand() % 256;
        float_arr[i] = (lcg_rand() % 1000) / 10.0f;
        int_arr[i] = lcg_rand() % 1000;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                arr3d[i][j][k] = lcg_rand() % 1000;
            }
        }
    }
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_deps(arr1, arr2);
    
    /* Volatile modification between kernels */
    volatile int inter_kernel_barrier = 0;
    inter_kernel_barrier++;
    
    kernel2_anti_deps(linear_arr, N*M);
    
    /* Another barrier */
    asm volatile("" ::: "memory");
    
    kernel3_output_deps(dbl_arr1, dbl_arr2, 1024);
    
    inter_kernel_barrier++;
    
    kernel4_mixed_types(char_arr, float_arr, int_arr, 2048);
    
    /* Final barrier */
    asm volatile("" ::: "memory");
    
    kernel5_strided_access(arr3d);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < N*M; i++) {
        checksum += linear_arr[i];
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (unsigned long long)dbl_arr1[i];
        checksum += (unsigned long long)dbl_arr2[i];
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += char_arr[i] + (unsigned long long)float_arr[i] + int_arr[i];
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}

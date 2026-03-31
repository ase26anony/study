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
    volatile int sink = 0;
    
    /* True dependencies (RAW) across all dimensions */
    for (int i = 2; i < N - 2; i++) {
        for (int j = 2; j < M - 2; j++) {
            for (int k = 1; k < P; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr1[i-2][j-1];
                arr2[i][j] = arr1[i][j-1] * 2 - arr2[i-1][j];
                
                /* Cross-array dependencies */
                arr1[i][j] += arr2[i-1][j+1] / 3;
            }
        }
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(sink) : "memory");
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    /* Create potentially aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    
    volatile int barrier = 0;
    
    /* Anti-dependencies through aliased pointers */
    for (int i = 2; i < size - 2; i++) {
        int temp = p[i];           /* Read */
        p[i] = q[i-1] * 3;         /* Write - creates WAR with next iteration */
        q[i-1] = temp + r[i-2];    /* Write to different alias */
        
        /* Conditional dependency distances */
        if (i % 4 == 0) {
            r[i] = p[i-3] + q[i-1];
        } else if (i % 3 == 0) {
            r[i] = p[i-2] * 2;
        } else {
            r[i] = p[i-1] - 1;
        }
        
        /* Memory barrier to preserve dependencies */
        if (i % 32 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    asm volatile("" : : "r"(barrier) : "memory");
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict d1, 
                                 double* restrict d2, 
                                 double* restrict d3, 
                                 int len) {
    /* Output dependencies with restrict qualification */
    for (int i = 3; i < len - 3; i++) {
        /* WAW dependencies */
        d1[i] = d2[i-1] + d3[i-2];
        d1[i] = d1[i] * 1.5;           /* WAW on d1[i] */
        
        /* Chain of dependencies with different distances */
        d2[i] = d1[i-1] + d1[i-2];
        d3[i] = d2[i-1] * d2[i-3];
        
        /* Mixed distance dependencies */
        if (i % 5 == 0) {
            d1[i] = d3[i-4] / 2.0;
        }
    }
    
    /* Volatile write to prevent reordering */
    volatile double v = d1[len-1];
    asm volatile("" : : "r"(v) : "memory");
}

/* Kernel 4: Mixed data types and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, float* f_arr, 
                                 int* i_arr, int size) {
    union mixed_data {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile union mixed_data vu;
    
    /* Dependencies across different data types */
    for (int i = 4; i < size - 4; i++) {
        /* Type casting creating dependencies */
        u.i = i_arr[i-1];
        f_arr[i] = (float)u.i * 0.5f;
        
        /* Bitwise operations with dependencies */
        i_arr[i] = (i_arr[i-2] & 0xFF00) | (i_arr[i-1] & 0x00FF);
        
        /* Char array with pointer arithmetic */
        c_arr[i] = (char)(f_arr[i-1] * 100.0f);
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&c_arr[i], &c_arr[i-4], 4);
        }
        
        /* Inline assembly barrier at strategic points */
        if (i % 64 == 0) {
            asm volatile("" ::: "memory");
            vu.i = i_arr[i];
        }
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
}

/* Complex loop with all dependency types */
__attribute__((noinline))
static void kernel5_complex_deps(int mat[N][M], float fmat[N][M]) {
    int* row_ptrs[N];
    float* frow_ptrs[N];
    
    /* Setup row pointers (potential aliasing) */
    for (int i = 0; i < N; i++) {
        row_ptrs[i] = &mat[i][0];
        frow_ptrs[i] = &fmat[i][0];
    }
    
    /* Complex nested loops with mixed dependencies */
    for (int i = 3; i < N - 3; i++) {
        int* p1 = row_ptrs[i];
        int* p2 = row_ptrs[i-1];
        int* p3 = row_ptrs[i-2];
        
        float* fp1 = frow_ptrs[i];
        float* fp2 = frow_ptrs[i-1];
        
        for (int j = 3; j < M - 3; j++) {
            /* Flow dependency (RAW) */
            int temp = p2[j+1] + p3[j-1];
            
            /* Anti-dependency (WAR) */
            p1[j] = temp * 2;
            
            /* Output dependency (WAW) with type conversion */
            fp1[j] = (float)p1[j] / 3.0f;
            fp1[j] = fp1[j] + fp2[j-1];  /* WAW on fp1[j] */
            
            /* Loop-carried dependency with varying distance */
            if (j % 7 == 0) {
                p1[j] = p1[j-6] + p2[j-3];
            } else if (j % 5 == 0) {
                p1[j] = p1[j-4] * p2[j-2];
            }
            
            /* Cross-type dependency */
            p3[j+1] = (int)(fp1[j] * 10.0f);
        }
        
        /* Memory barrier between outer loop iterations */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    double darr1[1024];
    double darr2[1024];
    double darr3[1024];
    char carr[2048];
    float farr[2048];
    int iarr[2048];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)(lcg_rand() % 1000);
            arr2[i][j] = (int)(lcg_rand() % 1000);
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        darr1[i] = (double)(lcg_rand() % 1000) / 3.0;
        darr2[i] = (double)(lcg_rand() % 1000) / 5.0;
        darr3[i] = (double)(lcg_rand() % 1000) / 7.0;
    }
    
    for (int i = 0; i < 2048; i++) {
        carr[i] = (char)(lcg_rand() % 256);
        farr[i] = (float)(lcg_rand() % 1000) / 11.0f;
        iarr[i] = (int)(lcg_rand() % 10000);
    }
    
    /* Execute kernels with volatile operations between them */
    volatile int inter_kernel_barrier = 0;
    
    kernel1_flow_deps(arr1, arr2);
    inter_kernel_barrier = arr1[0][0];
    
    kernel2_anti_deps(&arr1[0][0], N * M);
    inter_kernel_barrier = arr2[N-1][M-1];
    
    kernel3_output_deps(darr1, darr2, darr3, 1024);
    inter_kernel_barrier = (int)darr1[0];
    
    kernel4_mixed_types(carr, farr, iarr, 2048);
    inter_kernel_barrier = carr[0];
    
    kernel5_complex_deps(arr1, (float(*)[M])farr);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (unsigned int)arr1[i][j];
            checksum += (unsigned int)arr2[i][j];
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (unsigned long long)darr1[i];
        checksum += (unsigned long long)darr2[i];
        checksum += (unsigned long long)darr3[i];
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += (unsigned char)carr[i];
        checksum += (unsigned int)farr[i];
        checksum += (unsigned int)iarr[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}

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
    
    /* Flow dependencies (RAW) across i, j, and k dimensions */
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* Multi-dimensional strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j-1];
                arr2[i][j] = arr1[i][j-2] * arr2[i-1][j+1];
                
                /* Additional dependency with varying distance */
                if (i % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[i][j];
                } else if (i % 5 == 0) {
                    arr1[i][j] = arr1[i-3][j+1] * 2;
                }
                
                /* Memory barrier to prevent optimization */
                asm volatile("" ::: "memory");
                barrier = arr1[i][j];
            }
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* arr, int size) {
    /* Create aliasing pointers */
    int* p = &arr[0];
    int* q = &arr[1];
    int* r = &arr[2];
    
    volatile int barrier = 0;
    
    for (int i = 10; i < size - 10; i++) {
        /* Anti-dependencies (WAR) through aliasing pointers */
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (aliases p+1) */
        temp = *q;              /* Read from q */
        *r = temp * 2;          /* Write to r (aliases p+2) */
        temp = *r;              /* Read from r */
        *p = temp - 1;          /* Write to p (completes WAR chain) */
        
        /* Pointer arithmetic that may alias */
        p = &arr[i % size];
        q = &arr[(i + 1) % size];
        r = &arr[(i + 2) % size];
        
        /* Conditional with varying dependency distances */
        if (i % 7 == 0) {
            arr[i] = arr[i-4] + arr[i-3];
        } else if (i % 11 == 0) {
            arr[i] = arr[i-6] * arr[i-5];
        }
        
        barrier = *p;
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict d1, double* restrict d2, 
                                double* restrict d3, int size) {
    volatile double barrier = 0.0;
    
    /* Output dependencies (WAW) with restrict pointers */
    for (int i = 2; i < size - 2; i++) {
        /* Multiple writes to same location through restrict */
        d1[i] = d2[i-1] + d3[i+1];
        d1[i] = d1[i] * 1.5;           /* WAW on d1[i] */
        d2[i] = d1[i-2] * d3[i];
        d2[i] = d2[i] + 0.5;           /* WAW on d2[i] */
        
        /* Loop-carried dependency with varying distance */
        if (i % 4 == 0) {
            d3[i] = d3[i-2] * 0.8;
        } else {
            d3[i] = d3[i-1] * 1.2;
        }
        
        /* Mixed operations causing WAW */
        d1[i] = d2[i] + d3[i];
        d1[i] = d1[i-1] * d1[i];       /* Another WAW */
        
        barrier = d1[i];
        asm volatile("" ::: "memory");
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, int* i_arr, float* f_arr, 
                                double* d_arr, int size) {
    volatile int barrier_int = 0;
    volatile float barrier_float = 0.0f;
    
    /* Union for type punning */
    union {
        int i;
        float f;
        char bytes[4];
    } converter;
    
    for (int i = 4; i < size - 4; i++) {
        /* Mixed type dependencies with casting */
        int int_val = i_arr[i-1];
        float float_val = f_arr[i-2];
        double double_val = d_arr[i-3];
        
        /* Type conversions creating dependencies */
        f_arr[i] = (float)int_val * 1.1f;
        i_arr[i] = (int)(float_val * 2.0f);
        d_arr[i] = (double)i_arr[i] / 3.0;
        
        /* Bitwise operations */
        i_arr[i] = i_arr[i] ^ i_arr[i-1];
        i_arr[i] = i_arr[i] | 0xFF00FF00;
        
        /* Memory operations creating dependencies */
        memcpy(&c_arr[i], &i_arr[i], 1);
        converter.i = i_arr[i];
        f_arr[i] = converter.f;
        
        /* Inline assembly barriers at strategic points */
        asm volatile("" ::: "memory");
        
        /* Volatile accesses */
        barrier_int = i_arr[i];
        barrier_float = f_arr[i];
        
        /* Complex dependency chain with mixed types */
        if (i % 8 == 0) {
            d_arr[i] = (double)f_arr[i-4] * (double)i_arr[i-2];
            i_arr[i] = (int)d_arr[i];
        }
        
        asm volatile("" ::: "memory");
    }
}

/* Additional kernel with complex loop nests and dependencies */
__attribute__((noinline))
static void kernel5_complex_nest(int arr3d[32][32][32]) {
    volatile int barrier = 0;
    
    /* 4-level nested loop with various dependencies */
    for (int i = 1; i < 30; i++) {
        for (int j = 1; j < 30; j++) {
            for (int k = 1; k < 30; k++) {
                for (int l = 1; l < 30; l++) {
                    /* Complex indexing with multiple dependencies */
                    arr3d[i][j][k] = arr3d[i-1][j+1][k-1] + 
                                     arr3d[i][j-1][k+1] * 
                                     arr3d[i+1][j][k-2];
                    
                    /* Conditional dependencies with varying distances */
                    switch ((i + j + k + l) % 5) {
                        case 0:
                            arr3d[i][j][k] = arr3d[i-2][j][k] + l;
                            break;
                        case 1:
                            arr3d[i][j][k] = arr3d[i][j-3][k] * 2;
                            break;
                        case 2:
                            arr3d[i][j][k] = arr3d[i][j][k-4] - l;
                            break;
                        default:
                            arr3d[i][j][k] = arr3d[i-1][j-1][k-1] / 3;
                            break;
                    }
                    
                    barrier = arr3d[i][j][k];
                }
                asm volatile("" ::: "memory");
            }
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    int linear_arr[N * M];
    double dbl_arr[1024];
    float flt_arr[1024];
    char char_arr[1024];
    int arr3d[32][32][32];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
            linear_arr[i * M + j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        dbl_arr[i] = (double)(lcg_rand() % 1000) / 3.0;
        flt_arr[i] = (float)(lcg_rand() % 1000) / 2.0f;
        char_arr[i] = (char)(lcg_rand() % 256);
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                arr3d[i][j][k] = lcg_rand() % 1000;
            }
        }
    }
    
    volatile int inter_kernel_barrier = 0;
    
    /* Execute kernels with volatile barriers between them */
    kernel1_flow_deps(arr1, arr2);
    inter_kernel_barrier = arr1[0][0];
    asm volatile("" ::: "memory");
    
    kernel2_anti_deps(linear_arr, N * M);
    inter_kernel_barrier = linear_arr[0];
    asm volatile("" ::: "memory");
    
    kernel3_output_deps(dbl_arr, dbl_arr + 512, dbl_arr + 256, 512);
    inter_kernel_barrier = (int)dbl_arr[0];
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(char_arr, linear_arr, flt_arr, dbl_arr, 512);
    inter_kernel_barrier = linear_arr[0];
    asm volatile("" ::: "memory");
    
    kernel5_complex_nest(arr3d);
    inter_kernel_barrier = arr3d[0][0][0];
    asm volatile("" ::: "memory");
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < N * M; i++) {
        checksum += linear_arr[i];
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (unsigned long long)dbl_arr[i];
        checksum += (unsigned long long)flt_arr[i];
        checksum += char_arr[i];
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

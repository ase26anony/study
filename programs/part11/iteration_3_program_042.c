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
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    volatile int sink = 0; /* Prevent optimization */
    
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* Complex flow dependencies with varying distances */
                arr1[i][j] = arr1[i-1][j+1] + arr1[i][j-1] * 2;
                arr2[i][j] = arr1[i][j] + arr2[i-2][j] - arr2[i][j-3];
                
                /* Conditional dependency with different distances */
                if ((i + j) % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[i][j-1];
                } else if ((i + j) % 5 == 0) {
                    arr1[i][j] = arr1[i-1][j+2] * arr2[i-3][j];
                } else {
                    arr1[i][j] = arr1[i][j-1] + arr2[i-1][j];
                }
                
                /* Anti-dependency (WAR) */
                int temp = arr2[i][j];
                arr2[i][j] = arr1[i][j] * 3;
                arr1[i][j] = temp + arr1[i][j];
                
                /* Output dependency (WAW) */
                arr1[i][j] = arr1[i][j] * 2;
                arr1[i][j] = arr1[i][j] + 1;
            }
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies */
__attribute__((noinline))
static void kernel2_pointer_aliasing(int* base_arr, int size) {
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    int* s = &base_arr[size/2 + 1];
    
    volatile int accumulator = 0;
    
    for (int i = 1; i < size - 10; i++) {
        /* Aliasing pointers creating complex dependencies */
        *p = *q + *r;
        *q = *p - *s;
        
        /* Pointer arithmetic with potential aliasing */
        *(p + i) = *(q + i - 1) + *(r + i - 2);
        *(q + i) = *(p + i) * *(s + i - 3);
        
        /* Anti-dependencies through aliasing */
        int temp = *p;
        *p = *q * 2;
        *q = temp + 1;
        
        /* Move pointers - creates varying access patterns */
        if (i % 7 == 0) {
            p = &base_arr[i];
            q = &base_arr[i + 1];
        } else if (i % 11 == 0) {
            r = &base_arr[i * 2];
            s = &base_arr[i * 2 + 1];
        }
        
        accumulator += *p + *q;
    }
    
    /* Prevent dead code elimination */
    asm volatile("" ::: "memory");
}

/* Kernel 3: Loop with restrict pointers and output dependencies */
__attribute__((noinline))
static void kernel3_restrict_pointers(int* restrict r1, int* restrict r2, 
                                      int* restrict r3, int size) {
    /* restrict qualifier allows more aggressive optimization */
    for (int i = 2; i < size - 2; i++) {
        /* Output dependencies (WAW) */
        r1[i] = r2[i-1] + r3[i-2];
        r1[i] = r1[i] * r1[i-1];  /* WAW on r1[i] */
        
        /* Flow dependencies with restrict */
        r2[i] = r1[i] + r2[i-1];
        r3[i] = r2[i] * r3[i-2];
        
        /* Complex dependency chain */
        if (i % 4 == 0) {
            r1[i] = r1[i-2] + r2[i-1] * r3[i-3];
        } else {
            r1[i] = r1[i-1] + r2[i-2] - r3[i-4];
        }
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* Kernel 4: Mixed data type dependencies and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(double darr[], float farr[], 
                                char carr[], int iarr[], int size) {
    union mixed_data {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile double v_d = 0.0;
    volatile float v_f = 0.0f;
    
    for (int i = 4; i < size - 4; i++) {
        /* Mixed type dependencies with casting */
        darr[i] = (double)farr[i-1] + darr[i-2] * 1.5;
        farr[i] = (float)darr[i] * 0.5f + farr[i-3];
        
        /* Integer and char dependencies */
        iarr[i] = (int)darr[i] + iarr[i-1] * 2;
        carr[i] = (char)(iarr[i] & 0xFF) + carr[i-2];
        
        /* Union creating type-punning dependencies */
        u.i = iarr[i-1];
        farr[i] = u.f * 2.0f;
        u.f = farr[i];
        iarr[i] = u.i + iarr[i-2];
        
        /* Inline assembly creating artificial dependencies */
        asm volatile("" : "+r" (iarr[i]), "+r" (iarr[i-1]) : : "memory");
        
        /* Volatile operations */
        v_d = darr[i];
        darr[i] = v_d + 1.0;
        v_f = farr[i];
        farr[i] = v_f * 1.1f;
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&carr[i], &carr[i-4], 4);
            memset(&iarr[i], iarr[i-1], sizeof(int));
        }
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
}

/* Kernel 5: Complex strided access patterns */
__attribute__((noinline))
static void kernel5_strided_access(int arr3d[N][M][8]) {
    volatile int counter = 0;
    
    for (int i = 2; i < N-2; i += 2) {  /* Non-unit stride */
        for (int j = 3; j < M-3; j += 3) {  /* Different stride */
            for (int k = 1; k < 7; k++) {
                /* Multi-dimensional strided access with dependencies */
                arr3d[i][j][k] = arr3d[i-2][j+1][k-1] + 
                                 arr3d[i][j-3][k] * 2 - 
                                 arr3d[i-1][j][k+1];
                
                /* Conditional striding */
                if (k % 2 == 0) {
                    arr3d[i][j][k] = arr3d[i][j][k] + arr3d[i-1][j+2][k-2];
                } else {
                    arr3d[i][j][k] = arr3d[i][j][k] - arr3d[i+1][j-2][k+1];
                }
                
                /* Reverse access pattern */
                arr3d[N-i-1][M-j-1][7-k] = arr3d[i][j][k] + 
                                          arr3d[N-i][M-j][7-k+1];
                
                counter += arr3d[i][j][k];
            }
        }
    }
    
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    int linear_arr[N * M * 2];
    double darr[1024];
    float farr[1024];
    char carr[1024];
    int iarr[1024];
    int arr3d[N][M][8];
    
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
        darr[i] = (double)(lcg_rand() % 1000) / 10.0;
        farr[i] = (float)(lcg_rand() % 1000) / 10.0f;
        carr[i] = (char)(lcg_rand() % 256);
        iarr[i] = lcg_rand() % 1000;
    }
    
    /* Execute kernels with volatile barriers between them */
    volatile int barrier = 0;
    
    kernel1_flow_dependencies(arr1, arr2);
    barrier = arr1[0][0];  /* Prevent cross-kernel optimization */
    
    kernel2_pointer_aliasing(linear_arr, N * M * 2);
    barrier = linear_arr[0];
    
    kernel3_restrict_pointers(&linear_arr[0], &linear_arr[100], 
                              &linear_arr[200], 500);
    barrier = linear_arr[100];
    
    kernel4_mixed_types(darr, farr, carr, iarr, 1024);
    barrier = (int)darr[0];
    
    kernel5_strided_access(arr3d);
    barrier = arr3d[0][0][0];
    
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
        checksum += (unsigned long long)darr[i] + 
                   (unsigned long long)farr[i] + 
                   carr[i] + iarr[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}

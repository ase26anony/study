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

/* Volatile variables to prevent optimization */
volatile int volatile_barrier = 0;

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* Flow dependencies (RAW) across i dimension */
    for (i = 1; i < N - 1; i++) {
        /* Flow dependencies across j dimension with stride */
        for (j = 2; j < M - 2; j += 2) {
            /* Innermost loop with k dimension dependencies */
            for (k = 1; k < P; k++) {
                /* Complex dependency chain with varying distances */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j-1] * k;
                
                /* Additional flow dependency with distance 3 */
                if (k % 4 == 0) {
                    arr2[i][j] = arr1[i][j] + arr2[i-3][j] / (k + 1);
                } else {
                    arr2[i][j] = arr1[i][j] - arr2[i-1][j+1] * (k % 3);
                }
                
                /* Memory barrier to ensure dependencies are visible */
                asm volatile("" ::: "memory");
            }
            
            /* Output dependency (WAW) within same iteration */
            arr1[i][j] = arr1[i][j] + (i * j) % 7;
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies */
__attribute__((noinline))
void kernel2_pointer_aliasing(int* base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* q aliases with p+1 */
    int *r = &base_arr[size/2]; /* Potential aliasing with p/q */
    
    int i;
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (i = 1; i < size - 10; i++) {
        int temp = p[i];      /* Read from p[i] */
        q[i-1] = temp * 2;    /* Write to what might be p[i-1] or p[i] */
        
        /* Complex aliasing pattern */
        if (i % 5 == 0) {
            r[i/2] = p[i] + q[i-2];  /* r may alias with p or q */
        }
        
        /* Flow dependency with pointer arithmetic */
        p[i+1] = q[i] + r[(i*3) % (size/2)];
        
        /* Varying dependency distances */
        if (i % 3 == 0) {
            base_arr[i] = base_arr[i-2] + 1;  /* Distance 2 */
        } else {
            base_arr[i] = base_arr[i-1] * 2;  /* Distance 1 */
        }
        
        /* Volatile read to create artificial dependency */
        volatile_barrier = base_arr[i % 16];
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies */
__attribute__((noinline))
void kernel3_restrict_pointers(double* restrict arr1, double* restrict arr2, 
                               double* restrict arr3, int n) {
    int i, j;
    
    /* Output dependencies (WAW) with restrict qualification */
    for (i = 2; i < n - 2; i++) {
        /* Multiple writes to same location creating WAW */
        arr1[i] = (double)i * 1.5;
        arr1[i] = arr1[i] + arr2[i-1] * 0.5;  /* WAW dependency */
        
        /* Flow dependency chain with restrict */
        arr2[i] = arr1[i-1] + arr3[i+1];
        arr3[i] = arr2[i] * arr1[i-2];
        
        /* Nested inner loop with dependencies */
        for (j = 0; j < 8; j++) {
            /* Complex index calculations */
            int idx = (i * j) % (n - 1);
            arr3[idx] = arr1[(idx + 1) % n] + arr2[(idx + 2) % n];
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
        }
    }
}

/* Kernel 4: Mixed data type dependencies and inline assembly */
__attribute__((noinline))
void kernel4_mixed_types(char* char_arr, int* int_arr, 
                         float* float_arr, double* double_arr, int len) {
    union mixed_types {
        int i;
        float f;
        char c[4];
    } u;
    
    int i;
    
    for (i = 1; i < len - 4; i++) {
        /* Type casting creating dependencies */
        int int_val = int_arr[i-1];
        float float_val = (float)int_val * 0.5f;
        
        /* Dependency through union */
        u.i = int_arr[i];
        float_arr[i] = u.f + float_val;
        
        /* Bitwise operations with dependencies */
        char_arr[i] = (char)((int_arr[i] & 0xFF) | (int_arr[i-1] & 0xF0));
        
        /* Double precision dependency chain */
        double_arr[i] = (double)float_arr[i] * 1.25;
        double_arr[i+1] = double_arr[i] + (double)int_arr[i-2] / 3.0;
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&char_arr[i], &char_arr[i-8], 8);
        }
        
        /* Inline assembly with memory clobber */
        asm volatile("" ::: "memory");
        
        /* Volatile operations */
        volatile int volatile_tmp = int_arr[i % 32];
        char_arr[(i + volatile_tmp) % len] = (char)(i & 0xFF);
    }
}

/* Helper function to initialize arrays */
void initialize_arrays(int arr1[N][M], int arr2[N][M], 
                       int* flat_arr, double* dbl_arr1,
                       double* dbl_arr2, double* dbl_arr3,
                       char* char_arr, float* float_arr,
                       int flat_size, int dbl_size) {
    int i, j;
    
    /* Initialize multi-dimensional arrays */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = (int)(lcg_rand() % 1000);
            arr2[i][j] = (int)(lcg_rand() % 1000);
        }
    }
    
    /* Initialize flat arrays */
    for (i = 0; i < flat_size; i++) {
        flat_arr[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Initialize double arrays */
    for (i = 0; i < dbl_size; i++) {
        dbl_arr1[i] = (double)(lcg_rand() % 1000) / 3.0;
        dbl_arr2[i] = (double)(lcg_rand() % 1000) / 5.0;
        dbl_arr3[i] = (double)(lcg_rand() % 1000) / 7.0;
    }
    
    /* Initialize mixed type arrays */
    for (i = 0; i < dbl_size; i++) {
        char_arr[i] = (char)(lcg_rand() % 256);
        float_arr[i] = (float)(lcg_rand() % 1000) / 11.0f;
    }
}

/* Compute checksum to prevent dead code elimination */
long long compute_checksum(int arr1[N][M], int arr2[N][M],
                           int* flat_arr, double* dbl_arr1,
                           double* dbl_arr2, double* dbl_arr3,
                           char* char_arr, float* float_arr,
                           int flat_size, int dbl_size) {
    long long checksum = 0;
    int i, j;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += arr2[i][j];
        }
    }
    
    for (i = 0; i < flat_size; i++) {
        checksum += flat_arr[i];
    }
    
    for (i = 0; i < dbl_size; i++) {
        checksum += (long long)dbl_arr1[i];
        checksum += (long long)dbl_arr2[i];
        checksum += (long long)dbl_arr3[i];
        checksum += (long long)float_arr[i];
        checksum += (unsigned char)char_arr[i];
    }
    
    return checksum;
}

int main(void) {
    /* Allocate multi-dimensional arrays on stack */
    int arr1[N][M];
    int arr2[N][M];
    
    /* Allocate flat arrays for pointer aliasing */
    int flat_size = 1024;
    int flat_arr[flat_size];
    
    /* Allocate arrays for restrict pointers */
    int dbl_size = 512;
    double dbl_arr1[dbl_size];
    double dbl_arr2[dbl_size];
    double dbl_arr3[dbl_size];
    
    /* Allocate arrays for mixed types */
    char char_arr[dbl_size];
    float float_arr[dbl_size];
    
    /* Initialize all arrays with pseudo-random values */
    initialize_arrays(arr1, arr2, flat_arr, dbl_arr1, dbl_arr2, dbl_arr3,
                     char_arr, float_arr, flat_size, dbl_size);
    
    /* Execute kernel 1: Triple-nested loops with flow dependencies */
    kernel1_flow_dependencies(arr1, arr2);
    
    /* Modify array contents between kernels */
    volatile_barrier = arr1[0][0];
    
    /* Execute kernel 2: Pointer aliasing with anti-dependencies */
    kernel2_pointer_aliasing(flat_arr, flat_size);
    
    /* Modify array contents between kernels */
    asm volatile("" ::: "memory");
    
    /* Execute kernel 3: Restrict pointers with output dependencies */
    kernel3_restrict_pointers(dbl_arr1, dbl_arr2, dbl_arr3, dbl_size);
    
    /* Modify array contents between kernels */
    volatile_barrier = flat_arr[0];
    
    /* Execute kernel 4: Mixed data types with dependencies */
    kernel4_mixed_types(char_arr, flat_arr, float_arr, dbl_arr1, dbl_size);
    
    /* Compute and print checksum to prevent dead code elimination */
    long long checksum = compute_checksum(arr1, arr2, flat_arr, dbl_arr1,
                                         dbl_arr2, dbl_arr3, char_arr,
                                         float_arr, flat_size, dbl_size);
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

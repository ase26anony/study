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

/* Volatile counter to prevent optimization */
static volatile int volatile_counter = 0;

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* Complex 3-level nested loop with RAW dependencies */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            for (k = 1; k < P-1; k++) {
                /* Flow dependencies with varying distances */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j];  /* Distance 1 in i, -2 in j */
                arr2[i][j] = arr1[i][j-1] * 2 - arr2[i-2][j]; /* Distance 2 in i, 1 in j */
                
                /* Conditional loop-carried dependency */
                if ((i + j) % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[i][j-1];  /* Distance 2 in i, 1 in j */
                } else if ((i + j) % 5 == 0) {
                    arr1[i][j] = arr1[i-1][j+1] * arr2[i-3][j]; /* Distance 3 in i, -1 in j */
                }
            }
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(int* arr, int size) {
    int i;
    int *p = &arr[0];
    int *q = &arr[1];  /* q aliases with p+1 */
    int *r = &arr[size/2]; /* Potential aliasing with p/q depending on size */
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (i = 1; i < size - 5; i++) {
        int temp = p[i];      /* Read from p[i] */
        q[i-1] = temp * 3;    /* Write to q[i-1] which aliases p[i] when i=1 */
        r[i/2] = p[i+2] + 1;  /* More complex aliasing pattern */
        
        /* Output dependencies (WAW) */
        p[i] = q[i+1] * 2;    /* Write to p[i] after reading from aliased location */
        
        /* Inline assembly memory barrier to force dependency */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_dependencies(double* restrict dbl_arr1, 
                                        double* restrict dbl_arr2,
                                        int size) {
    int i;
    
    /* Output dependencies with restrict qualifiers */
    for (i = 2; i < size - 2; i++) {
        /* WAW dependencies */
        dbl_arr1[i] = dbl_arr2[i-1] * 1.5;
        dbl_arr1[i] = dbl_arr1[i-2] + dbl_arr2[i+1];  /* Overwrites previous value */
        
        /* Flow dependency chain with restrict */
        dbl_arr2[i] = dbl_arr1[i] * 0.5;
        dbl_arr1[i+1] = dbl_arr2[i] * 2.0;
        
        /* Conditional with different dependency distances */
        if (i % 4 == 0) {
            dbl_arr1[i] = dbl_arr1[i-4] * 3.14;  /* Distance 4 */
        } else {
            dbl_arr1[i] = dbl_arr1[i-1] * 2.71;  /* Distance 1 */
        }
    }
}

/* Kernel 4: Mixed data type dependencies and memory operations */
__attribute__((noinline))
static void kernel4_mixed_types(char* char_arr, float* float_arr, 
                                int* int_arr, int size) {
    int i;
    union {
        int i;
        float f;
        char c[4];
    } converter;
    
    /* Mixed type dependency chain */
    for (i = 1; i < size - 1; i++) {
        /* Type casting creating dependencies */
        converter.i = int_arr[i-1];
        float_arr[i] = converter.f * 1.1f;
        
        /* Bitwise operations with char */
        char_arr[i] = (char_arr[i-1] ^ 0x55) + 1;
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&int_arr[i], &int_arr[i-8], sizeof(int));  /* Distance 8 */
        }
        
        /* Volatile operations to force dependencies */
        volatile_counter++;
        int_arr[i] = volatile_counter + float_arr[i-1];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More type mixing */
        converter.f = float_arr[i];
        char_arr[i+1] = converter.c[0] + converter.c[1];
    }
}

/* Complex kernel with all dependency types combined */
__attribute__((noinline))
static void kernel5_combined(int arr3d[N][M][8], float* farr, int size) {
    int i, j, k;
    
    /* Multi-dimensional strided access with all dependency types */
    for (i = 2; i < N-2; i++) {
        for (j = 2; j < M-2; j++) {
            /* RAW dependency across 3D array */
            arr3d[i][j][0] = arr3d[i-1][j+1][0] + arr3d[i][j-2][0];
            
            /* WAR dependency with mixed array access */
            int temp = arr3d[i][j][1];
            arr3d[i-1][j][1] = temp * 2;
            
            /* WAW dependency */
            arr3d[i][j][2] = farr[i*M + j] * 3;
            arr3d[i][j][2] = arr3d[i][j][2] + 1;  /* Overwrite */
            
            /* Loop-carried with varying distance */
            for (k = 1; k < 7; k++) {
                if ((i + j + k) % 3 == 0) {
                    arr3d[i][j][k] = arr3d[i-2][j][k-1] + 5;  /* Distance 2 in i, 1 in k */
                } else {
                    arr3d[i][j][k] = arr3d[i-1][j+1][k] * 2;  /* Distance 1 in i, -1 in j */
                }
            }
            
            /* Memory barrier every 16 iterations */
            if ((i * M + j) % 16 == 0) {
                asm volatile("" ::: "memory");
            }
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    int arr_linear[N*M];
    double dbl_arr1[N*M/2];
    double dbl_arr2[N*M/2];
    char char_arr[N*M];
    float float_arr[N*M];
    int int_arr[N*M];
    int arr3d[N][M][8];
    
    int i, j, k;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
            arr_linear[i*M + j] = lcg_rand() % 1000;
            
            for (k = 0; k < 8; k++) {
                arr3d[i][j][k] = lcg_rand() % 1000;
            }
        }
    }
    
    for (i = 0; i < N*M; i++) {
        char_arr[i] = lcg_rand() % 256;
        float_arr[i] = (lcg_rand() % 1000) / 10.0f;
        int_arr[i] = lcg_rand() % 1000;
    }
    
    for (i = 0; i < N*M/2; i++) {
        dbl_arr1[i] = (lcg_rand() % 1000) / 10.0;
        dbl_arr2[i] = (lcg_rand() % 1000) / 10.0;
    }
    
    /* Execute kernels with volatile operations between them */
    volatile_counter = 1;
    kernel1_flow_dependencies(arr1, arr2);
    
    volatile_counter++;
    kernel2_anti_dependencies(arr_linear, N*M);
    
    volatile_counter++;
    kernel3_output_dependencies(dbl_arr1, dbl_arr2, N*M/2);
    
    volatile_counter++;
    kernel4_mixed_types(char_arr, float_arr, int_arr, N*M);
    
    volatile_counter++;
    kernel5_combined(arr3d, float_arr, N*M);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += arr2[i][j];
            checksum += arr_linear[i*M + j];
            
            for (k = 0; k < 8; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    for (i = 0; i < N*M; i++) {
        checksum += char_arr[i];
        checksum += (unsigned long long)float_arr[i];
        checksum += int_arr[i];
    }
    
    for (i = 0; i < N*M/2; i++) {
        checksum += (unsigned long long)dbl_arr1[i];
        checksum += (unsigned long long)dbl_arr2[i];
    }
    
    checksum += volatile_counter;
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}

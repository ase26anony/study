/* test_ddg_coverage.c
 * Complex loop nests to trigger DDG edge creation in GCC's scheduler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile counter to prevent optimization */
static volatile int volatile_counter = 0;

/* ========== KERNEL 1: Triple-nested loop with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[M][P]) {
    int i, j, k;
    
    /* Triple nested loop with flow dependencies across dimensions */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            /* Flow dependency (RAW) with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j] + arr2[j][i % P];
            
            for (k = 1; k < P-1; k++) {
                /* Flow dependency with distance 2 in k dimension */
                if (k % 3 == 0) {
                    arr2[j][k] = arr2[j][k-2] * 2 + arr1[i][j];
                } else {
                    arr2[j][k] = arr2[j][k-1] + arr1[i][j] / 2;
                }
                
                /* Additional dependency with varying distance */
                if ((i + j + k) % 5 == 0) {
                    arr1[i][j] += arr2[j][k] - arr2[j][k-3];
                }
            }
            
            /* Anti-dependency (WAR) */
            int temp = arr1[i][j];
            arr1[i][j] = arr2[j][i % P];
            arr2[j][i % P] = temp;
        }
    }
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_pointer_aliasing(double* base_arr, int size) {
    /* Create potentially aliasing pointers */
    double* p = &base_arr[0];
    double* q = &base_arr[1];
    double* r = &base_arr[size/2];
    
    int i;
    
    /* Loop with pointer aliasing creating WAR dependencies */
    for (i = 2; i < size - 2; i++) {
        /* p and q may alias - compiler must assume dependencies */
        double read_q = *q;
        *p = read_q * 2.0 + i;
        
        /* Anti-dependency: read from r, then write to overlapping region */
        double read_r = r[i % (size/2)];
        p[i % 10] = read_r * 3.0;
        
        /* Output dependency (WAW) */
        q[i % 5] = read_q + 1.0;
        if (i % 7 == 0) {
            q[i % 5] = read_r * 2.0;  /* WAW on q[i%5] */
        }
        
        /* Shift pointers to create complex access pattern */
        if (i % 3 == 0) {
            p = &base_arr[i % size];
        }
        if (i % 4 == 0) {
            q = &base_arr[(i + 1) % size];
        }
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_restrict_pointers(float* restrict arr1, 
                                      float* restrict arr2,
                                      float* restrict arr3,
                                      int len) {
    int i;
    
    /* Using restrict allows better optimization but still creates dependencies */
    for (i = 1; i < len - 1; i++) {
        /* Flow dependencies within same array */
        arr1[i] = arr1[i-1] * 1.5f + arr2[i];
        
        /* Output dependency chain */
        arr2[i] = arr3[i] * 2.0f;
        arr2[i] = arr2[i] + arr1[i];  /* WAW on arr2[i] */
        
        /* Dependency with varying distance */
        if (i % 4 == 0) {
            arr3[i] = arr3[i-3] * 0.5f;
        } else {
            arr3[i] = arr3[i-1] * 0.8f;
        }
        
        /* Cross-array flow dependency */
        arr1[i] += arr3[i-2] * 0.3f;
    }
}

/* ========== KERNEL 4: Mixed data types and inline assembly ========== */
__attribute__((noinline))
static void kernel4_mixed_types(char* char_arr, int* int_arr, 
                                float* float_arr, double* double_arr,
                                int size) {
    int i;
    
    /* Union to create type-punning dependencies */
    union mixed_types {
        int i;
        float f;
        char c[4];
    } u;
    
    for (i = 1; i < size - 1; i++) {
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Mixed type dependencies */
        u.i = int_arr[i-1];
        float_arr[i] = u.f * 1.1f;
        
        /* Type casting creating dependencies */
        double_arr[i] = (double)float_arr[i] + (double)int_arr[i] * 0.01;
        
        /* Char array with byte-level dependencies */
        char_arr[i] = (char)(int_arr[i] % 256);
        char_arr[i+1] = char_arr[i] + char_arr[i-1];
        
        /* Volatile operation to force dependency */
        volatile_counter++;
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* Bitwise operations creating dependencies */
        int_arr[i] = (int_arr[i-1] << 2) | (int_arr[i] >> 1);
        
        /* memcpy creating dependencies */
        if (i % 8 == 0) {
            memcpy(&char_arr[i], &char_arr[i-4], 4);
        }
    }
}

/* ========== KERNEL 5: Complex strided multi-dimensional access ========== */
__attribute__((noinline))
static void kernel5_strided_access(int arr3d[P][M][N]) {
    int i, j, k;
    
    /* Complex strided access patterns */
    for (i = 2; i < P-2; i += 2) {  /* Stride 2 in outer loop */
        for (j = 3; j < M-3; j += 3) {  /* Stride 3 */
            for (k = 4; k < N-4; k += 4) {  /* Stride 4 */
                /* Flow dependency with multi-dimensional offsets */
                arr3d[i][j][k] = arr3d[i-1][j+1][k-2] 
                               + arr3d[i][j-2][k+1] 
                               - arr3d[i-2][j][k-1];
                
                /* Anti-dependency with strided access */
                int temp = arr3d[i][j][k];
                arr3d[i][j][k] = arr3d[i+1][j-1][k] * 2;
                arr3d[i+1][j-1][k] = temp;
                
                /* Conditional dependency with varying distance */
                if ((i + j + k) % 7 == 0) {
                    arr3d[i][j][k] += arr3d[i][j][k-4] / 2;
                } else if ((i + j + k) % 11 == 0) {
                    arr3d[i][j][k] += arr3d[i][j][k-6] / 3;
                }
            }
            
            /* Dependency across j dimension */
            arr3d[i][j][N/2] = arr3d[i][j-3][N/2] + arr3d[i][j][N/4];
        }
        
        /* Insert volatile operation between i iterations */
        volatile_counter += i;
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Allocate and initialize arrays with pseudo-random data */
    int arr1[N][M];
    int arr2[M][P];
    double double_arr[N * 2];
    float float_arr[M * 3];
    char char_arr[P * 4];
    int int_arr[N];
    int arr3d[P][M][N];
    
    int i, j, k;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
        }
        int_arr[i] = lcg_rand() % 10000;
    }
    
    for (i = 0; i < M; i++) {
        for (j = 0; j < P; j++) {
            arr2[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (i = 0; i < N * 2; i++) {
        double_arr[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    for (i = 0; i < M * 3; i++) {
        float_arr[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    for (i = 0; i < P * 4; i++) {
        char_arr[i] = (char)(lcg_rand() % 256);
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                arr3d[i][j][k] = lcg_rand() % 500;
            }
        }
    }
    
    /* Execute kernels with volatile operations between them */
    kernel1_flow_dependencies(arr1, arr2);
    volatile_counter++;
    
    kernel2_pointer_aliasing(double_arr, N * 2);
    asm volatile("" ::: "memory");
    
    kernel3_restrict_pointers(float_arr, &float_arr[M], &float_arr[M*2], M);
    volatile_counter += 2;
    
    kernel4_mixed_types(char_arr, int_arr, float_arr, double_arr, N);
    asm volatile("" ::: "memory");
    
    kernel5_strided_access(arr3d);
    volatile_counter += 3;
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j];
            if (j < P) {
                checksum += arr2[j][i % P];
            }
        }
        checksum += int_arr[i];
    }
    
    for (i = 0; i < N * 2; i++) {
        checksum += (long long)double_arr[i];
    }
    
    for (i = 0; i < M * 3; i++) {
        checksum += (long long)float_arr[i];
    }
    
    for (i = 0; i < P * 4; i++) {
        checksum += char_arr[i];
    }
    
    for (i = 0; i < P; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < N; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    checksum += volatile_counter;
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

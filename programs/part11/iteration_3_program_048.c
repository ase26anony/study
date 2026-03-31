/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64
#define ITER 10

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[M][P]) {
    volatile int barrier = 0;
    
    /* Complex 3-level nested loop with RAW dependencies */
    for (int i = 2; i < N - 2; i++) {
        for (int j = 1; j < M - 1; j++) {
            /* Strided access with varying patterns */
            int stride = (i % 4) + 1;
            for (int k = stride; k < P - stride; k += stride) {
                /* Flow dependency: read after write across iterations */
                arr2[j][k] = arr1[i-2][j+1] + arr1[i-1][j] * 2;
                
                /* Cross-dimensional dependency */
                arr1[i][j] = arr2[j-1][k] + arr1[i][j-1] / 3;
                
                /* Additional dependency chain */
                arr2[j][k-1] = arr1[i][j] + arr2[j][k] - 7;
            }
            
            /* Conditional dependency with varying distance */
            if (i % 3 == 0) {
                arr1[i][j] = arr1[i-2][j] + arr1[i][j-2];
            } else if (i % 3 == 1) {
                arr1[i][j] = arr1[i-1][j+1] * arr1[i][j-1];
            } else {
                arr1[i][j] = arr1[i-3][j] - arr1[i][j-3];
            }
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
        barrier = i;
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(float* restrict arr_restrict, 
                                      float* arr_normal, 
                                      float* arr_alias) {
    /* Create aliasing pointers */
    float* p1 = &arr_normal[0];
    float* p2 = &arr_normal[1];  /* p2 aliases p1+1 */
    float* p3 = arr_alias;
    
    volatile float v = 0.0f;
    
    for (int i = 2; i < N*2 - 2; i++) {
        /* Anti-dependency: write after read */
        float temp = p1[i];      /* Read */
        p1[i] = p2[i-1] * 1.5f;  /* Write to potentially overlapping location */
        
        /* More complex aliasing pattern */
        p3[i] = temp + p1[i+1];
        
        /* WAR with pointer arithmetic */
        *(p1 + i) = *(p2 + i - 2) + *(p3 + i + 1);
        
        /* Conditional anti-dependency */
        if (i % 5 == 0) {
            float read_val = p2[i];
            p1[i-3] = read_val * 2.0f;  /* WAR with distance 3 */
        }
        
        /* Use volatile to prevent optimization */
        v = p1[i];
    }
    
    /* Process restrict-qualified pointer separately */
    for (int i = 1; i < N - 1; i++) {
        /* No aliasing assumed due to restrict */
        arr_restrict[i] = arr_restrict[i-1] + arr_restrict[i+1];
    }
}

/* Kernel 3: Loop with output dependencies (WAW) and mixed types */
__attribute__((noinline))
static void kernel3_output_dependencies(double arr_dbl[N], 
                                        char arr_char[M*2],
                                        int arr_int[P]) {
    /* Union for type-punning dependencies */
    union mixed_types {
        double d;
        int i[2];
        char c[8];
    } u;
    
    volatile union mixed_types vu;
    
    for (int i = 3; i < N - 3; i++) {
        /* Output dependency: multiple writes to same location */
        arr_dbl[i] = (double)i * 1.234;
        
        /* WAW with different expressions */
        if (i % 4 == 0) {
            arr_dbl[i] = arr_dbl[i-2] * 0.5;
        } else {
            arr_dbl[i] = arr_dbl[i-1] + 1.0;
        }
        
        /* Mixed-type dependency chain */
        u.d = arr_dbl[i];
        arr_int[i % P] = u.i[0] + u.i[1];
        
        /* Character array with byte-level dependencies */
        for (int j = 0; j < 8 && (i*8 + j) < M*2; j++) {
            /* WAW on byte boundaries */
            arr_char[i*8 + j] = u.c[j] ^ 0x55;
            arr_char[i*8 + j] = arr_char[i*8 + j] + 1;  /* Another write */
        }
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&arr_char[i], &arr_char[i-8], 8);  /* Creates RAW and WAW */
        }
        
        vu = u;
    }
    
    /* Additional WAW pattern with memset */
    for (int i = 0; i < M; i += 16) {
        memset(&arr_char[i], i % 256, 8);
        memset(&arr_char[i], (i+1) % 256, 8);  /* Overwrites previous memset */
    }
}

/* Kernel 4: Complex loop-carried dependencies with varying distances */
__attribute__((noinline))
static void kernel4_variable_distance(long long arr_ll[N*2], 
                                      int arr_src[M][P]) {
    volatile long long barrier_ll = 0;
    
    /* Loop with varying dependency distances */
    for (int i = 4; i < N*2 - 4; i++) {
        /* Variable distance loop-carried dependency */
        switch (i % 7) {
            case 0:
                arr_ll[i] = arr_ll[i-4] + arr_src[i % M][0];  /* distance 4 */
                break;
            case 1:
                arr_ll[i] = arr_ll[i-1] * 2;  /* distance 1 */
                break;
            case 2:
                arr_ll[i] = arr_ll[i-6] - arr_src[i % M][1];  /* distance 6 */
                break;
            case 3:
                arr_ll[i] = arr_ll[i-2] | 0xFF;  /* distance 2, bitwise */
                break;
            case 4:
                arr_ll[i] = arr_ll[i-3] ^ arr_ll[i-5];  /* distances 3 and 5 */
                break;
            case 5:
                arr_ll[i] = (arr_ll[i-1] << 2) + (arr_ll[i-2] >> 1); /* mixed */
                break;
            default:
                arr_ll[i] = arr_ll[i-1] + arr_ll[i-2] + arr_ll[i-3]; /* chain */
                break;
        }
        
        /* Nested loop with strided dependency */
        for (int j = 0; j < P; j += (i % 3) + 1) {
            /* Multi-dimensional dependency */
            arr_src[i % M][j] = arr_src[(i-1) % M][(j+2) % P] 
                              + arr_src[(i-2) % M][j % P];
            
            /* Additional dependency with modulo */
            arr_src[(i+1) % M][j] = arr_src[i % M][j] * 3;
        }
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        barrier_ll = arr_ll[i];
    }
}

/* Main function orchestrating all kernels */
int main(void) {
    /* Allocate multi-dimensional arrays with different types */
    int (*arr1)[M] = malloc(N * sizeof(*arr1));
    int (*arr2)[P] = malloc(M * sizeof(*arr2));
    float *arr_float1 = malloc(N * 2 * sizeof(float));
    float *arr_float2 = malloc(N * 2 * sizeof(float));
    double *arr_double = malloc(N * sizeof(double));
    char *arr_char = malloc(M * 2 * sizeof(char));
    int *arr_int = malloc(P * sizeof(int));
    long long *arr_ll = malloc(N * 2 * sizeof(long long));
    int (*arr_src)[P] = malloc(M * sizeof(*arr_src));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr2[i][j] = (int)lcg_rand() % 1000;
            arr_src[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < N * 2; i++) {
        arr_float1[i] = (float)(lcg_rand() % 1000) / 10.0f;
        arr_float2[i] = (float)(lcg_rand() % 1000) / 10.0f;
        arr_ll[i] = lcg_rand() % 10000;
    }
    
    for (int i = 0; i < N; i++) {
        arr_double[i] = (double)(lcg_rand() % 1000) / 3.0;
    }
    
    for (int i = 0; i < M * 2; i++) {
        arr_char[i] = (char)(lcg_rand() % 256);
    }
    
    for (int i = 0; i < P; i++) {
        arr_int[i] = lcg_rand() % 1000;
    }
    
    volatile int iter_counter = 0;
    
    /* Execute kernels multiple times to increase coverage chances */
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify array contents between kernels using volatile */
        volatile int modifier = lcg_rand() % 100;
        arr1[0][0] += modifier;
        
        /* Kernel 1: Flow dependencies */
        kernel1_flow_dependencies(arr1, arr2);
        
        /* Inter-kernel volatile operation */
        asm volatile("" ::: "memory");
        iter_counter = iter;
        
        /* Kernel 2: Anti-dependencies with aliasing */
        kernel2_anti_dependencies(arr_float1, arr_float2, arr_float1 + 1);
        
        /* More inter-kernel operations */
        arr_double[0] += (double)modifier;
        
        /* Kernel 3: Output dependencies with mixed types */
        kernel3_output_dependencies(arr_double, arr_char, arr_int);
        
        /* Kernel 4: Variable distance dependencies */
        kernel4_variable_distance(arr_ll, arr_src);
        
        /* Cross-kernel dependency */
        arr_ll[0] = arr1[0][0];
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            checksum += arr2[i][j];
            checksum += arr_src[i][j];
        }
    }
    
    for (int i = 0; i < N * 2; i++) {
        checksum += (long long)arr_float1[i];
        checksum += (long long)arr_float2[i];
        checksum += arr_ll[i];
    }
    
    for (int i = 0; i < N; i++) {
        checksum += (long long)arr_double[i];
    }
    
    for (int i = 0; i < M * 2; i++) {
        checksum += arr_char[i];
    }
    
    for (int i = 0; i < P; i++) {
        checksum += arr_int[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr_float1);
    free(arr_float2);
    free(arr_double);
    free(arr_char);
    free(arr_int);
    free(arr_ll);
    free(arr_src);
    
    return 0;
}

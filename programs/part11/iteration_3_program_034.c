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

/* Volatile sink to prevent optimization */
static volatile int volatile_sink;

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    for (int i = 2; i < N-2; i++) {
        for (int j = 2; j < M-2; j++) {
            for (int k = 1; k < P; k++) {
                /* Complex flow dependencies with varying distances */
                arr1[i][j] = arr1[i-1][j+1] + arr1[i-2][j] * 3;
                arr2[i][j] = arr1[i][j-1] + arr2[i-1][j+2];
                
                /* Cross-dimensional dependency */
                if (k % 4 == 0) {
                    arr1[i][j] += arr2[i][j-2] >> 1;
                }
                
                /* Memory barrier to preserve dependencies */
                asm volatile("" ::: "memory");
            }
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* q aliases p+1 */
    int *r = &base_arr[2];  /* r aliases p+2 */
    
    for (int i = 3; i < size-3; i++) {
        /* Anti-dependency: read before write to aliased location */
        int temp = *p + *q;
        
        /* Write to p (WAR with next iteration's read) */
        *p = temp * 2 - *r;
        
        /* Shift pointers creating overlapping accesses */
        p = &base_arr[i];
        q = &base_arr[i-1];
        r = &base_arr[i-2];
        
        /* Conditional dependency with varying distance */
        if (i % 5 == 0) {
            base_arr[i] = base_arr[i-3] + base_arr[i-4];
        } else if (i % 3 == 0) {
            base_arr[i] = base_arr[i-2] * base_arr[i-1];
        }
        
        volatile_sink = *p;  /* Volatile read to preserve dependencies */
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(int* restrict r1, int* restrict r2, 
                                double* restrict d1, int len) {
    /* Output dependencies through restrict pointers */
    for (int i = 2; i < len-2; i++) {
        /* Multiple writes to same location (WAW) */
        r1[i] = r1[i-1] + r2[i];
        r1[i] = r1[i] * 2 - r2[i-1];  /* Overwrites previous value */
        
        /* Type mixing with casting */
        d1[i] = (double)r1[i] / 3.14159;
        r2[i] = (int)(d1[i-1] * 2.71828);
        
        /* Strided access pattern */
        if (i % 8 == 0) {
            r1[i] = r1[i-8] ^ r1[i-4];  /* Bitwise operation dependency */
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, float* f_arr, 
                                double* d_arr, int* i_arr, int len) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    for (int i = 4; i < len-4; i++) {
        /* Dependency chain through different types */
        u.i = i_arr[i-1];
        f_arr[i] = u.f * 1.5f;
        
        /* Type punning through union creates dependencies */
        u.f = f_arr[i-2];
        i_arr[i] = u.i >> 2;
        
        /* Char array with pointer arithmetic */
        char* cp = &c_arr[i];
        cp[0] = (char)(i_arr[i-3] & 0xFF);
        cp[1] = (char)((i_arr[i-2] >> 8) & 0xFF);
        
        /* Double precision dependency */
        d_arr[i] = d_arr[i-1] * 1.01 + (double)f_arr[i];
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&c_arr[i], &c_arr[i-4], 4);
        }
        
        /* Inline assembly barrier preserving all dependencies */
        asm volatile("" ::: "memory");
        
        /* Volatile write */
        volatile int* vp = (volatile int*)&i_arr[i];
        *vp = *vp + 1;
    }
}

/* Complex loop with all dependency types */
__attribute__((noinline))
static void kernel5_complex_nest(int arr3d[32][32][32]) {
    for (int i = 2; i < 30; i++) {
        for (int j = 2; j < 30; j++) {
            for (int k = 2; k < 30; k++) {
                /* RAW dependency */
                int t1 = arr3d[i-1][j][k] + arr3d[i][j-1][k];
                
                /* WAR dependency */
                arr3d[i][j][k] = t1 * arr3d[i][j][k-1];
                
                /* WAW dependency */
                arr3d[i][j][k] = arr3d[i][j][k] + arr3d[i-2][j+1][k-1];
                
                /* Dependency with non-linear distance */
                if ((i * j * k) % 7 == 0) {
                    arr3d[i][j][k] = arr3d[i-3][j][k] ^ arr3d[i][j-2][k];
                } else if ((i + j + k) % 5 == 0) {
                    arr3d[i][j][k] = arr3d[i][j][k-4] | arr3d[i-1][j+1][k];
                }
                
                /* Memory barrier every 8 iterations */
                if (k % 8 == 0) {
                    asm volatile("" ::: "memory");
                }
            }
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    int linear_arr[N*M];
    double double_arr[1024];
    float float_arr[1024];
    char char_arr[1024];
    int int_arr[1024];
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
        double_arr[i] = (double)(lcg_rand() % 1000) / 3.0;
        float_arr[i] = (float)(lcg_rand() % 1000) / 7.0f;
        char_arr[i] = (char)(lcg_rand() % 256);
        int_arr[i] = lcg_rand() % 1000;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                arr3d[i][j][k] = lcg_rand() % 1000;
            }
        }
    }
    
    /* Execute kernels with volatile operations between them */
    kernel1_flow_deps(arr1, arr2);
    
    volatile_sink = arr1[0][0];  /* Prevent cross-kernel optimization */
    
    kernel2_anti_deps(linear_arr, N*M);
    
    asm volatile("" ::: "memory");  /* Compiler barrier */
    
    kernel3_output_deps(&linear_arr[100], &linear_arr[200], 
                       double_arr, 512);
    
    volatile_sink = linear_arr[150];
    
    kernel4_mixed_types(char_arr, float_arr, double_arr, int_arr, 512);
    
    volatile_sink = char_arr[0] + float_arr[0];
    
    kernel5_complex_nest(arr3d);
    
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
        checksum += (unsigned long long)double_arr[i];
        checksum += (unsigned long long)float_arr[i];
        checksum += char_arr[i];
        checksum += int_arr[i];
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}

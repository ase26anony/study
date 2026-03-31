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

/* ========== KERNEL 1: Triple-nested loop with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* Triple nested loop with flow dependencies across all dimensions */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            for (k = 1; k < P-1; k++) {
                /* Flow (RAW) dependency with varying distances */
                arr1[i][j] = arr1[i-1][j+1] + arr1[i][j-1] * 2;
                
                /* Cross-array dependency */
                arr2[i][j] = arr1[i][j] + arr2[i-2][j+2];
                
                /* Additional dependency with non-unit stride */
                if (k % 3 == 0) {
                    arr1[i][j] += arr1[i-2][j] / 3;
                } else {
                    arr1[i][j] += arr1[i-1][j+1] * 2;
                }
            }
            
            /* Anti-dependency (WAR) within same iteration */
            int temp = arr1[i][j];
            arr1[i][j] = arr2[i][j] + 1;
            arr2[i][j] = temp - 1;
        }
    }
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_pointer_aliasing(float* arr, int size) {
    int i;
    
    /* Create aliasing pointers */
    float* p = &arr[0];
    float* q = &arr[1];
    float* r = &arr[2];
    
    /* Loop with anti-dependencies through aliasing pointers */
    for (i = 3; i < size - 3; i++) {
        /* Read from q (will be overwritten by p write) */
        float val = *q + *r;
        
        /* Write through p (may alias with q/r) - creates WAR */
        *p = val * 2.0f;
        
        /* Output dependency (WAW) - p and q may alias */
        *q = *p + *r;
        
        /* Complex addressing with potential aliasing */
        if (i % 4 == 0) {
            *(p + 1) = *(q - 1) + *(r + 1);
        } else if (i % 4 == 1) {
            *(p + 2) = *(q - 2) * *(r + 2);
        }
        
        /* Move pointers - creates varying access patterns */
        p = &arr[i % 16];
        q = &arr[(i + 1) % 16];
        r = &arr[(i + 2) % 16];
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_restrict_pointers(double* restrict dst, 
                                      double* restrict src1, 
                                      double* restrict src2, 
                                      int size) {
    int i;
    
    /* Loop with restrict pointers - compiler knows no aliasing */
    for (i = 2; i < size - 2; i++) {
        /* Output dependency (WAW) on dst */
        dst[i] = src1[i-1] + src2[i+1];
        
        /* Flow dependency chain */
        dst[i] = dst[i] * 1.5 - src1[i-2];
        
        /* Conditional dependency with varying distance */
        if (i % 5 == 0) {
            dst[i] = dst[i-3] * 0.8;
        } else if (i % 5 == 1) {
            dst[i] = dst[i-2] * 1.2;
        } else {
            dst[i] = dst[i-1] * 0.9;
        }
        
        /* Mixed operations */
        dst[i] = (double)((int)dst[i] ^ 0xFF) / 256.0;
    }
}

/* ========== KERNEL 4: Mixed data types and assembly barriers ========== */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, int* i_arr, 
                                float* f_arr, double* d_arr, int size) {
    int i;
    union {
        int i;
        float f;
        char bytes[4];
    } converter;
    
    /* Volatile force dependency */
    volatile int vol_dep = volatile_counter;
    
    for (i = 1; i < size - 1; i++) {
        /* Memory barrier */
        asm volatile("" ::: "memory");
        
        /* Dependency chain across different types */
        int int_val = i_arr[i-1] + vol_dep;
        
        /* Type conversion creates dependency */
        converter.i = int_val;
        float float_val = converter.f * 1.5f;
        
        /* Write to float array */
        f_arr[i] = float_val + f_arr[i-1];
        
        /* Cast to char and back */
        char char_val = (char)(int_val & 0xFF);
        c_arr[i] = char_val + c_arr[i-1];
        
        /* Double precision computation */
        d_arr[i] = (double)f_arr[i] / (double)(c_arr[i] + 1);
        
        /* Bitwise operations creating dependencies */
        i_arr[i] = (i_arr[i-1] << 2) | (i_arr[i] >> 3);
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        
        /* Use memcpy for dependency */
        if (i % 8 == 0) {
            memcpy(&c_arr[i], &c_arr[i-4], 4);
        }
        
        /* Update volatile to force dependency */
        vol_dep++;
    }
    
    volatile_counter = vol_dep;
}

/* ========== KERNEL 5: Complex loop-carried dependencies ========== */
__attribute__((noinline))
static void kernel5_complex_dependencies(int arr[N][M]) {
    int i, j;
    
    /* Complex loop-carried dependencies with varying distances */
    for (i = 2; i < N - 2; i++) {
        for (j = 2; j < M - 2; j++) {
            /* Multiple dependencies with different distances */
            if ((i + j) % 3 == 0) {
                /* Distance 2 dependency */
                arr[i][j] = arr[i-2][j+1] * arr[i][j-2];
            } else if ((i + j) % 3 == 1) {
                /* Distance 1 dependency */
                arr[i][j] = arr[i-1][j+2] + arr[i][j-1];
            } else {
                /* Distance 3 dependency */
                arr[i][j] = arr[i-3][j] - arr[i][j-3];
            }
            
            /* Additional cross-iteration dependency */
            arr[i][j] += arr[i][j] * (arr[i-1][j-1] % 7);
            
            /* Strided access pattern */
            if (j % 4 == 0) {
                arr[i][j] ^= arr[i][j-4];  /* Bitwise dependency */
            }
        }
        
        /* Dependency across outer loop iterations */
        if (i % 5 == 0) {
            for (j = 1; j < M - 1; j++) {
                arr[i][j] = arr[i-5][j] + arr[i][j];
            }
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with different data types */
    int arr1[N][M];
    int arr2[N][M];
    float float_arr[512];
    double double_arr[1024];
    char char_arr[1024];
    int int_arr[1024];
    
    int i, j;
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (i = 0; i < 512; i++) {
        float_arr[i] = (float)(lcg_rand() % 1000) / 3.0f;
    }
    
    for (i = 0; i < 1024; i++) {
        double_arr[i] = (double)(lcg_rand() % 1000) / 7.0;
        char_arr[i] = (char)(lcg_rand() % 256);
        int_arr[i] = lcg_rand() % 10000;
    }
    
    /* Execute kernels with volatile operations between them */
    kernel1_flow_dependencies(arr1, arr2);
    
    volatile_counter++;
    asm volatile("" ::: "memory");
    
    kernel2_pointer_aliasing(float_arr, 512);
    
    volatile_counter += 2;
    asm volatile("" ::: "memory");
    
    kernel3_restrict_pointers(double_arr, float_arr, double_arr, 1024);
    
    volatile_counter += 3;
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(char_arr, int_arr, float_arr, double_arr, 1024);
    
    volatile_counter += 4;
    asm volatile("" ::: "memory");
    
    kernel5_complex_dependencies(arr1);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += (unsigned int)arr1[i][j];
            checksum += (unsigned int)arr2[i][j];
        }
    }
    
    for (i = 0; i < 512; i++) {
        checksum += (unsigned int)(float_arr[i] * 1000);
    }
    
    for (i = 0; i < 1024; i++) {
        checksum += (unsigned long long)(double_arr[i] * 1000);
        checksum += (unsigned char)char_arr[i];
        checksum += int_arr[i];
    }
    
    checksum += volatile_counter;
    
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}

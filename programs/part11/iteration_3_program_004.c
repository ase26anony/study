/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128
#define ITER 10

/* Volatile counter to prevent optimization */
static volatile int volatile_counter = 0;

/* Simple LCG for pseudo-random initialization */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* True dependencies (RAW) across i dimension */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            /* Flow dependency with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j] + arr2[i][j];
            
            /* Additional dependency with stride in j dimension */
            if (j % 3 == 0) {
                arr1[i][j] += arr1[i][j-2] * 2;  /* Distance 2 in j */
            } else {
                arr1[i][j] += arr1[i][j-1] * 3;  /* Distance 1 in j */
            }
        }
    }
    
    /* Third dimension simulated with k loop */
    for (k = 0; k < P; k++) {
        for (i = 2; i < N-2; i++) {
            for (j = 2; j < M-2; j++) {
                /* Complex dependency pattern with varying distances */
                int idx = (i * j + k) % (M-4);
                arr2[i][j] = arr1[i-1][idx] + arr1[i-2][j+1] - arr2[i-1][j-1];
                
                /* Conditional dependency distance */
                if ((i + j) % 4 == 0) {
                    arr2[i][j] += arr2[i-2][j+2];  /* Distance 2 in i, -2 in j */
                } else if ((i + j) % 4 == 1) {
                    arr2[i][j] += arr2[i-1][j+1];  /* Distance 1 in i, -1 in j */
                }
            }
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        volatile_counter++;
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(int* base_arr, int size) {
    int i;
    
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    
    /* Anti-dependencies through aliasing pointers */
    for (i = 1; i < size-1; i++) {
        int temp = p[i];           /* Read from p[i] */
        q[i-1] = temp * 2;         /* Write to q[i-1] which may alias p[i] */
        
        /* Additional anti-dependency chain */
        temp = r[i/2];             /* Read from r */
        p[i] = temp + i;           /* Write to p[i] */
        
        /* Complex aliasing pattern */
        if (i % 8 == 0) {
            int* alias_ptr = (i % 16 == 0) ? p : q;
            int read_val = alias_ptr[i/2];
            base_arr[i] = read_val * 3;
        }
    }
    
    /* Volatile operation to create barrier */
    volatile int* volatile_ptr = (volatile int*)base_arr;
    for (i = 0; i < 4; i++) {
        volatile_ptr[i] = volatile_ptr[i] + 1;
    }
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_dependencies(double* restrict arr1, 
                                        double* restrict arr2, 
                                        double* restrict arr3, 
                                        int len) {
    int i, j;
    
    /* Output dependencies within single array */
    for (i = 2; i < len-2; i++) {
        /* Multiple writes to same location with intermediate reads */
        double val = arr1[i-1] + arr2[i];
        arr1[i] = val * 1.5;           /* First write */
        
        /* Computation that might be reordered without dependencies */
        double tmp = arr3[i+1] - arr3[i-1];
        
        arr1[i] = arr1[i] + tmp;       /* Second write to same location (WAW) */
        
        /* Loop-carried output dependency */
        if (i % 3 == 0) {
            arr2[i/3] = arr1[i] * 2.0;
        }
    }
    
    /* Nested loop with restrict but complex indexing */
    for (j = 0; j < 4; j++) {
        for (i = j; i < len - 4; i += 4) {
            /* Output dependency across iterations */
            arr3[i] = arr1[i+j] * arr2[i-j];
            arr3[i] = arr3[i] / (j + 1);  /* Overwrite */
            
            /* Flow dependency with restrict pointers */
            arr1[i+1] = arr3[i] + arr2[i];
        }
        
        /* Memory barrier between outer loop iterations */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* char_arr, int* int_arr, 
                                float* float_arr, double* double_arr, 
                                int size) {
    int i;
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    /* Dependencies across different data types */
    for (i = 1; i < size-1; i++) {
        /* Type casting creating dependencies */
        int int_val = int_arr[i-1];
        float float_val = float_arr[i];
        
        /* Mixed-type computation */
        double_arr[i] = (double)int_val * (double)float_val;
        
        /* Bitwise operations with char */
        char_arr[i] = (char)(int_val & 0xFF) + char_arr[i-1];
        
        /* Union access creating aliasing dependencies */
        u.i = int_arr[i];
        float_arr[i] = u.f * 0.5f;
        
        /* Inline assembly barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
            volatile_counter += char_arr[i];
        }
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&int_arr[i], &int_arr[i-4], sizeof(int) * 2);
        }
    }
    
    /* Volatile operations with different types */
    volatile float volatile_float = 0.0f;
    for (i = 0; i < 8; i++) {
        volatile_float += float_arr[i];
        asm volatile("" ::: "memory");
    }
    
    /* memset creating output dependencies */
    memset(&char_arr[size-16], 0, 16);
}

/* Main function orchestrating all kernels */
int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    double double_arr1[512];
    double double_arr2[512];
    double double_arr3[512];
    char char_arr[1024];
    int int_arr[1024];
    float float_arr[1024];
    
    int i, j;
    
    /* Initialize with LCG to avoid compile-time computation */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (i = 0; i < 512; i++) {
        double_arr1[i] = (double)(lcg_rand() % 1000) / 10.0;
        double_arr2[i] = (double)(lcg_rand() % 1000) / 10.0;
        double_arr3[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    for (i = 0; i < 1024; i++) {
        char_arr[i] = (char)(lcg_rand() % 256);
        int_arr[i] = lcg_rand() % 10000;
        float_arr[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    /* Execute kernels multiple times to ensure DDG construction */
    for (int iter = 0; iter < ITER; iter++) {
        /* Volatile operation between kernels */
        volatile_counter += iter;
        
        /* Kernel 1: Complex flow dependencies */
        kernel1_flow_dependencies(arr1, arr2);
        
        /* Kernel 2: Anti-dependencies with pointer aliasing */
        kernel2_anti_dependencies(&arr1[0][0], N*M);
        
        /* Kernel 3: Output dependencies with restrict */
        kernel3_output_dependencies(double_arr1, double_arr2, double_arr3, 512);
        
        /* Kernel 4: Mixed types with barriers */
        kernel4_mixed_types(char_arr, int_arr, float_arr, double_arr1, 512);
        
        /* Modify array contents between iterations */
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                arr1[i][j] += volatile_counter;
                asm volatile("" ::: "memory");
            }
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (i = 0; i < 512; i++) {
        checksum += (unsigned long long)double_arr1[i];
        checksum += (unsigned long long)double_arr2[i];
        checksum += (unsigned long long)double_arr3[i];
    }
    
    for (i = 0; i < 1024; i++) {
        checksum += char_arr[i] + int_arr[i] + (unsigned long long)float_arr[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Volatile counter: %d\n", volatile_counter);
    
    return 0;
}

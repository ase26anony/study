#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128

/* Volatile variables to prevent optimization */
volatile int v_counter = 0;
volatile int v_seed = 12345;

/* Simple LCG for pseudo-random initialization */
static inline int lcg_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* True dependencies (RAW) across i dimension */
    for (i = 1; i < N; i++) {
        for (j = 1; j < M - 2; j++) {
            /* Flow dependency with distance 1 in i, distance 2 in j */
            arr1[i][j] = arr1[i-1][j+2] + arr2[i][j];
            
            /* Additional dependency chain */
            for (k = 0; k < P; k += 4) {
                /* Strided access with dependency */
                arr2[i][j] += arr1[i][j] * (k % 16);
                
                /* Memory barrier to prevent reordering */
                asm volatile("" ::: "memory");
            }
        }
    }
    
    /* Anti-dependencies (WAR) in reverse traversal */
    for (i = N - 2; i >= 0; i--) {
        for (j = M - 2; j >= 1; j--) {
            int temp = arr1[i][j];
            arr1[i+1][j-1] = temp * 2;  // WAR: read arr1[i][j], write arr1[i+1][j-1]
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies */
__attribute__((noinline))
void kernel2_aliasing_deps(int *base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  // q aliases p+1
    int *r = &base_arr[size/2];  // May alias depending on size
    
    int i;
    
    /* WAR (anti-dependency) through aliasing pointers */
    for (i = 1; i < size - 1; i++) {
        int read_val = p[i];          // Read from p[i]
        q[i-1] = read_val * 3;        // Write to q[i-1] (aliases p[i] when i=1)
        r[i % (size/2)] += read_val;  // Potential aliasing with p
        
        /* Varying dependency distances */
        if (i % 3 == 0) {
            p[i] = p[i-2] + 1;        // Distance 2 dependency
        } else if (i % 5 == 0) {
            p[i] = p[i-3] * 2;        // Distance 3 dependency
        } else {
            p[i] = p[i-1] + v_counter; // Distance 1 with volatile
        }
    }
    
    /* Output dependencies (WAW) */
    for (i = 0; i < size - 4; i += 2) {
        base_arr[i] = i * i;
        base_arr[i] = base_arr[i] + base_arr[i+4];  // WAW on base_arr[i]
    }
}

/* Kernel 3: Restrict pointers and output dependencies */
__attribute__((noinline))
void kernel3_restrict_deps(double *restrict d1, double *restrict d2, 
                           float *restrict f1, int len) {
    int i;
    
    /* Output dependencies (WAW) with restrict */
    for (i = 2; i < len; i++) {
        d1[i] = d1[i-1] * 1.5;        // Flow dependency
        d1[i-1] = d2[i] + 2.0;        // WAW on d1[i-1]
        
        /* Mixed type dependencies through casting */
        f1[i] = (float)d1[i] + (float)d2[i-2];
        
        /* Dependency with varying stride */
        if (i % 4 == 0) {
            d2[i] = d2[i-4] * 0.5;    // Distance 4
        } else {
            d2[i] = d2[i-1] + 1.0;    // Distance 1
        }
    }
    
    /* Complex loop-carried dependencies */
    for (i = 4; i < len - 4; i++) {
        /* Multiple overlapping dependencies */
        d1[i] = d1[i-2] + d1[i-3] + d1[i-4];
        d2[i] = d1[i] * d2[i-1];
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&f1[i], &f1[i-8], sizeof(float) * 4);
        }
    }
}

/* Kernel 4: Mixed data types and assembly barriers */
__attribute__((noinline))
void kernel4_mixed_types(char *c_arr, int *i_arr, float *f_arr, 
                         double *d_arr, int size) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    int i;
    
    /* Type-punning through union creates dependencies */
    for (i = 1; i < size; i++) {
        u.i = i_arr[i-1];
        f_arr[i] = u.f * 0.5f;  // Dependency through union
        
        /* Bitwise operations with dependencies */
        i_arr[i] = (i_arr[i-1] << 2) | (i_arr[i] & 0x3);
        
        /* Char array with pointer arithmetic */
        c_arr[i] = c_arr[i-1] + (i % 128);
        
        /* Double with volatile influence */
        d_arr[i] = d_arr[i-1] * (1.0 + (v_counter * 0.001));
        
        /* Inline assembly barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Mixed type casting dependency chain */
    for (i = 2; i < size - 2; i++) {
        /* Chain: int -> float -> double -> int */
        float temp_f = (float)i_arr[i];
        double temp_d = (double)temp_f + d_arr[i-1];
        i_arr[i+1] = (int)temp_d + i_arr[i-2];
        f_arr[i] = temp_f * 0.75f;
        
        /* Volatile write to force dependency */
        v_counter = i & 0xFF;
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int arr1[N][M], int arr2[N][M], double d_arr1[], 
                 double d_arr2[], float f_arr[], char c_arr[], 
                 int i_arr[], int size) {
    int seed = 54321;
    int i, j;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand(&seed) % 1000;
            arr2[i][j] = lcg_rand(&seed) % 1000;
        }
    }
    
    for (i = 0; i < size; i++) {
        d_arr1[i] = (lcg_rand(&seed) % 1000) * 0.1;
        d_arr2[i] = (lcg_rand(&seed) % 1000) * 0.1;
        f_arr[i] = (lcg_rand(&seed) % 1000) * 0.01f;
        c_arr[i] = lcg_rand(&seed) % 256;
        i_arr[i] = lcg_rand(&seed) % 10000;
    }
}

/* Compute checksum to prevent dead code elimination */
long long compute_checksum(int arr1[N][M], int arr2[N][M], double d_arr1[],
                          double d_arr2[], float f_arr[], char c_arr[],
                          int i_arr[], int size) {
    long long checksum = 0;
    int i, j;
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += arr2[i][j];
        }
    }
    
    for (i = 0; i < size; i++) {
        checksum += (long long)d_arr1[i];
        checksum += (long long)d_arr2[i];
        checksum += (long long)f_arr[i];
        checksum += c_arr[i];
        checksum += i_arr[i];
    }
    
    return checksum;
}

int main() {
    /* Allocate multi-dimensional arrays */
    int (*arr1)[M] = malloc(N * sizeof(*arr1));
    int (*arr2)[M] = malloc(N * sizeof(*arr2));
    
    const int size = 1024;
    double *d_arr1 = malloc(size * sizeof(double));
    double *d_arr2 = malloc(size * sizeof(double));
    float *f_arr = malloc(size * sizeof(float));
    char *c_arr = malloc(size * sizeof(char));
    int *i_arr = malloc(size * sizeof(int));
    
    if (!arr1 || !arr2 || !d_arr1 || !d_arr2 || !f_arr || !c_arr || !i_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(arr1, arr2, d_arr1, d_arr2, f_arr, c_arr, i_arr, size);
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_deps(arr1, arr2);
    
    /* Volatile operation between kernels */
    v_counter++;
    asm volatile("" ::: "memory");
    
    kernel2_aliasing_deps(&arr1[0][0], N * M);
    
    /* More volatile operations */
    v_seed = v_counter * 7;
    asm volatile("" ::: "memory");
    
    kernel3_restrict_deps(d_arr1, d_arr2, f_arr, size);
    
    v_counter += v_seed;
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(c_arr, i_arr, f_arr, d_arr1, size);
    
    /* Final checksum to prevent optimization */
    long long checksum = compute_checksum(arr1, arr2, d_arr1, d_arr2, 
                                         f_arr, c_arr, i_arr, size);
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(d_arr1);
    free(d_arr2);
    free(f_arr);
    free(c_arr);
    free(i_arr);
    
    return 0;
}

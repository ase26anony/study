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
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* True dependencies (RAW) across i dimension */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            /* Flow dependency with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j] + arr2[i][j];
            
            /* Flow dependency with distance 2 in j dimension */
            if (j >= 2) {
                arr2[i][j] = arr1[i][j-2] * 3 - arr2[i][j-1];
            }
            
            /* Additional dependency with varying distance */
            for (k = 0; k < P; k++) {
                /* Multi-dimensional strided access creating complex dependencies */
                int idx = (i * 3 + j * 5 + k * 7) % N;
                if (idx > 0) {
                    /* Flow dependency through arr1 with non-unit stride */
                    arr1[i][j] += arr1[idx][(j + k) % M] / (k + 1);
                }
            }
        }
    }
    
    /* Anti-dependencies (WAR) in reverse traversal */
    for (i = N-2; i > 0; i--) {
        for (j = M-2; j > 0; j--) {
            /* Read after write from previous loop creates anti-dependency */
            int temp = arr1[i][j];
            arr1[i-1][j+1] = temp * 2;  // WAR with previous iteration's writes
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies */
__attribute__((noinline))
static void kernel2_aliasing_deps(int *base_arr, int size) {
    int i;
    
    /* Create aliasing pointers */
    int *p = &base_arr[0];
    int *q = &base_arr[1];
    int *r = &base_arr[size/2];
    
    /* Output dependencies (WAW) with aliasing */
    for (i = 1; i < size-1; i++) {
        p[i] = q[i-1] + r[i+1];  // Flow dependency through q and r
        
        /* Conditional aliasing creates varying dependency patterns */
        if (i % 4 == 0) {
            *q = p[i] * 2;  // q aliases base_arr[1], may alias p[i] for some i
            q = &base_arr[i % 16];  // Change q pointer periodically
        }
        
        /* Anti-dependency (WAR) */
        int read_val = p[i-1];  // Read
        p[i] = read_val + i;    // Write to different location
        
        /* Output dependency (WAW) with potential aliasing */
        if (i % 3 == 0) {
            r[i/2] = p[i] * 3;
            p[i] = r[i/2] + 1;  // WAW if r[i/2] aliases p[i]
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Kernel 3: Loop with restrict pointers and output dependencies */
__attribute__((noinline))
static void kernel3_restrict_deps(double *restrict d1, 
                                  double *restrict d2, 
                                  double *restrict d3, 
                                  int len) {
    int i;
    
    /* Output dependencies (WAW) within restrict pointers */
    for (i = 2; i < len-2; i++) {
        d1[i] = d2[i-1] + d3[i+1];  // Flow dependencies
        
        /* Loop-carried dependency with distance 2 */
        if (i % 5 == 0) {
            d2[i] = d1[i-2] * 1.5;  // Distance 2 flow dependency
        } else {
            d2[i] = d1[i-1] * 2.0;  // Distance 1 flow dependency
        }
        
        /* Multiple writes creating output dependencies */
        double temp = d3[i];
        d3[i] = temp * 0.5;        // WAW on d3[i]
        d3[i] = d3[i] + d1[i];     // Another WAW on d3[i]
    }
    
    /* Nested loop with strided access */
    for (i = 0; i < len; i += 8) {
        for (int j = 0; j < 8; j++) {
            int idx = i + j;
            if (idx < len) {
                /* Complex addressing with restrict pointers */
                d1[idx] = d2[(idx * 3) % len] + d3[(idx * 7) % len];
            }
        }
    }
}

/* Kernel 4: Mixed data type dependencies and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(float *farr, double *darr, 
                                char *carr, int *iarr, int size) {
    int i;
    union {
        int i;
        float f;
        char c[4];
    } converter;
    
    /* Dependencies through type punning */
    for (i = 1; i < size-1; i++) {
        /* Flow dependency with type conversion */
        converter.i = iarr[i-1];
        farr[i] = converter.f * 2.0f;
        
        /* Anti-dependency with different type */
        float old_f = farr[i];  // Read float
        iarr[i] = (int)(old_f * 10.0f);  // Write int
        
        /* Output dependency through char array */
        carr[i] = (char)(iarr[i] & 0xFF);
        carr[i] = carr[i] + 1;  // WAW on carr[i]
        
        /* Memory barrier every 16 iterations */
        if (i % 16 == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Dependency through memcpy */
        if (i % 32 == 0) {
            memcpy(&darr[i], &farr[i], sizeof(float));
            darr[i] = darr[i] * 3.14159;  // WAW on darr[i]
        }
        
        /* Volatile operations creating artificial dependencies */
        v_counter++;
        if (v_counter % 64 == 0) {
            iarr[i] = v_counter;
        }
    }
    
    /* Bitwise operations creating dependencies */
    for (i = 0; i < size; i++) {
        /* Flow dependency through bitwise ops */
        iarr[i] = (iarr[i] << 3) | (iarr[i] >> 29);
        
        /* Dependency through union access */
        converter.f = farr[i];
        carr[i % 256] = converter.c[0];  // May alias with other accesses
    }
}

int main(void) {
    /* Allocate and initialize multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    int linear_arr[N * M * 2];
    double darr1[1024];
    double darr2[1024];
    double darr3[1024];
    float farr[2048];
    char carr[4096];
    int iarr[2048];
    
    int seed = 12345;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand(&seed) % 1000;
            arr2[i][j] = lcg_rand(&seed) % 1000;
        }
    }
    
    for (int i = 0; i < N * M * 2; i++) {
        linear_arr[i] = lcg_rand(&seed) % 1000;
    }
    
    for (int i = 0; i < 1024; i++) {
        darr1[i] = (double)(lcg_rand(&seed) % 1000) / 10.0;
        darr2[i] = (double)(lcg_rand(&seed) % 1000) / 10.0;
        darr3[i] = (double)(lcg_rand(&seed) % 1000) / 10.0;
    }
    
    for (int i = 0; i < 2048; i++) {
        farr[i] = (float)(lcg_rand(&seed) % 1000) / 10.0f;
        iarr[i] = lcg_rand(&seed) % 1000;
    }
    
    for (int i = 0; i < 4096; i++) {
        carr[i] = (char)(lcg_rand(&seed) % 256);
    }
    
    /* Execute kernels with volatile barriers between them */
    kernel1_flow_deps(arr1, arr2);
    
    v_seed = lcg_rand(&v_seed);
    asm volatile("" ::: "memory");
    
    kernel2_aliasing_deps(linear_arr, N * M * 2);
    
    v_counter++;
    asm volatile("" ::: "memory");
    
    kernel3_restrict_deps(darr1, darr2, darr3, 1024);
    
    v_seed = lcg_rand(&v_seed);
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(farr, darr1, carr, iarr, 2048);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < N * M * 2; i++) {
        checksum += linear_arr[i];
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (unsigned long long)(darr1[i] + darr2[i] + darr3[i]);
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += (unsigned long long)farr[i] + iarr[i];
    }
    
    for (int i = 0; i < 4096; i++) {
        checksum += carr[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    printf("Volatile counter: %d\n", v_counter);
    printf("Volatile seed: %d\n", v_seed);
    
    return 0;
}

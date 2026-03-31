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

/* Volatile counter to prevent cross-kernel optimization */
static volatile int volatile_counter = 0;

/* ========== KERNEL 1: Triple-nested loop with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M], float arr3[P]) {
    int i, j, k;
    
    /* Triple nested loop with flow dependencies across dimensions */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            /* Flow dependency (RAW) with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j] + arr2[i][j];
            
            /* Flow dependency with non-unit stride and distance 2 in j dimension */
            if (j % 3 == 0) {
                arr1[i][j] += arr1[i][j-2] * 2;
            } else {
                arr1[i][j] += arr1[i][j-1] * 3;
            }
            
            /* Cross-array dependency with type conversion */
            for (k = 0; k < P; k++) {
                /* Flow dependency in k dimension with varying distance */
                if (k % 4 == 0) {
                    arr3[k] = (float)arr1[i][j] * 0.5f + arr3[k-3];
                } else {
                    arr3[k] = (float)arr1[i][j] * 0.3f + arr3[k-1];
                }
                
                /* Additional dependency chain */
                arr3[k] = arr3[k] * 1.1f - (float)(i * j);
            }
        }
    }
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_anti_dependencies(int* base_arr, int size) {
    int i;
    
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[2];
    
    /* Anti-dependencies (WAR) through aliasing pointers */
    for (i = 2; i < size-2; i++) {
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (may alias with p) */
        
        /* Complex aliasing pattern */
        if (i % 5 == 0) {
            *r = *p + *q;       /* Read from p and q, write to r */
            p = &base_arr[i];   /* Change pointer */
        } else if (i % 3 == 0) {
            q = &base_arr[i-1]; /* Change pointer */
            *p = *q * 2;        /* Anti-dependency */
        }
        
        /* Output dependency (WAW) */
        base_arr[i] = base_arr[i-1] + base_arr[i-2];
        base_arr[i+1] = base_arr[i] * 3;  /* Flow dependency */
        
        /* Memory barrier to prevent optimization */
        asm volatile("" ::: "memory");
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_restrict_pointers(double* restrict dst, 
                                      const double* restrict src1,
                                      const double* restrict src2,
                                      int len) {
    int i;
    
    /* Output dependencies (WAW) with restrict qualifiers */
    for (i = 1; i < len; i++) {
        /* Multiple writes to same location creating output dependencies */
        dst[i] = src1[i] * src2[i];
        dst[i] = dst[i] + src1[i-1];  /* Overwrite - WAW */
        
        /* Loop-carried output dependency with distance 2 */
        if (i % 7 == 0) {
            dst[i] = dst[i-2] * 0.7;
        }
        
        /* Mixed operations creating complex dependency web */
        dst[i] = (dst[i] > 100.0) ? dst[i-1] : dst[i] * 0.5;
    }
    
    /* Second loop with different stride */
    for (i = 4; i < len; i += 2) {
        /* Flow dependency with restrict pointers */
        dst[i] = dst[i-3] + dst[i-4] * src1[i/2];
        
        /* Volatile operation to create barrier */
        volatile_counter++;
    }
}

/* ========== KERNEL 4: Mixed data types and inline assembly ========== */
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
    
    /* Complex dependency chain with mixed types */
    for (i = 1; i < size-1; i++) {
        /* Type casting creating dependencies */
        int_arr[i] = (int)float_arr[i] + (int)(double_arr[i] * 10.0);
        
        /* Bitwise operations with char array */
        char_arr[i] = (char)(int_arr[i] & 0xFF) ^ char_arr[i-1];
        
        /* Union access creating ambiguous dependencies */
        u.i = int_arr[i];
        float_arr[i] = u.f * 1.5f + float_arr[i-1];
        
        /* Inline assembly memory barrier */
        asm volatile("" ::: "memory");
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&double_arr[i], &double_arr[i-4], sizeof(double));
        }
        
        /* Volatile variable creating artificial dependency */
        double_arr[i] += (double)volatile_counter * 0.01;
        
        /* More type mixing */
        u.f = float_arr[i];
        int_arr[i+1] = u.i >> 2;
    }
    
    /* Additional loop with memset creating dependencies */
    for (i = size/2; i < size; i++) {
        memset(&char_arr[i-4], int_arr[i] & 0xFF, 4);
        char_arr[i] = char_arr[i-4] + char_arr[i-8];
    }
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    int i, j;
    
    /* Allocate and initialize multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    float arr3[P];
    double arr4[1024];
    char arr5[2048];
    int arr6[1024];
    float arr7[512];
    double arr8[512];
    
    /* Initialize with pseudo-random values */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (i = 0; i < P; i++) {
        arr3[i] = (float)(lcg_rand() % 1000) * 0.1f;
    }
    
    for (i = 0; i < 1024; i++) {
        arr4[i] = (double)(lcg_rand() % 1000) * 0.01;
        arr6[i] = lcg_rand() % 1000;
    }
    
    for (i = 0; i < 2048; i++) {
        arr5[i] = (char)(lcg_rand() % 256);
    }
    
    for (i = 0; i < 512; i++) {
        arr7[i] = (float)(lcg_rand() % 1000) * 0.05f;
        arr8[i] = (double)(lcg_rand() % 1000) * 0.02;
    }
    
    /* Execute kernels with volatile operations between them */
    kernel1_flow_dependencies(arr1, arr2, arr3);
    volatile_counter += 1;
    
    kernel2_anti_dependencies(&arr1[0][0], N*M);
    asm volatile("" ::: "memory");
    
    kernel3_restrict_pointers(arr4, arr8, arr8, 512);
    volatile_counter *= 2;
    
    kernel4_mixed_types(arr5, arr6, arr7, arr8, 512);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (i = 0; i < P; i++) {
        checksum += (long long)arr3[i];
    }
    
    for (i = 0; i < 1024; i++) {
        checksum += (long long)arr4[i] + arr6[i];
    }
    
    for (i = 0; i < 2048; i++) {
        checksum += arr5[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Volatile counter: %d\n", volatile_counter);
    
    return 0;
}

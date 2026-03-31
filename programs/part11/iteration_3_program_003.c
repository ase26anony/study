/* test_ddg_coverage.c
 * Complex loop nests designed to trigger GCC's Data Dependency Graph edge creation
 * Specifically targets the edge creation block in ddg.cc lines 749-757
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_2D 256
#define SIZE_1D 1024
#define ITER 100

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[SIZE_2D][SIZE_2D], 
                                      float arr2[SIZE_2D][SIZE_2D]) {
    int i, j, k;
    
    /* Complex 3-level nest with RAW dependencies */
    for (i = 1; i < SIZE_2D - 1; i++) {
        for (j = 1; j < SIZE_2D - 2; j++) {
            /* Flow dependency with distance 1 in i dimension */
            arr1[i][j] = arr1[i-1][j+2] + arr1[i][j-1];
            
            /* Additional dependency chain with varying distance */
            for (k = 2; k < 8; k++) {
                if (j % k == 0) {
                    /* Dependency distance varies with k */
                    arr1[i][j] += arr1[i-k/2][j] * 2;
                } else {
                    arr1[i][j] += arr1[i-1][j+1] / 3;
                }
            }
            
            /* Cross-type dependency with float array */
            arr2[i][j] = (float)arr1[i][j] * 0.5f + arr2[i-1][j+1];
        }
    }
    
    /* Reverse traversal with different stride */
    for (i = SIZE_2D - 2; i > 0; i -= 2) {
        for (j = SIZE_2D - 3; j > 0; j -= 3) {
            /* Output dependency (WAW) */
            arr1[i][j] = arr1[i+1][j-1] * arr1[i][j+2];
            
            /* Anti-dependency (WAR) */
            int temp = arr1[i][j];
            arr1[i][j] = arr1[i-1][j+1] + temp;
            arr1[i][j] = temp - arr1[i][j];
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies */
__attribute__((noinline))
static void kernel2_pointer_aliasing(int* restrict base_arr, 
                                     int* alias_arr1, 
                                     int* alias_arr2) {
    int i;
    
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];  /* q aliases p+1 */
    int* r = alias_arr1;
    int* s = alias_arr2;
    
    /* Force compiler to assume aliasing */
    for (i = 1; i < SIZE_1D - 10; i++) {
        /* Complex anti-dependency chain with aliasing */
        int temp = *p;          /* Read from p */
        *q = temp + i;          /* Write to q (aliases p+1) - WAR */
        
        /* Chain continues with multiple aliases */
        *r = *q * 2;            /* Read q, write r */
        *s = *r + *p;           /* Read r and p, write s */
        
        /* Pointer arithmetic that may create overlapping accesses */
        p = &base_arr[i];
        q = &base_arr[i + (i % 5)];
        r = &alias_arr1[i * 2 % SIZE_1D];
        s = &alias_arr2[i * 3 % SIZE_1D];
        
        /* Conditional dependency distance */
        if (i % 7 == 0) {
            base_arr[i] = base_arr[i-3] + base_arr[i-5];
        } else if (i % 3 == 0) {
            base_arr[i] = base_arr[i-2] * base_arr[i-1];
        } else {
            base_arr[i] = base_arr[i-1] + 1;
        }
    }
    
    /* Additional loop with strided aliasing */
    int* ptr1 = base_arr;
    int* ptr2 = base_arr + SIZE_1D/2;
    for (i = 0; i < SIZE_1D/2 - 5; i++) {
        /* Output dependency between potentially aliasing pointers */
        ptr1[i] = ptr2[i+2] + ptr1[i+1];
        ptr2[i] = ptr1[i] * ptr2[i+3];
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies */
__attribute__((noinline))
static void kernel3_restrict_pointers(int* restrict r1, 
                                      int* restrict r2,
                                      int* restrict r3) {
    int i, j;
    
    /* Restrict pointers allow better dependency analysis */
    for (i = 0; i < SIZE_1D - 8; i++) {
        /* Pure flow dependencies with restrict */
        r1[i] = r1[i-1] + r2[i];
        r2[i] = r3[i] * r1[i];
        r3[i] = r2[i-2] - r1[i+1];
    }
    
    /* Nested loop with output dependencies */
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            int index = i * 32 + j;
            /* WAW dependency */
            r1[index] = r2[index] + r3[index];
            r1[index] = r1[index] * 2;  /* Overwrites previous value */
            
            /* Flow dependency with computed index */
            if (j > 0) {
                r2[index] = r1[index-1] + r3[index+1];
            }
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(double* darr, 
                                float* farr, 
                                char* carr,
                                volatile int* varr) {
    int i;
    union {
        int i;
        float f;
        char bytes[4];
    } converter;
    
    /* Mixed type dependency chain */
    for (i = 1; i < SIZE_1D - 4; i++) {
        /* Double to float with cast */
        farr[i] = (float)darr[i-1] * 1.5f;
        
        /* Float to int through union */
        converter.f = farr[i];
        varr[i] = converter.i + i;
        
        /* Int to char with bitwise operations */
        carr[i] = (char)((varr[i] & 0xFF) | (carr[i-1] ^ 0x55));
        
        /* Char back to double */
        darr[i] = (double)carr[i] * 0.01 + darr[i-1];
        
        /* Memory barrier every 8 iterations */
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
        
        /* Volatile access creates artificial dependency */
        *varr = *varr + 1;
    }
    
    /* Memory function dependencies */
    for (i = 0; i < SIZE_1D - 64; i += 16) {
        /* memcpy creates flow dependencies */
        memcpy(&carr[i], &carr[i+8], 8);
        
        /* memset creates output dependencies */
        memset(&farr[i/4], 0, 4 * sizeof(float));
        
        /* Mixed type pointer arithmetic */
        double* dp = (double*)&carr[i];
        float* fp = (float*)&carr[i+8];
        
        *dp = (double)*fp * 2.0;
        *fp = (float)*dp / 3.0f;
    }
}

/* Main function with initialization and checksum */
int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int (*arr1)[SIZE_2D] = malloc(SIZE_2D * SIZE_2D * sizeof(int));
    float (*arr2)[SIZE_2D] = malloc(SIZE_2D * SIZE_2D * sizeof(float));
    int* base_arr = malloc(SIZE_1D * sizeof(int));
    int* alias_arr1 = malloc(SIZE_1D * sizeof(int));
    int* alias_arr2 = malloc(SIZE_1D * sizeof(int));
    int* restrict1 = malloc(SIZE_1D * sizeof(int));
    int* restrict2 = malloc(SIZE_1D * sizeof(int));
    int* restrict3 = malloc(SIZE_1D * sizeof(int));
    double* darr = malloc(SIZE_1D * sizeof(double));
    float* farr = malloc(SIZE_1D * sizeof(float));
    char* carr = malloc(SIZE_1D * sizeof(char));
    volatile int* varr = malloc(SIZE_1D * sizeof(int));
    
    /* Initialize with LCG to avoid compile-time computation */
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (float)(lcg_rand() % 1000) * 0.1f;
        }
    }
    
    for (int i = 0; i < SIZE_1D; i++) {
        base_arr[i] = (int)lcg_rand() % 1000;
        alias_arr1[i] = (int)lcg_rand() % 1000;
        alias_arr2[i] = (int)lcg_rand() % 1000;
        restrict1[i] = (int)lcg_rand() % 1000;
        restrict2[i] = (int)lcg_rand() % 1000;
        restrict3[i] = (int)lcg_rand() % 1000;
        darr[i] = (double)(lcg_rand() % 1000) * 0.01;
        farr[i] = (float)(lcg_rand() % 1000) * 0.1f;
        carr[i] = (char)(lcg_rand() % 256);
        varr[i] = (int)lcg_rand() % 1000;
    }
    
    /* Execute kernels multiple times to ensure DDG construction */
    for (int iter = 0; iter < ITER; iter++) {
        kernel1_flow_dependencies(arr1, arr2);
        
        /* Volatile operation between kernels */
        *varr = *varr + iter;
        
        kernel2_pointer_aliasing(base_arr, alias_arr1, alias_arr2);
        
        /* Another barrier */
        asm volatile("" ::: "memory");
        
        kernel3_restrict_pointers(restrict1, restrict2, restrict3);
        
        /* Modify array contents to prevent cross-kernel optimization */
        for (int i = 0; i < 10; i++) {
            base_arr[i] = base_arr[i] ^ restrict1[i];
        }
        
        kernel4_mixed_types(darr, farr, carr, varr);
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < SIZE_2D; i++) {
        for (int j = 0; j < SIZE_2D; j++) {
            checksum += (unsigned int)arr1[i][j];
            checksum += (unsigned int)(arr2[i][j] * 1000);
        }
    }
    
    for (int i = 0; i < SIZE_1D; i++) {
        checksum += base_arr[i];
        checksum += alias_arr1[i];
        checksum += alias_arr2[i];
        checksum += restrict1[i];
        checksum += restrict2[i];
        checksum += restrict3[i];
        checksum += (unsigned long long)(darr[i] * 1000);
        checksum += (unsigned int)(farr[i] * 1000);
        checksum += (unsigned char)carr[i];
        checksum += varr[i];
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(base_arr);
    free(alias_arr1);
    free(alias_arr2);
    free(restrict1);
    free(restrict2);
    free(restrict3);
    free(darr);
    free(farr);
    free(carr);
    free((void*)varr);
    
    return 0;
}

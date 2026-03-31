/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

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
void kernel1_flow_dependencies(int arr1[N][M], int arr2[M][P]) {
    int i, j, k;
    
    /* True dependencies (RAW) across i dimension */
    for (i = 1; i < N; i++) {
        for (j = 1; j < M - 2; j++) {
            for (k = 1; k < P - 1; k++) {
                /* Flow dependency with varying distances */
                arr1[i][j] = arr1[i-1][j+2] + arr2[j][k];
                
                /* Additional dependency chain */
                arr2[j][k] = arr1[i][j-1] * 2 - arr2[j-1][k+1];
                
                /* Cross-dimensional dependency */
                if (k % 3 == 0) {
                    arr1[i-2][j] = arr1[i][j] + arr2[j][k-2];
                } else {
                    arr1[i][j] = arr1[i][j] + arr2[j][k-1];
                }
            }
        }
    }
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
void kernel2_anti_dependencies(float *arr3, float *arr4, int size) {
    float *p = &arr3[0];
    float *q = &arr3[1];  /* q aliases with p+1 */
    float *r = &arr4[0];
    float *s = &arr4[size/2]; /* Potential aliasing with arr3 */
    
    int i;
    
    /* Anti-dependencies through aliased pointers */
    for (i = 1; i < size - 1; i++) {
        float temp = *p;          /* Read from p */
        *q = temp * 2.0f;         /* Write to q (aliases p+1) - WAR */
        
        /* Complex aliasing pattern */
        if (i % 4 == 0) {
            *s = *p + *q;         /* s may alias with arr3 */
            p++;
            q = p + 1;
        } else if (i % 4 == 1) {
            *r = *q - *p;
            r++;
            s = r + 2;
        } else if (i % 4 == 2) {
            /* Output dependency (WAW) */
            *p = *q * 3.0f;
            *p = *p + 1.0f;       /* WAW on *p */
        } else {
            /* Mixed dependencies */
            temp = *q;
            *p = temp + *r;
            *q = *p - temp;       /* WAR on temp through *q */
        }
        
        /* Varying dependency distances */
        if (i % 5 == 0) {
            arr3[i] = arr3[i-3] + arr4[i-2];
        } else if (i % 5 == 1) {
            arr3[i] = arr3[i-1] * arr4[i-4];
        } else {
            arr3[i] = arr3[i-2] / (arr4[i-3] + 1.0f);
        }
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
void kernel3_output_dependencies(double *restrict d1, 
                                 double *restrict d2,
                                 double *restrict d3,
                                 int len) {
    int i, j;
    
    /* Output dependencies with restrict qualifiers */
    for (i = 2; i < len - 2; i++) {
        /* WAW on d1[i] */
        d1[i] = d2[i-1] + d3[i+1];
        d1[i] = d1[i] * 1.5;      /* Second write to same location */
        
        /* Loop-carried output dependency */
        d2[i] = d1[i-2] * d2[i-1];
        d2[i] = d2[i] + d3[i];    /* Another WAW */
        
        /* Complex dependency chain with restrict */
        for (j = 0; j < 4; j++) {
            d3[i+j] = d1[i-j] + d2[i+j];
            d3[i+j] = d3[i+j] * 0.8;  /* WAW in inner loop */
            
            /* Cross-iteration dependency */
            if (j > 0) {
                d1[i] = d3[i+j-1] + d1[i];
            }
        }
        
        /* Conditional WAW */
        if (i % 7 == 0) {
            d3[i] = d1[i-3] * d2[i-5];
            d3[i] = d3[i] / 2.0;  /* Conditional WAW */
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
void kernel4_mixed_types(char *carr, int *iarr, float *farr, 
                         double *darr, union mixed *uarr, int size) {
    union mixed {
        int i;
        float f;
        double d;
        char c[8];
    };
    
    int i;
    
    /* Type-punning through unions creates complex dependencies */
    for (i = 1; i < size - 8; i++) {
        /* Write as int, read as float */
        uarr[i].i = iarr[i-1] + v_counter;
        farr[i] = uarr[i].f * 0.5f;  /* Type-based dependency */
        
        /* Memory barrier between different type accesses */
        asm volatile("" ::: "memory");
        
        /* Write as float, read as double */
        uarr[i].f = farr[i-2] * 2.0f;
        darr[i] = (double)uarr[i].f + darr[i-1];
        
        /* Write as double, read as char array */
        uarr[i].d = darr[i-3] * 1.1;
        carr[i] = uarr[i].c[0] + carr[i-4];
        
        /* Bitwise operations creating dependencies */
        iarr[i] = (iarr[i-1] << 2) | (iarr[i-2] >> 3);
        iarr[i] = iarr[i] ^ 0x55AA55AA;
        
        /* Inline assembly with memory clobber */
        asm volatile("" ::: "memory");
        
        /* memcpy creating dependencies */
        if (i % 8 == 0) {
            memcpy(&uarr[i], &uarr[i-4], sizeof(union mixed));
            memcpy(&carr[i], &carr[i-2], 4);
        }
        
        /* memset with dependency */
        if (i % 16 == 0) {
            memset(&carr[i], iarr[i-1] & 0xFF, 4);
            carr[i+1] = carr[i] + 1;  /* Dependency on memset result */
        }
        
        /* Volatile access creating artificial dependency */
        v_counter++;
        iarr[i] = iarr[i] + v_counter;
    }
}

/* Kernel 5: Complex loop-carried dependencies with varying distances */
__attribute__((noinline))
void kernel5_varying_distances(int *arr, int *brr, int *crr, int size) {
    int i;
    
    for (i = 4; i < size; i++) {
        /* Multiple dependency distances in same loop */
        switch (i % 6) {
            case 0:
                arr[i] = arr[i-4] + brr[i-3];  /* Distance 4 */
                break;
            case 1:
                arr[i] = arr[i-1] * crr[i-2];  /* Distance 1 */
                break;
            case 2:
                arr[i] = arr[i-3] - brr[i-4];  /* Distance 3 */
                break;
            case 3:
                arr[i] = arr[i-2] | crr[i-1];  /* Distance 2 */
                break;
            case 4:
                arr[i] = arr[i-5] & brr[i-3];  /* Distance 5 */
                break;
            case 5:
                arr[i] = arr[i-1] ^ arr[i-6];  /* Distance 1 and 6 */
                break;
        }
        
        /* Nested conditional dependencies */
        if (arr[i] > 0) {
            brr[i] = brr[i-1] + 1;
            if (brr[i] % 10 == 0) {
                crr[i] = crr[i-2] * 2;
            } else {
                crr[i] = crr[i-1] / 2;
            }
        } else {
            brr[i] = brr[i-3] - 1;
            crr[i] = crr[i-4] + brr[i-2];
        }
        
        /* Pointer chasing creating dependencies */
        int *ptr = &arr[i];
        *ptr = *ptr + *(ptr - (i % 3 + 1));
    }
}

int main() {
    /* Allocate and initialize multi-dimensional arrays */
    int arr1[N][M];
    int arr2[M][P];
    float arr3[N*2];
    float arr4[N];
    double darr1[N];
    double darr2[N];
    double darr3[N];
    char carr[N*4];
    int iarr[N*2];
    float farr[N];
    double darr[N];
    
    union mixed {
        int i;
        float f;
        double d;
        char c[8];
    } uarr[N];
    
    int brr[N*2];
    int crr[N*2];
    
    int seed = 12345;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand(&seed) % 1000;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr2[i][j] = lcg_rand(&seed) % 1000;
        }
    }
    
    for (int i = 0; i < N*2; i++) {
        arr3[i] = (lcg_rand(&seed) % 1000) / 10.0f;
        arr4[i % N] = (lcg_rand(&seed) % 1000) / 10.0f;
        iarr[i] = lcg_rand(&seed) % 1000;
        brr[i] = lcg_rand(&seed) % 1000;
        crr[i] = lcg_rand(&seed) % 1000;
    }
    
    for (int i = 0; i < N; i++) {
        darr1[i] = (lcg_rand(&seed) % 1000) / 10.0;
        darr2[i] = (lcg_rand(&seed) % 1000) / 10.0;
        darr3[i] = (lcg_rand(&seed) % 1000) / 10.0;
        farr[i] = (lcg_rand(&seed) % 1000) / 10.0f;
        darr[i] = (lcg_rand(&seed) % 1000) / 10.0;
        carr[i] = lcg_rand(&seed) % 256;
        uarr[i].i = lcg_rand(&seed);
    }
    
    /* Execute kernels with volatile operations between them */
    kernel1_flow_dependencies(arr1, arr2);
    
    v_seed = lcg_rand(&seed);
    asm volatile("" ::: "memory");
    
    kernel2_anti_dependencies(arr3, arr4, N*2);
    
    v_counter++;
    asm volatile("" ::: "memory");
    
    kernel3_output_dependencies(darr1, darr2, darr3, N);
    
    v_seed = lcg_rand(&seed);
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(carr, iarr, farr, darr, uarr, N);
    
    v_counter += 2;
    asm volatile("" ::: "memory");
    
    kernel5_varying_distances(iarr, brr, crr, N*2);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            checksum += arr2[i][j];
        }
    }
    
    for (int i = 0; i < N*2; i++) {
        checksum += (unsigned long long)arr3[i];
        checksum += iarr[i];
        checksum += brr[i];
        checksum += crr[i];
    }
    
    for (int i = 0; i < N; i++) {
        checksum += (unsigned long long)darr1[i];
        checksum += (unsigned long long)darr2[i];
        checksum += (unsigned long long)darr3[i];
        checksum += (unsigned long long)farr[i];
        checksum += (unsigned long long)darr[i];
        checksum += carr[i];
        checksum += uarr[i].i;
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Volatile counter: %d\n", v_counter);
    
    return 0;
}

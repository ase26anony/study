/* test_ddg_coverage.c
 * Complex loop nests with various dependencies to trigger DDG edge creation
 * Compile with: gcc -O2 -fmodulo-sched -fdump-ddg test_ddg_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Volatile variables to prevent optimization */
volatile int volatile_barrier = 0;

/* ========== KERNEL 1: Triple-nested loop with flow dependencies ========== */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* Triple nested loop with flow dependencies across dimensions */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            for (k = 1; k < P-1; k++) {
                /* Flow (RAW) dependencies with varying distances */
                arr1[i][j] = arr1[i-1][j+1] + arr2[i][j];  /* distance 1 in i */
                arr2[i][j] = arr1[i][j-2] * 3;             /* distance 2 in j */
                
                /* Conditional loop-carried dependency */
                if ((i + j) % 4 == 0) {
                    arr1[i][j] = arr1[i-3][j] + arr2[i-1][j];  /* distance 3 */
                } else {
                    arr1[i][j] = arr1[i-1][j] + arr2[i][j-1];  /* distance 1 */
                }
                
                /* Anti-dependency (WAR) */
                int temp = arr2[i][j];
                arr2[i][j] = arr1[i][j] * 2;
                arr1[i][j] = temp + 1;
            }
        }
    }
}

/* ========== KERNEL 2: Pointer aliasing with anti-dependencies ========== */
__attribute__((noinline))
static void kernel2_pointer_aliasing(double* arr3, double* arr4, int size) {
    int i;
    
    /* Create aliasing pointers */
    double* p = &arr3[0];
    double* q = &arr3[1];  /* q aliases p+1 */
    double* r = &arr4[0];
    
    /* Loop with anti-dependencies through aliasing pointers */
    for (i = 2; i < size - 2; i++) {
        /* WAR: Read then write to overlapping locations */
        double val1 = p[i];      /* Read from p[i] */
        double val2 = q[i-1];    /* Read from p[i] (aliased) */
        p[i+1] = val1 + val2;    /* Write to p[i+1] (affects next iteration) */
        
        /* Output dependency (WAW) */
        r[i] = p[i] * 0.5;
        q[i] = r[i] * 2.0;       /* WAW on p[i+1] in next iteration */
        
        /* Complex aliasing pattern */
        if (i % 5 == 0) {
            double* alias1 = p + (i % 3);
            double* alias2 = q - (i % 2);
            *alias1 = *alias2 + r[i];
        }
    }
}

/* ========== KERNEL 3: Restrict pointers with output dependencies ========== */
__attribute__((noinline))
static void kernel3_restrict_pointers(float* restrict r1, 
                                      float* restrict r2, 
                                      float* restrict r3, 
                                      int len) {
    int i;
    
    /* restrict allows better optimization but still creates DDG edges */
    for (i = 4; i < len - 4; i++) {
        /* Output dependencies (WAW) */
        r1[i] = r2[i-2] + r3[i-1];
        r1[i] = r1[i] * 1.5f;  /* WAW on r1[i] */
        
        /* Flow dependency chain */
        r2[i] = r1[i-3] * 2.0f;  /* distance 3 */
        r3[i] = r2[i-1] + r1[i-4]; /* distance 1 and 4 */
        
        /* Memory barrier to ensure dependencies are visible */
        asm volatile("" ::: "memory");
    }
}

/* ========== KERNEL 4: Mixed data types and inline assembly ========== */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, int* i_arr, 
                                float* f_arr, double* d_arr, 
                                int size) {
    int i;
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    for (i = 1; i < size - 1; i++) {
        /* Type casting creating dependencies */
        u.i = i_arr[i-1];
        f_arr[i] = u.f * 0.5f;  /* Flow: i_arr -> f_arr */
        
        /* Bitwise operations with dependencies */
        i_arr[i] = (int)f_arr[i] ^ i_arr[i-1];
        
        /* Char array with pointer arithmetic */
        char* cp = c_arr + i;
        *cp = (char)(i_arr[i] & 0xFF);
        
        /* Double with volatile read */
        volatile double vd = d_arr[i-1];
        d_arr[i] = vd * 2.0 + (double)f_arr[i];
        
        /* Inline assembly barrier creating artificial dependency */
        asm volatile("" ::: "memory");
        
        /* Mixed type dependency chain */
        if (i % 7 == 0) {
            u.f = f_arr[i-2];
            i_arr[i] = u.i | 0x7F;
            c_arr[i] = (char)(i_arr[i] % 256);
        }
    }
}

/* ========== KERNEL 5: Strided multi-dimensional access ========== */
__attribute__((noinline))
static void kernel5_strided_access(int arr5[N][N], int stride) {
    int i, j;
    
    /* Non-unit stride access pattern */
    for (i = 2; i < N-2; i += stride) {
        for (j = 2; j < N-2; j += stride) {
            /* Strided flow dependencies */
            arr5[i][j] = arr5[i-stride][j+stride] + arr5[i][j-stride];
            
            /* Diagonal dependencies */
            arr5[i][j] += arr5[i-1][j-1] * arr5[i-2][j+1];
            
            /* Volatile operation to prevent optimization */
            volatile_barrier = arr5[i][j] % 100;
            
            /* Conditional strided dependency */
            if ((i / stride) % 3 == 0) {
                arr5[i][j] = arr5[i-2*stride][j] + arr5[i][j-3*stride];
            }
        }
    }
}

/* ========== KERNEL 6: Complex loop-carried dependencies ========== */
__attribute__((noinline))
static void kernel6_complex_dependencies(long* long_arr, int* int_arr, int size) {
    int i;
    
    for (i = 5; i < size - 5; i++) {
        /* Varying distance loop-carried dependencies */
        switch (i % 6) {
            case 0:
                long_arr[i] = long_arr[i-5] * 2;  /* distance 5 */
                break;
            case 1:
                long_arr[i] = long_arr[i-2] + int_arr[i-1];  /* distances 2 and 1 */
                break;
            case 2:
                long_arr[i] = long_arr[i-3] ^ long_arr[i-4];  /* distances 3 and 4 */
                break;
            case 3:
                int_arr[i] = (int)long_arr[i-1] + int_arr[i-6];  /* distances 1 and 6 */
                break;
            case 4:
                long_arr[i] = (long_arr[i-2] << 2) | (long_arr[i-1] & 0xF);
                break;
            default:
                int_arr[i] = int_arr[i-3] * int_arr[i-2] - int_arr[i-4];
                break;
        }
        
        /* Memory function creating dependencies */
        if (i % 32 == 0) {
            memcpy(&long_arr[i], &int_arr[i-2], sizeof(int));
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with different data types */
    int arr1[N][M];
    int arr2[N][M];
    double arr3[1024];
    double arr4[1024];
    float farray1[512];
    float farray2[512];
    float farray3[512];
    char carray[1024];
    int iarray[1024];
    float farray[1024];
    double darray[1024];
    int arr5[N][N];
    long larray[2048];
    int intarray[2048];
    
    int i, j;
    
    /* Initialize with pseudo-random values using LCG */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (i = 0; i < 1024; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) / 10.0;
        arr4[i] = (double)(lcg_rand() % 1000) / 10.0;
        carray[i] = (char)(lcg_rand() % 256);
        iarray[i] = lcg_rand() % 10000;
        farray[i] = (float)(lcg_rand() % 1000) / 5.0f;
        darray[i] = (double)(lcg_rand() % 1000) / 3.0;
    }
    
    for (i = 0; i < 512; i++) {
        farray1[i] = (float)(lcg_rand() % 1000) / 7.0f;
        farray2[i] = (float)(lcg_rand() % 1000) / 9.0f;
        farray3[i] = (float)(lcg_rand() % 1000) / 11.0f;
    }
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            arr5[i][j] = lcg_rand() % 500;
        }
    }
    
    for (i = 0; i < 2048; i++) {
        larray[i] = (long)lcg_rand() * lcg_rand();
        intarray[i] = lcg_rand() % 777;
    }
    
    /* Execute kernels with various dependency patterns */
    kernel1_flow_dependencies(arr1, arr2);
    
    /* Volatile operation between kernels */
    volatile_barrier = arr1[10][10];
    
    kernel2_pointer_aliasing(arr3, arr4, 1024);
    
    /* Memory barrier between kernels */
    asm volatile("" ::: "memory");
    
    kernel3_restrict_pointers(farray1, farray2, farray3, 512);
    
    volatile_barrier = (int)arr3[100];
    
    kernel4_mixed_types(carray, iarray, farray, darray, 1024);
    
    kernel5_strided_access(arr5, 3);  /* stride of 3 */
    
    asm volatile("" ::: "memory");
    
    kernel6_complex_dependencies(larray, intarray, 2048);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (i = 0; i < 1024; i++) {
        checksum += (long long)arr3[i] + (long long)arr4[i];
        checksum += carray[i] + iarray[i] + (long long)farray[i] + (long long)darray[i];
    }
    
    for (i = 0; i < 512; i++) {
        checksum += (long long)farray1[i] + (long long)farray2[i] + (long long)farray3[i];
    }
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            checksum += arr5[i][j];
        }
    }
    
    for (i = 0; i < 2048; i++) {
        checksum += larray[i] + intarray[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    printf("Volatile barrier value: %d\n", volatile_barrier);
    
    return 0;
}

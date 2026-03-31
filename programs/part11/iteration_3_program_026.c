#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], float arr2[N][M], double arr3[P]) {
    volatile int barrier = 0;
    
    /* Flow dependencies (RAW) across i dimension */
    for (int i = 2; i < N - 2; i++) {
        /* Flow dependencies across j dimension with varying strides */
        for (int j = 3; j < M - 3; j += 2) {
            /* Complex dependency chain with mixed operations */
            int temp = arr1[i-1][j+1] + arr1[i-2][j];
            arr1[i][j] = temp * 2 - arr1[i][j-3];
            
            /* Cross-dimensional dependency */
            if (j % 4 == 0) {
                arr1[i][j] += arr1[i-1][j/2];
            }
            
            /* Flow dependency to float array with type conversion */
            arr2[i][j] = (float)arr1[i][j] * 0.5f + arr2[i-1][j+2];
            
            /* Memory barrier to prevent optimization */
            asm volatile("" ::: "memory");
            barrier = arr1[i][j];
        }
        
        /* Loop-carried dependency to double array with varying distance */
        for (int k = 0; k < P; k++) {
            int idx = (i * k) % P;
            if (i % 3 == 0) {
                /* Distance 2 dependency */
                arr3[idx] = arr3[(idx - 2 + P) % P] * 1.5;
            } else {
                /* Distance 1 dependency */
                arr3[idx] = arr3[(idx - 1 + P) % P] * 0.75;
            }
            
            /* Anti-dependency (WAR) with type punning */
            float* fptr = (float*)&arr3[idx];
            *fptr = (float)arr1[i][M/2];
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_pointer_aliasing(int* base_arr, int size) {
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    volatile int* vp = &base_arr[size/4];
    
    /* Anti-dependencies through aliasing pointers */
    for (int i = 2; i < size - 2; i++) {
        /* Read through p */
        int val_p = p[i];
        
        /* Write through q (may alias with p[i+1]) */
        q[i-1] = val_p * 3;
        
        /* Read through r */
        int val_r = r[i/2];
        
        /* Write through p creating WAR */
        p[i] = val_r + i;
        
        /* Volatile write creates memory barrier */
        *vp = i;
        
        /* Output dependency (WAW) with potential aliasing */
        if (i % 5 == 0) {
            q[i] = p[i] * 2;
            r[i/3] = q[i] - 1;
        }
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_restrict_pointers(int* restrict r1, int* restrict r2, 
                                      int* restrict r3, int len) {
    /* Output dependencies with restrict qualifiers */
    for (int i = 4; i < len - 4; i++) {
        /* Independent chains with WAW dependencies */
        int t1 = r1[i-1] + r2[i-2];
        int t2 = r3[i-3] * 2;
        
        /* WAW dependency - multiple writes to same location */
        r1[i] = t1;
        if (i % 7 == 0) {
            r1[i] = t2;  // Overwrites previous value
        }
        
        /* Strided access pattern */
        r2[i*2 % len] = r1[i] - r3[(i+1) % len];
        r3[(i*3) % len] = r2[i] + r1[(i-2+len) % len];
        
        /* Conditional WAW with varying distances */
        if (i % 11 == 0) {
            r3[(i-5+len) % len] = r1[i] * 3;
        } else if (i % 13 == 0) {
            r3[(i-3+len) % len] = r2[i] / 2;
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* cbuf, short* sbuf, int* ibuf, 
                                float* fbuf, double* dbuf, int size) {
    union type_pun {
        int i;
        float f;
        char bytes[4];
    } u;
    
    volatile union type_pun vu;
    
    /* Complex dependency chain with mixed types */
    for (int i = 8; i < size - 8; i++) {
        /* Type punning through union */
        u.i = ibuf[i-1];
        fbuf[i] = u.f * 1.25f;
        
        /* Dependency through byte-level access */
        for (int b = 0; b < 4; b++) {
            cbuf[i*4 + b] = u.bytes[b] ^ 0x55;
        }
        
        /* Memory function creating dependencies */
        memcpy(&sbuf[i], &cbuf[i*2], sizeof(short));
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Volatile union access */
        vu.i = ibuf[i];
        vu.f = vu.f * 2.0f;
        dbuf[i] = (double)vu.f + dbuf[i-2];
        
        /* Bitwise operations with dependencies */
        ibuf[i] = (ibuf[i-1] << 3) | (ibuf[i-2] >> 5);
        ibuf[i] ^= sbuf[i/2];
        
        /* Another memory barrier */
        asm volatile("" ::: "memory");
        vu.i = ibuf[i];
    }
}

/* Helper to initialize arrays with pseudo-random values */
static void initialize_arrays(int arr1[N][M], float arr2[N][M], double arr3[P],
                              int arr4[N*M], char arr5[N*M*2],
                              short arr6[N*M], float arr7[P*2], double arr8[P]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (float)(lcg_rand() % 1000) * 0.001f;
        }
    }
    
    for (int i = 0; i < P; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) * 0.0001;
        arr7[i] = (float)(lcg_rand() % 1000) * 0.001f;
        arr8[i] = (double)(lcg_rand() % 1000) * 0.0001;
    }
    
    for (int i = 0; i < N*M; i++) {
        arr4[i] = (int)lcg_rand() % 1000;
        arr5[i] = (char)(lcg_rand() % 256);
        arr6[i] = (short)(lcg_rand() % 1000);
    }
}

/* Compute checksum to prevent dead code elimination */
static long long compute_checksum(int arr1[N][M], float arr2[N][M], double arr3[P],
                                  int arr4[N*M], char arr5[N*M*2],
                                  short arr6[N*M], float arr7[P*2], double arr8[P]) {
    long long sum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr1[i][j];
            sum += (long long)(arr2[i][j] * 1000);
        }
    }
    
    for (int i = 0; i < P; i++) {
        sum += (long long)(arr3[i] * 10000);
        sum += (long long)(arr7[i] * 1000);
        sum += (long long)(arr8[i] * 10000);
    }
    
    for (int i = 0; i < N*M; i++) {
        sum += arr4[i];
        sum += arr5[i];
        sum += arr6[i];
    }
    
    return sum;
}

int main(void) {
    /* Allocate multi-dimensional arrays */
    static int arr1[N][M];
    static float arr2[N][M];
    static double arr3[P];
    static int arr4[N*M];
    static char arr5[N*M*2];
    static short arr6[N*M];
    static float arr7[P*2];
    static double arr8[P];
    
    /* Initialize with pseudo-random values */
    initialize_arrays(arr1, arr2, arr3, arr4, arr5, arr6, arr7, arr8);
    
    /* Execute kernels with complex dependencies */
    kernel1_flow_dependencies(arr1, arr2, arr3);
    
    /* Modify array contents between kernels */
    volatile int mod = 0;
    for (int i = 0; i < 100; i++) {
        arr4[i] ^= mod;
        mod = arr4[i];
    }
    
    kernel2_pointer_aliasing(arr4, N*M);
    
    /* More modifications */
    asm volatile("" ::: "memory");
    for (int i = 0; i < P; i++) {
        arr7[i] = arr7[i] * 2.0f - 1.0f;
    }
    
    kernel3_restrict_pointers(arr4, &arr4[N*M/4], &arr4[N*M/2], N*M);
    
    /* Final modifications before last kernel */
    volatile float vf = 0.0f;
    for (int i = 0; i < 50; i++) {
        arr5[i] = (arr5[i] + i) % 256;
        vf = arr7[i];
    }
    
    kernel4_mixed_types(arr5, arr6, arr4, arr7, arr8, P*2);
    
    /* Compute and print checksum to prevent optimization */
    long long checksum = compute_checksum(arr1, arr2, arr3, arr4, arr5, arr6, arr7, arr8);
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}

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

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], float arr2[N][M]) {
    volatile int barrier = 0;
    
    /* True dependencies (RAW) across i dimension */
    for (int i = 2; i < N - 2; i++) {
        for (int j = 1; j < M - 1; j++) {
            for (int k = 0; k < P; k++) {
                /* Flow dependency with varying distances */
                if (k % 4 == 0) {
                    arr1[i][j] = arr1[i-2][j+1] + arr1[i-1][j] * 2;
                } else if (k % 4 == 1) {
                    arr1[i][j] = arr1[i-1][j+2] + arr1[i][j-1] / 3;
                } else if (k % 4 == 2) {
                    arr1[i][j] = arr1[i-2][j-1] + arr1[i-1][j+1] - 5;
                } else {
                    arr1[i][j] = arr1[i-1][j-2] + arr1[i][j+1] + 7;
                }
                
                /* Cross-type dependency */
                arr2[i][j] = (float)arr1[i][j] * 1.5f + arr2[i-1][j] * 0.8f;
                
                /* Memory barrier to prevent optimization */
                if (barrier) asm volatile("" ::: "memory");
            }
        }
    }
}

/* Kernel 2: Loop with pointer aliasing and anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_dependencies(int* arr3, double* arr4, int size) {
    /* Create aliasing pointers */
    int* p = &arr3[0];
    int* q = &arr3[1];
    int* r = &arr3[2];
    volatile double* vp = arr4;
    
    for (int i = 3; i < size - 3; i++) {
        /* Anti-dependency: read before write to same location via aliases */
        int temp = *p + *q;
        
        /* Write through different alias - creates WAR */
        *r = temp * 2;
        
        /* Update pointers to create loop-carried dependencies */
        if (i % 3 == 0) {
            p = &arr3[i-2];
            q = &arr3[i-1];
            r = &arr3[i];
        } else if (i % 3 == 1) {
            p = &arr3[i-1];
            q = &arr3[i];
            r = &arr3[i+1];
        } else {
            p = &arr3[i];
            q = &arr3[i+1];
            r = &arr3[i+2];
        }
        
        /* Mixed type operations with volatile */
        *vp = (double)(*p + *q) / 3.14159;
        vp = &arr4[i % 64];
        
        /* Inline assembly barrier */
        asm volatile("" : "+r"(temp) : : "memory");
    }
}

/* Kernel 3: Loop with restrict pointers and output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_dependencies(int* restrict r1, int* restrict r2, 
                                        float* restrict r3, int len) {
    /* Output dependencies with restrict qualification */
    for (int i = 4; i < len - 4; i++) {
        /* Multiple writes to same location - WAW */
        r1[i] = r1[i-2] + r1[i-1];
        r1[i] = r1[i] * 3 - r1[i-3];  /* Overwrites previous value */
        
        /* Chain of output dependencies */
        r2[i] = r1[i] << 2;
        r2[i] = r2[i] | 0xFF;  /* Another WAW */
        r2[i] = r2[i] ^ r2[i-1];
        
        /* Floating point with type conversion dependencies */
        r3[i] = (float)r1[i] * 0.25f + (float)r2[i] * 0.75f;
        r3[i] = r3[i-1] * 1.1f + r3[i] * 0.9f;  /* WAW with flow */
        
        /* Conditional dependency distances */
        if (i % 5 == 0) {
            r1[i] = r1[i-4] + r2[i-3];
        } else if (i % 5 == 1) {
            r1[i] = r1[i-3] * r2[i-2];
        } else if (i % 5 == 2) {
            r1[i] = r1[i-2] - r2[i-1];
        } else if (i % 5 == 3) {
            r1[i] = r1[i-1] / (r2[i] + 1);
        } else {
            r1[i] = r1[i] ^ r2[i];  /* Self-dependency */
        }
    }
}

/* Kernel 4: Mixed data type dependencies with assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* carr, short* sarr, int* iarr, 
                                float* farr, double* darr, int size) {
    union mixed {
        int i;
        float f;
        char bytes[4];
    } u;
    
    volatile int v1 = 0;
    volatile float v2 = 0.0f;
    
    for (int i = 8; i < size - 8; i++) {
        /* Complex type casting chain */
        u.i = iarr[i-1];
        u.f = u.f * 2.0f;
        carr[i] = u.bytes[0] + u.bytes[1];
        
        /* Bitwise and arithmetic mixed */
        sarr[i] = (short)((iarr[i-2] & 0xFFFF) + (iarr[i-3] >> 16));
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&iarr[i], &iarr[i-4], sizeof(int));
            memset(&carr[i], sarr[i-1] & 0xFF, 1);
        }
        
        /* Floating point dependency chain */
        farr[i] = (float)iarr[i] * 0.5f + farr[i-1] * 0.5f;
        darr[i] = (double)farr[i] * 3.14159 + darr[i-2] * 0.618;
        
        /* Volatile operations to force dependencies */
        v1 = iarr[i-1];
        v2 = farr[i-2];
        
        /* Multiple assembly barriers */
        asm volatile("" ::: "memory");
        asm volatile("" : "+m"(*iarr) : : "memory");
        
        /* Output dependency with different type */
        iarr[i] = (int)darr[i] ^ (int)farr[i];
        iarr[i] = iarr[i] + v1;  /* WAW */
    }
}

/* Complex strided access pattern */
__attribute__((noinline))
static void kernel5_strided_access(int arr5[N][M][P/2]) {
    /* Multi-dimensional strided access with dependencies */
    for (int i = 4; i < N - 4; i += 2) {
        for (int j = 3; j < M - 3; j += 3) {
            for (int k = 2; k < P/2 - 2; k += 4) {
                /* Non-unit stride access patterns */
                arr5[i][j][k] = arr5[i-2][j+1][k-1] * 2 
                              + arr5[i-1][j][k+1] / 3 
                              - arr5[i][j-1][k-2];
                
                /* Cross-iteration dependency with varying distance */
                if ((i + j + k) % 7 == 0) {
                    arr5[i][j][k] = arr5[i-3][j+2][k] + arr5[i][j-2][k+2];
                } else if ((i + j + k) % 7 == 1) {
                    arr5[i][j][k] = arr5[i-1][j+3][k-1] * arr5[i][j][k-3];
                } else if ((i + j + k) % 7 == 2) {
                    arr5[i][j][k] = arr5[i-4][j][k+1] - arr5[i][j+1][k-4];
                }
                
                /* Pointer chase dependency */
                int* ptr1 = &arr5[i][j][k];
                int* ptr2 = &arr5[i-1][j+1][k];
                int* ptr3 = &arr5[i][j-1][k+1];
                
                *ptr1 = *ptr2 + *ptr3;
                *ptr2 = *ptr1 - *ptr3;  /* WAR via pointers */
                *ptr3 = *ptr1 ^ *ptr2;  /* WAW potential */
            }
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    float arr2[N][M];
    int arr3[N*M];
    double arr4[512];
    int rarr1[1024];
    int rarr2[1024];
    float rarr3[1024];
    char carr[2048];
    short sarr[2048];
    int iarr[2048];
    float farr[2048];
    double darr[2048];
    int arr5[N][M][P/2];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (float)(lcg_rand() % 1000) * 0.001f;
        }
    }
    
    for (int i = 0; i < N*M; i++) {
        arr3[i] = (int)lcg_rand() % 1000;
    }
    
    for (int i = 0; i < 512; i++) {
        arr4[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
    
    for (int i = 0; i < 1024; i++) {
        rarr1[i] = (int)lcg_rand() % 1000;
        rarr2[i] = (int)lcg_rand() % 1000;
        rarr3[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    for (int i = 0; i < 2048; i++) {
        carr[i] = (char)(lcg_rand() % 256);
        sarr[i] = (short)(lcg_rand() % 65536);
        iarr[i] = (int)lcg_rand() % 10000;
        farr[i] = (float)(lcg_rand() % 10000) * 0.0001f;
        darr[i] = (double)(lcg_rand() % 10000) * 0.0001;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P/2; k++) {
                arr5[i][j][k] = (int)lcg_rand() % 1000;
            }
        }
    }
    
    /* Execute kernels with volatile operations between them */
    volatile int sync = 0;
    
    kernel1_flow_dependencies(arr1, arr2);
    sync = arr1[10][10];  /* Force dependency between kernels */
    
    kernel2_anti_dependencies(arr3, arr4, N*M);
    sync = arr3[100];  /* Another synchronization point */
    
    kernel3_output_dependencies(rarr1, rarr2, rarr3, 1024);
    sync = rarr1[500];
    
    kernel4_mixed_types(carr, sarr, iarr, farr, darr, 2048);
    sync = iarr[1000];
    
    kernel5_strided_access(arr5);
    sync = arr5[50][50][10];
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (unsigned)arr1[i][j];
            checksum += (unsigned)(arr2[i][j] * 1000);
        }
    }
    
    for (int i = 0; i < N*M; i++) {
        checksum += (unsigned)arr3[i];
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (unsigned)(arr4[i] * 1000);
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += rarr1[i] + rarr2[i] + (unsigned)(rarr3[i] * 1000);
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += (unsigned char)carr[i] + sarr[i] + iarr[i] 
                  + (unsigned)(farr[i] * 1000) + (unsigned)(darr[i] * 1000);
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P/2; k++) {
                checksum += arr5[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}

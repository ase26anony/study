/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64
#define ITER 5

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[M][P], int arr3[N][P]) {
    volatile int barrier = 0;
    
    /* Complex flow dependencies with varying strides */
    for (int i = 2; i < N - 2; i++) {
        for (int j = 1; j < M - 1; j++) {
            for (int k = 1; k < P - 1; k++) {
                /* RAW dependency: read after write with distance 1 in i, 2 in j */
                arr1[i][j] = arr1[i-1][j] + arr1[i][j-1] * 2 - arr1[i-2][j+1];
                
                /* Cross-array flow dependency */
                arr2[j][k] = arr1[i][j] + arr2[j-1][k] * 3;
                
                /* Multi-dimensional strided access with output dependency */
                arr3[i][k] = arr2[j][k] + arr3[i-1][k+1] + arr3[i][k-1];
                
                /* Conditional loop-carried dependency with varying distance */
                if ((i + j + k) % 4 == 0) {
                    arr1[i][j] = arr1[i-3][j+2] + arr2[j][k];
                } else if ((i + j + k) % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j-1] * arr2[j-1][k];
                }
                
                /* Memory barrier to prevent optimization */
                if (barrier) asm volatile("" ::: "memory");
            }
        }
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    /* Create potentially aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    int* s = &base_arr[size/2 + 1];
    
    volatile int* vp = (volatile int*)&base_arr[size/4];
    
    /* WAR dependencies through aliasing pointers */
    for (int i = 2; i < size - 2; i += 2) {
        int temp1 = p[i];           /* Read from p[i] */
        q[i-1] = temp1 * 2;         /* Write to q[i-1] which may alias p[i] */
        
        int temp2 = r[i/2];         /* Read from r[i/2] */
        s[i/2 - 1] = temp2 + 3;     /* Write to s[i/2-1] which may alias r[i/2] */
        
        /* Anti-dependency with pointer arithmetic */
        *(p + i) = *(q + i - 2) + *(r + i/2);
        *(q + i - 1) = *(p + i) * *(s + i/2 - 1);
        
        /* Volatile access creates artificial dependency */
        *vp = i;
        asm volatile("" ::: "memory");
    }
    
    /* Additional loop with overlapping pointer ranges */
    int* ptr1 = &base_arr[10];
    int* ptr2 = &base_arr[15];
    for (int i = 0; i < 50; i++) {
        /* WAR: Read then write to potentially overlapping memory */
        int val = ptr1[i];
        ptr2[i-5] = val + ptr1[i+1];
        
        /* Output dependency (WAW) */
        ptr1[i] = ptr2[i-5] * 2;
        ptr1[i+1] = ptr1[i] + 1;
    }
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict dbl_arr1, 
                                 double* restrict dbl_arr2,
                                 float* restrict flt_arr,
                                 int len) {
    /* Output dependencies with restrict qualification */
    for (int i = 4; i < len - 4; i++) {
        /* WAW dependency chain */
        dbl_arr1[i] = dbl_arr1[i-1] * 1.5;
        dbl_arr1[i] = dbl_arr1[i] + dbl_arr2[i-2];  /* Overwrites previous write */
        
        /* Flow dependency mixed with output dependency */
        dbl_arr2[i] = dbl_arr1[i-3] + dbl_arr2[i-1];
        dbl_arr2[i] = dbl_arr2[i] * 0.75;           /* Another WAW */
        
        /* Type conversion creating dependencies */
        flt_arr[i] = (float)dbl_arr1[i] + (float)dbl_arr2[i-1];
        flt_arr[i] = flt_arr[i] * 2.0f;             /* WAW on float array */
        
        /* Conditional WAW with different distances */
        if (i % 5 == 0) {
            dbl_arr1[i] = dbl_arr1[i-4] + dbl_arr2[i-3];
        } else if (i % 3 == 0) {
            dbl_arr1[i] = dbl_arr1[i-2] * dbl_arr2[i-1];
        }
    }
    
    /* Nested loop with restrict but complex indexing */
    for (int i = 10; i < len/2; i++) {
        for (int j = 2; j < 20; j++) {
            dbl_arr1[i*2 + j] = dbl_arr1[(i-1)*2 + j] + dbl_arr2[i*2 + j-1];
            dbl_arr2[i*2 + j] = dbl_arr1[i*2 + j-2] * 1.1;
            
            /* Inline assembly barrier every 8 iterations */
            if ((i * j) % 8 == 0) {
                asm volatile("" ::: "memory");
            }
        }
    }
}

/* Kernel 4: Mixed data types and memory operations */
__attribute__((noinline))
static void kernel4_mixed_types(char* char_arr, int* int_arr, 
                                 float* float_arr, double* double_arr,
                                 int size) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile union mixed_union vu;
    
    /* Dependencies through type punning and casts */
    for (int i = 3; i < size - 3; i++) {
        /* Flow dependency with type conversion */
        float temp_f = float_arr[i-1];
        int_arr[i] = (int)(temp_f * 100.0f);
        
        /* Output dependency with different type */
        char_arr[i] = (char)(int_arr[i] & 0xFF);
        char_arr[i] = char_arr[i-1] + 1;  /* WAW on char */
        
        /* Union access creating dependencies */
        u.i = int_arr[i-2];
        float_arr[i] = u.f * 2.0f;
        
        /* Bitwise operations with dependencies */
        int_arr[i] = (int_arr[i-1] << 2) | (int_arr[i-2] >> 1);
        int_arr[i] = int_arr[i] ^ 0xAAAAAAAA;
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&double_arr[i], &float_arr[i], sizeof(float));
            memset(&char_arr[i], int_arr[i] & 0xFF, 1);
        }
        
        /* Volatile union access */
        vu.i = i;
        asm volatile("" ::: "memory");
    }
    
    /* Loop with mixed array accesses and pointer chasing */
    int* ip = int_arr;
    char* cp = char_arr;
    for (int i = 5; i < size/2; i++) {
        /* Interleaved type dependencies */
        ip[i] = (int)cp[i*2] + ip[i-1];
        cp[i*2] = (char)(ip[i] % 256);
        
        float_arr[i] = (float)ip[i] / 255.0f;
        double_arr[i] = (double)float_arr[i] * 1.5;
        
        /* Dependency through volatile */
        vu.f = float_arr[i];
        ip[i] ^= vu.i;
    }
}

int main(void) {
    /* Allocate multi-dimensional arrays with different types */
    int arr1[N][M];
    int arr2[M][P];
    int arr3[N][P];
    
    double dbl_arr1[512];
    double dbl_arr2[512];
    float flt_arr[1024];
    
    char char_arr[2048];
    int int_arr[1024];
    float float_arr[1024];
    double double_arr[1024];
    
    /* Initialize with pseudo-random values */
    printf("Initializing arrays with pseudo-random data...\n");
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr2[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < P; j++) {
            arr3[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < 512; i++) {
        dbl_arr1[i] = (lcg_rand() % 10000) / 100.0;
        dbl_arr2[i] = (lcg_rand() % 10000) / 100.0;
    }
    
    for (int i = 0; i < 1024; i++) {
        flt_arr[i] = (lcg_rand() % 1000) / 10.0f;
        int_arr[i] = lcg_rand() % 10000;
        float_arr[i] = (lcg_rand() % 1000) / 10.0f;
        double_arr[i] = (lcg_rand() % 10000) / 100.0;
    }
    
    for (int i = 0; i < 2048; i++) {
        char_arr[i] = lcg_rand() % 256;
    }
    
    /* Execute kernels multiple times to ensure DDG construction */
    printf("Executing dependency kernels...\n");
    
    for (int iter = 0; iter < ITER; iter++) {
        /* Modify array contents between kernels using volatile */
        volatile int mod = iter;
        asm volatile("" ::: "memory");
        
        kernel1_flow_deps(arr1, arr2, arr3);
        
        /* Force dependency between kernels */
        mod = iter * 2;
        asm volatile("" ::: "memory");
        
        kernel2_anti_deps(&arr1[0][0], N*M);
        
        mod = iter * 3;
        asm volatile("" ::: "memory");
        
        kernel3_output_deps(dbl_arr1, dbl_arr2, flt_arr, 512);
        
        mod = iter * 4;
        asm volatile("" ::: "memory");
        
        kernel4_mixed_types(char_arr, int_arr, float_arr, double_arr, 1024);
    }
    
    /* Compute checksum to prevent dead code elimination */
    printf("Computing checksum...\n");
    
    long long checksum = 0;
    
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
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < P; j++) {
            checksum += arr3[i][j];
        }
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (long long)dbl_arr1[i];
        checksum += (long long)dbl_arr2[i];
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += int_arr[i];
        checksum += (long long)float_arr[i];
        checksum += (long long)double_arr[i];
        checksum += flt_arr[i];
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += char_arr[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

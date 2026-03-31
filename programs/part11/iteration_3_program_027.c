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
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M], float arr3[P]) {
    volatile int barrier = 0;
    
    /* Flow dependencies across i dimension */
    for (int i = 2; i < N - 2; i++) {
        /* Flow dependencies across j dimension */
        for (int j = 1; j < M - 1; j++) {
            /* Complex strided access with multiple dependencies */
            arr1[i][j] = arr1[i-1][j+1] + arr1[i-2][j] * 2;
            
            /* Cross-array flow dependency */
            arr2[i][j] = arr1[i][j-1] + arr2[i-1][j+2];
            
            /* Varying distance loop-carried dependency */
            if (i % 4 == 0) {
                arr1[i][j] += arr1[i-3][j] / 3;
            } else if (i % 3 == 0) {
                arr1[i][j] += arr1[i-2][j] * 2;
            } else {
                arr1[i][j] += arr1[i-1][j] + 1;
            }
        }
        
        /* Dependency with k dimension using 3D-like access pattern */
        for (int k = 0; k < P && k < i; k++) {
            /* Mixed dimension dependency */
            arr3[k] = arr3[k] * 0.9f + (float)arr1[i][k % M] * 0.1f;
            
            /* Additional flow dependency with distance */
            if (k > 2) {
                arr3[k] += arr3[k-2] * 0.5f;
            }
        }
        
        /* Memory barrier to prevent optimization */
        barrier += i;
        asm volatile("" ::: "memory");
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies (WAR) */
__attribute__((noinline))
static void kernel2_anti_deps(int* base_arr, int size) {
    volatile int barrier = 0;
    
    /* Create aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    
    /* Anti-dependencies through aliased pointers */
    for (int i = 2; i < size - 2; i++) {
        /* Read before write with aliasing */
        int temp = p[i] + q[i-1];
        
        /* WAR: Write after read with potential aliasing */
        p[i+1] = temp * 2;
        
        /* More complex aliasing pattern */
        if (i % 5 == 0) {
            r[i/2] = p[i] + r[i/2 - 1];
        }
        
        /* Output dependency (WAW) with pointer arithmetic */
        *(p + i) = *(q + i - 2) + 1;
        *(q + i) = *(p + i) * 3;
        
        /* Memory barrier */
        barrier += temp;
        asm volatile("" ::: "memory");
    }
}

/* Kernel 3: Restrict pointers with output dependencies (WAW) */
__attribute__((noinline))
static void kernel3_output_deps(double* restrict dbl_arr1, 
                                double* restrict dbl_arr2,
                                int* restrict int_arr,
                                int len) {
    volatile double barrier = 0.0;
    
    /* Output dependencies with restrict qualifier */
    for (int i = 4; i < len - 4; i++) {
        /* WAW: Multiple writes to same location */
        dbl_arr1[i] = (double)int_arr[i] * 1.5;
        dbl_arr1[i] = dbl_arr1[i] + dbl_arr2[i-1];
        
        /* Chain of output dependencies */
        dbl_arr2[i] = dbl_arr1[i] * 2.0;
        dbl_arr2[i] = dbl_arr2[i] - dbl_arr1[i-2];
        dbl_arr2[i] = dbl_arr2[i] * 0.8;
        
        /* Conditional WAW with varying distance */
        if (i % 6 == 0) {
            int_arr[i] = (int)dbl_arr1[i-3];
            int_arr[i] = int_arr[i] + int_arr[i-6] * 2;
        } else {
            int_arr[i] = (int)dbl_arr2[i-1];
            int_arr[i] = int_arr[i] | 0xFF;
        }
        
        /* Memory barrier */
        barrier += dbl_arr1[i];
        asm volatile("" ::: "memory");
    }
}

/* Kernel 4: Mixed data types with inline assembly barriers */
__attribute__((noinline))
static void kernel4_mixed_types(char* char_arr, float* float_arr, 
                                double* double_arr, int size) {
    volatile char v_char = 0;
    volatile float v_float = 0.0f;
    
    /* Union for type punning */
    union type_pun {
        int i;
        float f;
        char c[4];
    } u;
    
    /* Mixed type dependencies */
    for (int i = 8; i < size - 8; i++) {
        /* Char to float dependency with casting */
        u.i = char_arr[i] * 256 + char_arr[i-1];
        float_arr[i] = u.f * 0.1f;
        
        /* Float to double dependency */
        double_arr[i] = (double)float_arr[i-2] * 1.5;
        
        /* Double to char dependency with bitwise ops */
        int temp = (int)(double_arr[i-4] * 100.0);
        char_arr[i] = (char)((temp & 0xFF) ^ (temp >> 8));
        
        /* Memory function creating dependencies */
        if (i % 16 == 0) {
            memcpy(&char_arr[i-8], &char_arr[i], 8);
            memset(&float_arr[i-4], 0, 4 * sizeof(float));
        }
        
        /* Complex type casting chain */
        u.f = float_arr[i-1];
        char_arr[i+1] = u.c[0] + u.c[1];
        float_arr[i] = (float)(u.i % 1000) * 0.001f;
        
        /* Multiple inline assembly barriers */
        v_char += char_arr[i];
        v_float += float_arr[i];
        asm volatile("" ::: "memory");
        asm volatile("" : "=r"(v_char) : "0"(v_char));
        asm volatile("" : "=r"(v_float) : "0"(v_float));
    }
}

/* Main function with initialization and checksum */
int main(void) {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    float arr3[P];
    double dbl_arr1[512];
    double dbl_arr2[512];
    int int_arr[512];
    char char_arr[1024];
    float float_arr[1024];
    double double_arr[1024];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < P; i++) {
        arr3[i] = (float)(lcg_rand() % 1000) * 0.001f;
    }
    
    for (int i = 0; i < 512; i++) {
        dbl_arr1[i] = (double)(lcg_rand() % 1000) * 0.01;
        dbl_arr2[i] = (double)(lcg_rand() % 1000) * 0.01;
        int_arr[i] = lcg_rand() % 1000;
    }
    
    for (int i = 0; i < 1024; i++) {
        char_arr[i] = (char)(lcg_rand() % 256);
        float_arr[i] = (float)(lcg_rand() % 1000) * 0.001f;
        double_arr[i] = (double)(lcg_rand() % 1000) * 0.001;
    }
    
    /* Execute kernels with volatile operations between them */
    volatile int inter_kernel_barrier = 0;
    
    kernel1_flow_deps(arr1, arr2, arr3);
    inter_kernel_barrier++;
    asm volatile("" ::: "memory");
    
    kernel2_anti_deps(&arr1[0][0], N * M);
    inter_kernel_barrier++;
    asm volatile("" ::: "memory");
    
    kernel3_output_deps(dbl_arr1, dbl_arr2, int_arr, 512);
    inter_kernel_barrier++;
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(char_arr, float_arr, double_arr, 1024);
    inter_kernel_barrier++;
    asm volatile("" ::: "memory");
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < P; i++) {
        checksum += (long long)(arr3[i] * 1000.0f);
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (long long)(dbl_arr1[i] * 100.0) + 
                   (long long)(dbl_arr2[i] * 100.0) + 
                   int_arr[i];
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (long long)char_arr[i] + 
                   (long long)(float_arr[i] * 1000.0f) + 
                   (long long)(double_arr[i] * 1000.0);
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Barrier value: %d\n", inter_kernel_barrier);
    
    return 0;
}

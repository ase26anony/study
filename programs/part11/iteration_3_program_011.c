#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128

/* Volatile variables to prevent optimization */
volatile int v_counter = 0;
volatile float v_float = 0.0f;

/* Simple LCG for pseudo-random initialization */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_deps(int arr1[N][M], int arr2[N][M]) {
    int i, j, k;
    
    /* True dependencies (RAW) with varying distances */
    for (i = 1; i < N-1; i++) {
        for (j = 1; j < M-1; j++) {
            for (k = 1; k < P-1; k++) {
                /* Flow dependency across i dimension */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j];
                
                /* Flow dependency across j dimension with stride */
                if (j % 3 == 0) {
                    arr2[i][j] = arr1[i][j-2] * 2;
                } else {
                    arr2[i][j] = arr1[i][j-1] + 3;
                }
                
                /* Flow dependency across k simulated dimension */
                arr1[i][j] += (i * j) % 7;
            }
        }
    }
    
    /* Anti-dependency (WAR) in same loop */
    for (i = 1; i < N-1; i++) {
        int temp = arr1[i][0];
        arr1[i][0] = arr2[i][0] + 1;
        arr2[i][0] = temp;  /* WAR: Read after write of arr1[i][0] */
    }
}

/* Kernel 2: Pointer aliasing with anti-dependencies */
__attribute__((noinline))
static void kernel2_pointer_aliasing(int* base_arr, int size) {
    int *p = &base_arr[0];
    int *q = &base_arr[1];  /* q aliases p+1 */
    int *r = &base_arr[2];  /* r aliases p+2 */
    
    /* Create complex aliasing patterns */
    for (int i = 0; i < size - 10; i++) {
        /* Output dependency (WAW) */
        p[i] = q[i+1] * 2;
        
        /* Anti-dependency (WAR) with aliasing */
        int read_val = p[i+2];  /* Read from p[i+2] which equals q[i+3] */
        q[i+3] = read_val + i;  /* Write to q[i+3] which equals p[i+4] */
        
        /* Flow dependency with pointer arithmetic */
        r[i] = p[i] + q[i+1];
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    /* Additional loop with overlapping regions */
    int *alias1 = &base_arr[size/2];
    int *alias2 = &base_arr[size/2 + 1];
    
    for (int i = 0; i < size/4; i++) {
        /* WAW dependency */
        alias1[i] = alias2[i] + 1;
        alias1[i+1] = alias1[i] * 2;  /* RAW */
        alias2[i] = alias1[i+1] - 3;  /* WAR */
    }
}

/* Kernel 3: Restrict pointers and output dependencies */
__attribute__((noinline))
static void kernel3_restrict_pointers(double* restrict d1, 
                                      double* restrict d2, 
                                      double* restrict d3, 
                                      int len) {
    /* Output dependencies (WAW) with restrict */
    for (int i = 1; i < len - 1; i++) {
        d1[i] = d2[i-1] + d3[i+1];  /* RAW from d2 and d3 */
        d2[i] = d1[i] * 1.5;        /* RAW from d1 */
        d3[i] = d2[i] / 2.0;        /* RAW from d2 */
        
        /* WAW on d1 with different expressions */
        if (i % 4 == 0) {
            d1[i] = d3[i] * 3.14;
        }
    }
    
    /* Nested loop with restrict but potential dependencies */
    for (int i = 0; i < len/2; i++) {
        for (int j = 0; j < 8; j++) {
            d1[i*8 + j] = d2[i*8 + j] + (double)j;
            d2[i*8 + j] = d1[i*8 + j] - (double)i;
            
            /* Volatile operation to create barrier */
            v_float = (float)d3[i*8 + j];
            asm volatile("" ::: "memory");
        }
    }
}

/* Kernel 4: Mixed data types and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, int* i_arr, 
                                float* f_arr, double* d_arr, 
                                int size) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    /* Type-punning dependencies */
    for (int i = 0; i < size - 4; i++) {
        /* Write as int, read as float */
        u.i = i_arr[i];
        f_arr[i] = u.f * 2.0f;  /* RAW through union */
        
        /* Write as float, read as char */
        u.f = f_arr[i] + 1.0f;
        c_arr[i] = u.c[0] + (char)i;  /* RAW through union */
        
        /* Memory barrier between different types */
        asm volatile("" ::: "memory");
        
        /* Bitwise operations creating dependencies */
        i_arr[i+1] = (i_arr[i] << 2) | (i_arr[i] >> 30);
        
        /* Type casting dependencies */
        d_arr[i] = (double)f_arr[i] * 3.14159;
        f_arr[i+1] = (float)d_arr[i] / 2.71828f;
    }
    
    /* memcpy creating dependencies */
    for (int i = 0; i < size/2; i += 16) {
        memcpy(&c_arr[i+8], &c_arr[i], 8);  /* RAW through memcpy */
        memcpy(&i_arr[i/4], &c_arr[i], sizeof(int));  /* Type-punning copy */
    }
}

/* Main function with complex loop nests */
int main(void) {
    /* Allocate and initialize arrays with pseudo-random values */
    int arr1[N][M];
    int arr2[N][M];
    double arr3[1024];
    float arr4[512];
    char arr5[2048];
    int linear_arr[4096];
    
    /* Initialize with LCG to avoid compile-time computation */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)(lcg_rand() % 1000);
            arr2[i][j] = (int)(lcg_rand() % 1000);
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        arr3[i] = (double)(lcg_rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 512; i++) {
        arr4[i] = (float)(lcg_rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 2048; i++) {
        arr5[i] = (char)(lcg_rand() % 256);
    }
    
    for (int i = 0; i < 4096; i++) {
        linear_arr[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Execute kernels with volatile barriers between them */
    kernel1_flow_deps(arr1, arr2);
    
    v_counter++;
    asm volatile("" ::: "memory");
    
    kernel2_pointer_aliasing(&linear_arr[0], 4096);
    
    v_counter++;
    asm volatile("" ::: "memory");
    
    kernel3_restrict_pointers(arr3, &arr3[512], &arr3[256], 512);
    
    v_counter++;
    asm volatile("" ::: "memory");
    
    kernel4_mixed_types(arr5, linear_arr, arr4, arr3, 512);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += (unsigned)arr1[i][j];
            checksum += (unsigned)arr2[i][j];
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (unsigned long long)arr3[i];
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (unsigned)arr4[i];
    }
    
    for (int i = 0; i < 4096; i++) {
        checksum += (unsigned)linear_arr[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    return 0;
}

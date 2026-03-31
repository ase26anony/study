#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128

/* Prevent inlining to ensure separate DDG construction for each kernel */
__attribute__((noinline)) 
void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0;
    
    /* Triple-nested loop with flow dependencies across all dimensions */
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* Complex strided access with flow dependencies */
                arr1[i][j] = arr1[i-1][j+2] + arr1[i][j-1] * 2;
                arr2[i][j] = arr2[i][j] + arr1[i-2][j] - arr1[i][j-3];
                
                /* Cross-dimensional dependency */
                if (k % 3 == 0) {
                    arr1[i][j] += arr1[i-1][j] >> 1;
                }
                
                /* Memory barrier to prevent optimization */
                asm volatile("" ::: "memory");
            }
        }
    }
}

__attribute__((noinline))
void kernel2_aliasing_dependencies(int* arr, int size) {
    /* Create aliasing pointers */
    int* p = &arr[0];
    int* q = &arr[1];
    int* r = &arr[2];
    
    volatile int* vp = &arr[size/2];
    
    /* Loop with anti-dependencies (WAR) and pointer aliasing */
    for (int i = 3; i < size - 3; i++) {
        int temp = *p + *q;  /* Read from aliased locations */
        
        /* Anti-dependency: read before write to potentially aliased location */
        *r = temp * 2;
        
        /* Write to aliased pointer */
        *p = *q + i;
        
        /* Complex aliasing pattern */
        if (i % 4 == 0) {
            *q = *r - *p;
        } else if (i % 4 == 1) {
            *r = *p ^ *q;  /* Bitwise operation */
        }
        
        /* Volatile write creates memory dependency */
        *vp = i;
        
        /* Rotate pointers to create varying access patterns */
        int* tmp_ptr = p;
        p = q;
        q = r;
        r = tmp_ptr;
    }
}

__attribute__((noinline))
void kernel3_restrict_output_deps(double* restrict d1, double* restrict d2, 
                                  double* d3, int len) {
    /* Output dependencies (WAW) with restrict qualifiers */
    for (int i = 2; i < len - 2; i++) {
        /* Multiple writes to same location - output dependencies */
        d1[i] = d2[i-1] * 3.14;
        d1[i] = d1[i] + d2[i-2];  /* WAW on d1[i] */
        
        /* Loop-carried dependency with varying distance */
        if (i % 5 == 0) {
            d3[i] = d3[i-3] * 2.0;  /* Distance 3 */
        } else if (i % 5 == 1) {
            d3[i] = d3[i-1] + 1.0;  /* Distance 1 */
        } else {
            d3[i] = d3[i-2] - 0.5;  /* Distance 2 */
        }
        
        /* Complex output dependency chain */
        d2[i] = d1[i] * d3[i];
        d2[i] = d2[i] / (d3[i] + 1.0);  /* Another WAW */
    }
}

__attribute__((noinline))
void kernel4_mixed_types_asm(volatile float* farr, char* carr, 
                             union mixed_data* udata, int count) {
    union mixed_data {
        int i;
        float f;
        double d;
        char c[8];
    };
    
    /* Mixed data type dependencies */
    for (int i = 1; i < count - 1; i++) {
        /* Type casting creates data dependencies */
        int int_val = (int)farr[i];
        float float_val = (float)int_val;
        
        /* Union access creates potential aliasing */
        udata[i].f = float_val * 1.5f;
        udata[i-1].i = (int)udata[i].f;  /* Flow dependency through union */
        
        /* Char array with pointer arithmetic */
        char* cp = &carr[i * 4];
        cp[0] = (char)(udata[i].i & 0xFF);
        cp[1] = (char)((udata[i].i >> 8) & 0xFF);
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* Volatile access ensures dependency */
        farr[i] = farr[i-1] + (float)cp[0];
        
        /* Memory function with dependency */
        if (i % 8 == 0) {
            memcpy(&carr[i*4], &carr[(i-4)*4], 4);  /* Creates memory dependency */
        }
    }
}

/* Simple LCG for pseudo-random initialization */
static unsigned int seed = 123456789;
unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main() {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    double darr1[1024];
    double darr2[1024];
    double darr3[1024];
    float farr[512];
    char carr[2048];
    
    union mixed_data {
        int i;
        float f;
        double d;
        char c[8];
    } udata[256];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = lcg_rand() % 1000;
            arr2[i][j] = lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        darr1[i] = (lcg_rand() % 1000) / 10.0;
        darr2[i] = (lcg_rand() % 1000) / 10.0;
        darr3[i] = (lcg_rand() % 1000) / 10.0;
    }
    
    for (int i = 0; i < 512; i++) {
        farr[i] = (lcg_rand() % 1000) / 100.0f;
    }
    
    for (int i = 0; i < 2048; i++) {
        carr[i] = (char)(lcg_rand() % 256);
    }
    
    for (int i = 0; i < 256; i++) {
        udata[i].i = lcg_rand();
    }
    
    /* Execute kernels with volatile barriers between them */
    volatile int sync = 0;
    
    kernel1_flow_dependencies(arr1, arr2);
    sync = 1;
    
    kernel2_aliasing_dependencies(&arr1[0][0], N*M);
    sync = 2;
    
    kernel3_restrict_output_deps(darr1, darr2, darr3, 1024);
    sync = 3;
    
    kernel4_mixed_types_asm(farr, carr, udata, 256);
    sync = 4;
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j] + arr2[i][j];
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (long long)(darr1[i] + darr2[i] + darr3[i]);
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (long long)farr[i];
    }
    
    for (int i = 0; i < 2048; i++) {
        checksum += carr[i];
    }
    
    for (int i = 0; i < 256; i++) {
        checksum += udata[i].i;
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Sync value: %d\n", sync);
    
    return 0;
}

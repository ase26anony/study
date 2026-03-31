/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 128
#define P 64
#define ITER 10

/* Prevent inlining to ensure separate DDG construction for each kernel */
__attribute__((noinline)) 
void kernel1_flow_dependencies(int arr1[N][M], float arr2[M][P], double arr3[P]) {
    volatile int counter = 0;
    
    /* Triple-nested loop with flow dependencies across all dimensions */
    for (int i = 2; i < N-2; i++) {
        for (int j = 1; j < M-1; j += 2) {  /* Non-unit stride */
            for (int k = 0; k < P; k++) {
                /* Complex flow dependencies with varying distances */
                if ((i + j + k) % 3 == 0) {
                    /* Flow dependency with distance 2 in i dimension */
                    arr1[i][j] = arr1[i-2][j] + arr1[i][(j+1) % M] * 2;
                } else if ((i + j + k) % 5 == 0) {
                    /* Flow dependency with distance 1 in j dimension */
                    arr1[i][j] = arr1[i][j-1] + arr1[(i+1) % N][j] / 3;
                } else {
                    /* Flow dependency across multiple arrays */
                    arr1[i][j] = (int)(arr2[j][k % P] * 1.5) + arr1[i-1][(j+2) % M];
                }
                
                /* Cross-dimensional dependency */
                arr2[j][k % P] = arr1[i][j] * 0.5f + arr2[(j+1) % M][k % P];
                
                /* Loop-carried dependency in innermost loop */
                if (k >= 2) {
                    arr3[k % P] = arr3[(k-2) % P] * 1.1 + (double)arr1[i][j];
                }
                
                /* Memory barrier to prevent optimization */
                asm volatile("" ::: "memory");
                counter++;
            }
        }
    }
}

__attribute__((noinline))
void kernel2_pointer_aliasing(int* base_arr, int size) {
    /* Create potentially aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];
    int* r = &base_arr[size/2];
    volatile int* volatile vp = &base_arr[size/4];  /* Volatile pointer */
    
    /* Anti-dependencies (WAR) with pointer aliasing */
    for (int i = 2; i < size-2; i++) {
        int temp = *p;           /* Read from p */
        *q = temp + i;           /* Write to q (may alias with p) */
        
        /* Output dependency (WAW) */
        *r = *r * 2;
        if (i % 4 == 0) {
            p = &base_arr[i];    /* Change pointer target */
            *p = *q + *r;        /* Write to new p location */
        }
        
        /* Read through volatile pointer creates artificial dependency */
        int volatile_read = *vp;
        base_arr[i] = base_arr[i-1] + volatile_read + base_arr[i-2];
        
        /* Pointer arithmetic with potential aliasing */
        int* alias1 = &base_arr[i % size];
        int* alias2 = &base_arr[(i * 7) % size];
        *alias1 = *alias2 + 1;
        *alias2 = *alias1 - 1;  /* Anti-dependency */
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
}

__attribute__((noinline))
void kernel3_restrict_pointers(double* restrict dr1, double* restrict dr2, 
                               double* restrict dr3, int len) {
    /* Output dependencies (WAW) with restrict pointers */
    for (int i = 1; i < len; i++) {
        /* Chain of output dependencies */
        dr1[i] = dr1[i-1] * 1.5;
        dr2[i] = dr1[i] + dr2[i-1];  /* Flow dependency across restrict pointers */
        dr3[i] = dr2[i] * dr3[i-1];
        
        /* Conditional output dependency with varying distance */
        if (i % 3 == 0 && i >= 3) {
            dr1[i] = dr1[i-3] * 2.0;  /* Distance 3 */
        } else if (i % 5 == 0 && i >= 5) {
            dr2[i] = dr2[i-5] * 3.0;  /* Distance 5 */
        }
        
        /* Mixed operations creating dependencies */
        dr1[i] = dr1[i] + (double)((int)dr2[i] ^ (int)dr3[i]);
    }
    
    /* Second loop with different stride */
    for (int i = 4; i < len; i += 2) {
        /* Strided access pattern */
        dr1[i] = dr1[i-4] + dr2[i-2] + dr3[i-1];
        dr2[i] = dr1[i] * dr2[i-1];
    }
}

__attribute__((noinline))
void kernel4_mixed_types(char* cbuf, int* ibuf, float* fbuf, 
                         double* dbuf, int size) {
    union mixed_data {
        int i;
        float f;
        char bytes[4];
    } u;
    
    volatile union mixed_data vu;
    
    /* Mixed data type dependencies */
    for (int i = 1; i < size; i++) {
        /* Type casting creating dependencies */
        u.i = ibuf[i-1];
        fbuf[i] = u.f * 1.1f;  /* int -> float dependency */
        
        /* Bitwise operations with type punning */
        u.f = fbuf[i];
        ibuf[i] = u.i ^ 0x00FF00FF;  /* float -> int dependency */
        
        /* Char array with byte-wise dependencies */
        cbuf[i] = (char)(ibuf[i] & 0xFF) + cbuf[i-1];
        
        /* Double with mixed operations */
        dbuf[i] = (double)fbuf[i] * (double)ibuf[i] + dbuf[i-1];
        
        /* Memory function creating dependencies */
        if (i % 8 == 0) {
            memcpy(&cbuf[i-7], &cbuf[i-8], 8);  /* Creates flow dependencies */
        }
        
        /* Volatile union access */
        vu.i = ibuf[i];
        fbuf[i] = vu.f + 1.0f;
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Additional loop with memset creating output dependencies */
    for (int i = 0; i < size; i += 16) {
        memset(&cbuf[i], i & 0xFF, 16);
        ibuf[i/4] = (int)cbuf[i] * 2;  /* Dependency across memset */
    }
}

/* Simple PRNG for initialization to avoid compile-time computation */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main() {
    /* Allocate multi-dimensional arrays with different sizes */
    int arr1[N][M];
    float arr2[M][P];
    double arr3[P];
    int base_arr[N * M];
    double dr1[P], dr2[P], dr3[P];
    char cbuf[P * 4];
    int ibuf[P];
    float fbuf[P];
    double dbuf[P];
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = simple_rand() % 1000;
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr2[i][j] = (float)(simple_rand() % 1000) / 10.0f;
        }
    }
    
    for (int i = 0; i < P; i++) {
        arr3[i] = (double)(simple_rand() % 1000) / 100.0;
        dr1[i] = (double)(simple_rand() % 1000) / 50.0;
        dr2[i] = (double)(simple_rand() % 1000) / 50.0;
        dr3[i] = (double)(simple_rand() % 1000) / 50.0;
        ibuf[i] = simple_rand() % 1000;
        fbuf[i] = (float)(simple_rand() % 1000) / 10.0f;
        dbuf[i] = (double)(simple_rand() % 1000) / 100.0;
    }
    
    for (int i = 0; i < P * 4; i++) {
        cbuf[i] = (char)(simple_rand() % 256);
    }
    
    for (int i = 0; i < N * M; i++) {
        base_arr[i] = simple_rand() % 1000;
    }
    
    /* Execute kernels multiple times to ensure DDG construction */
    for (int iter = 0; iter < ITER; iter++) {
        kernel1_flow_dependencies(arr1, arr2, arr3);
        
        /* Volatile operation between kernels to prevent cross-kernel optimization */
        volatile int barrier = iter;
        asm volatile("" ::: "memory");
        
        kernel2_pointer_aliasing(base_arr, N * M);
        
        barrier = iter * 2;
        asm volatile("" ::: "memory");
        
        kernel3_restrict_pointers(dr1, dr2, dr3, P);
        
        barrier = iter * 3;
        asm volatile("" ::: "memory");
        
        kernel4_mixed_types(cbuf, ibuf, fbuf, dbuf, P);
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
        }
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            checksum += (long long)arr2[i][j];
        }
    }
    
    for (int i = 0; i < P; i++) {
        checksum += (long long)arr3[i];
        checksum += (long long)dr1[i];
        checksum += (long long)dr2[i];
        checksum += (long long)dr3[i];
        checksum += ibuf[i];
        checksum += (long long)fbuf[i];
        checksum += (long long)dbuf[i];
    }
    
    for (int i = 0; i < P * 4; i++) {
        checksum += cbuf[i];
    }
    
    for (int i = 0; i < N * M; i++) {
        checksum += base_arr[i];
    }
    
    printf("Final checksum: %lld\n", checksum);
    return 0;
}

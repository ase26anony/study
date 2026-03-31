/* test_ddg_coverage.c - Complex dependency patterns to exercise GCC's DDG edge creation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 256
#define M 256
#define P 128
#define ITER 10

/* Pseudo-random generator to avoid compile-time computation */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Kernel 1: Triple-nested loop with flow dependencies across all dimensions */
__attribute__((noinline))
static void kernel1_flow_dependencies(int arr1[N][M], int arr2[N][M]) {
    volatile int barrier = 0; /* Prevent optimization */
    
    for (int i = 1; i < N-1; i++) {
        for (int j = 1; j < M-1; j++) {
            for (int k = 1; k < P-1; k++) {
                /* Complex flow dependencies with varying distances */
                arr1[i][j] = arr1[i-1][j+2] + arr2[i][j];  /* RAW across i dimension */
                arr2[i][j] = arr1[i][j-1] * 2 - arr2[i-2][j]; /* RAW across j and i dimensions */
                
                /* Conditional loop-carried dependency with varying distance */
                if ((i + j) % 3 == 0) {
                    arr1[i][j] = arr1[i-2][j] + arr2[i][j];  /* Distance 2 in i */
                } else if ((i + j) % 5 == 0) {
                    arr1[i][j] = arr1[i-1][j+1] * arr2[i][j]; /* Distance 1 in i, -1 in j */
                } else {
                    arr1[i][j] = arr1[i][j-3] / (arr2[i][j] + 1); /* Distance 3 in j */
                }
                
                /* Anti-dependency (WAR) */
                int temp = arr2[i][j];  /* Read */
                arr2[i][j] = temp + i + j;  /* Write after read */
                
                /* Output dependency (WAW) */
                arr1[i][j] = arr1[i][j] * 2;  /* Multiple writes to same location */
                barrier = arr1[i][j];  /* Use volatile to prevent reordering */
            }
        }
    }
    
    /* Memory barrier to ensure dependencies are preserved */
    asm volatile("" ::: "memory");
}

/* Kernel 2: Pointer aliasing with anti-dependencies */
__attribute__((noinline))
static void kernel2_pointer_aliasing(int* base_arr, int size) {
    /* Create potentially aliasing pointers */
    int* p = &base_arr[0];
    int* q = &base_arr[1];  /* q overlaps with p[1] */
    int* r = &base_arr[size/2];
    int* s = &base_arr[size/2 + 1];
    
    volatile int* vp = p;  /* Volatile pointer */
    
    for (int i = 0; i < size - 10; i++) {
        /* WAR (anti-dependency) through aliasing pointers */
        int read_val = *q;          /* Read from q */
        *p = read_val + i;          /* Write to p (may alias with q's next location) */
        
        /* WAW (output dependency) with pointer arithmetic */
        *(p + 1) = *r * 2;          /* Write to p[1] */
        *q = *(p + 1) + 3;          /* Write to q (may be same as p[1]) */
        
        /* Complex aliasing pattern */
        if (i % 4 == 0) {
            *r = *s + *p;           /* Read from s and p, write to r */
            *s = *r - *q;           /* Read from r and q, write to s */
        } else {
            *r = *r * *q;           /* Read/write to r, read from q */
            *s = *s / (*p + 1);     /* Read/write to s, read from p */
        }
        
        /* Use volatile to force memory ordering */
        *vp = *p + *q;
        
        /* Pointer increment with wrap-around */
        p = &base_arr[(i * 7) % size];
        q = &base_arr[(i * 11) % size];
        r = &base_arr[(i * 13) % size];
        s = &base_arr[(i * 17) % size];
    }
    
    asm volatile("" ::: "memory");
}

/* Kernel 3: Restrict pointers with output dependencies */
__attribute__((noinline))
static void kernel3_restrict_pointers(double* restrict dst, 
                                      const double* restrict src1,
                                      const double* restrict src2,
                                      int len) {
    /* restrict qualifier tells compiler pointers don't alias */
    /* This allows different dependency analysis */
    
    for (int i = 2; i < len - 2; i++) {
        /* WAW (output dependencies) on dst */
        dst[i] = src1[i-2] * src2[i+2];
        dst[i] = dst[i] + src1[i-1] * src2[i+1];  /* Second write to dst[i] */
        
        /* Flow dependencies with restrict */
        double temp = dst[i-1];  /* Read from previous iteration */
        dst[i] = dst[i] * temp + src1[i] / src2[i];
        
        /* Conditional WAW */
        if (i % 7 == 0) {
            dst[i] = src1[i] - src2[i];  /* Another write to same location */
        }
        
        /* Strided access pattern */
        dst[i*2 % len] = dst[i] * 0.5;
        dst[i*3 % len] = dst[i*2 % len] + 1.0;
    }
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
}

/* Kernel 4: Mixed data types and inline assembly */
__attribute__((noinline))
static void kernel4_mixed_types(char* c_arr, int* i_arr, float* f_arr, 
                                double* d_arr, int size) {
    union mixed_union {
        int i;
        float f;
        char c[4];
    } u;
    
    volatile float vf = 0.0f;
    
    for (int i = 1; i < size - 1; i++) {
        /* Type casting creating dependencies */
        int int_val = i_arr[i];
        float float_val = (float)int_val;
        
        /* Dependencies through different types */
        f_arr[i] = float_val * 1.5f;
        i_arr[i] = (int)f_arr[i-1];  /* Flow dependency with type conversion */
        
        /* Union access creating aliasing */
        u.i = i_arr[i];
        c_arr[i] = u.c[0];  /* Access same memory as different type */
        
        /* Bitwise operations */
        i_arr[i] = i_arr[i] ^ (i_arr[i-1] << 2);  /* Flow dependency with bit ops */
        
        /* Mixed precision calculations */
        d_arr[i] = (double)f_arr[i] * 2.0 + (double)i_arr[i] / 256.0;
        f_arr[i] = (float)d_arr[i-1] * 0.25f;  /* Flow dependency across types */
        
        /* Inline assembly memory barrier */
        asm volatile("" ::: "memory");
        
        /* Volatile force dependency */
        vf = f_arr[i];
        f_arr[i] = vf + 1.0f;
        
        /* Memory function creating dependencies */
        if (i % 32 == 0) {
            memcpy(&c_arr[i], &c_arr[i-16], 16);  /* Flow dependency via memcpy */
        }
    }
    
    /* Final memory barrier */
    asm volatile("" ::: "memory");
}

/* Initialize arrays with pseudo-random values */
static void initialize_arrays(int arr1[N][M], int arr2[N][M], 
                             double d_arr1[1024], double d_arr2[1024],
                             float f_arr[512], char c_arr[1024]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr1[i][j] = (int)lcg_rand() % 1000;
            arr2[i][j] = (int)lcg_rand() % 1000;
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        d_arr1[i] = (double)(lcg_rand() % 10000) / 100.0;
        d_arr2[i] = (double)(lcg_rand() % 10000) / 100.0;
        c_arr[i] = (char)(lcg_rand() % 256);
    }
    
    for (int i = 0; i < 512; i++) {
        f_arr[i] = (float)(lcg_rand() % 10000) / 100.0f;
    }
}

/* Compute checksum to prevent dead code elimination */
static long long compute_checksum(int arr1[N][M], int arr2[N][M],
                                 double d_arr1[1024], double d_arr2[1024],
                                 float f_arr[512], char c_arr[1024]) {
    long long checksum = 0;
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += arr1[i][j];
            checksum += arr2[i][j];
        }
    }
    
    for (int i = 0; i < 1024; i++) {
        checksum += (long long)d_arr1[i];
        checksum += (long long)d_arr2[i];
        checksum += (int)c_arr[i];
    }
    
    for (int i = 0; i < 512; i++) {
        checksum += (long long)f_arr[i];
    }
    
    return checksum;
}

int main(void) {
    /* Allocate multi-dimensional arrays */
    int arr1[N][M];
    int arr2[N][M];
    double d_arr1[1024];
    double d_arr2[1024];
    float f_arr[512];
    char c_arr[1024];
    int linear_arr[2048];
    
    /* Initialize with pseudo-random values */
    initialize_arrays(arr1, arr2, d_arr1, d_arr2, f_arr, c_arr);
    
    /* Initialize linear array for pointer aliasing kernel */
    for (int i = 0; i < 2048; i++) {
        linear_arr[i] = (int)lcg_rand() % 1000;
    }
    
    /* Execute kernels multiple times to increase coverage chances */
    for (int iter = 0; iter < ITER; iter++) {
        /* Kernel 1: Complex nested loops with all dependency types */
        kernel1_flow_dependencies(arr1, arr2);
        
        /* Modify array contents between kernels using volatile */
        volatile int mod = iter;
        for (int i = 0; i < 10; i++) {
            arr1[mod % N][i % M] = mod + i;
        }
        
        /* Kernel 2: Pointer aliasing */
        kernel2_pointer_aliasing(linear_arr, 2048);
        
        /* More volatile modifications */
        asm volatile("" ::: "memory");
        mod = mod * 1103515245 + 12345;
        
        /* Kernel 3: Restrict pointers */
        kernel3_restrict_pointers(d_arr1, d_arr2, d_arr2, 1024);
        
        /* Kernel 4: Mixed data types */
        kernel4_mixed_types(c_arr, linear_arr, f_arr, d_arr2, 512);
        
        /* Shuffle data between iterations */
        if (iter % 2 == 0) {
            memcpy(d_arr1, d_arr2, sizeof(double) * 512);
        }
    }
    
    /* Compute and print checksum to prevent dead code elimination */
    long long checksum = compute_checksum(arr1, arr2, d_arr1, d_arr2, f_arr, c_arr);
    printf("Final checksum: %lld\n", checksum);
    
    return 0;
}

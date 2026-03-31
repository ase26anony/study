/* reload_stress.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100

/* Global volatile variables to force memory operations */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile float global_volatile_float = 2.71828f;

/* Complex structure for varied access patterns */
struct MixedData {
    int i;
    long l;
    float f;
    double d;
    int arr[8];
};

/* Helper with many arguments to stress calling convention */
__attribute__((noinline))
static double many_args_function(
    int a1, long a2, float a3, double a4,
    int a5, long a6, float a7, double a8,
    int a9, long a10, float a11, double a12,
    int a13, long a14, float a15, double a16,
    volatile int* a17, volatile double* a18
) {
    /* Complex computation mixing all arguments */
    double sum = (double)a1 + (double)a2 + (double)a3 + a4;
    sum += (double)a5 + (double)a6 + (double)a7 + a8;
    sum += (double)a9 + (double)a10 + (double)a11 + a12;
    sum += (double)a13 + (double)a14 + (double)a15 + a16;
    sum += (double)(*a17) + (*a18);
    
    /* Force memory access */
    *a17 += 1;
    *a18 *= 1.0001;
    
    return sum * 0.5;
}

/* Main computation with extreme register pressure */
__attribute__((noinline, optimize("no-unroll-loops")))
static void compute_heavy(struct MixedData* input, struct MixedData* output, int size) {
    /* Declare MANY local variables to create register pressure */
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    volatile int vi1, vi2, vi3;
    volatile double vd1, vd2, vd3;
    
    /* Additional pointer/index variables */
    int* ptr1, *ptr2, *ptr3;
    double* dptr1, *dptr2;
    int idx1, idx2, idx3, idx4, idx5;
    
    /* Initialize volatile locals */
    vi1 = global_volatile_int;
    vi2 = vi1 * 2;
    vi3 = vi2 + 1;
    vd1 = global_volatile_double;
    vd2 = vd1 * 2.0;
    vd3 = vd2 + 1.0;
    
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Complex loop with data dependencies preventing register reuse */
        for (int i = 0; i < size; i++) {
            /* Complex array indexing with multiple terms */
            idx1 = i * 3;
            idx2 = i * 5 + outer;
            idx3 = i * 7 + idx1;
            idx4 = i * 11 + idx2;
            idx5 = i * 13 + idx3;
            
            /* Load with complex addressing modes */
            i1 = input[i].arr[idx1 % 8];
            i2 = input[i].arr[idx2 % 8];
            i3 = input[i].arr[idx3 % 8];
            i4 = input[i].arr[idx4 % 8];
            i5 = input[i].arr[idx5 % 8];
            
            l1 = input[i].l + idx1;
            l2 = input[i].l + idx2;
            l3 = input[i].l + idx3;
            
            f1 = input[i].f * (float)idx1;
            f2 = input[i].f * (float)idx2;
            f3 = input[i].f * (float)idx3;
            
            d1 = input[i].d * (double)idx1;
            d2 = input[i].d * (double)idx2;
            d3 = input[i].d * (double)idx3;
            
            /* Long chain of mixed-type computations */
            f4 = (float)i1 * (float)d1 + f1;
            d4 = (double)i2 * d2 + (double)f2;
            i6 = (int)f3 + (int)d3 + i3;
            l4 = (long)f4 * (long)d4 + l1;
            
            f5 = f4 * 1.5f + (float)l2;
            d5 = d4 * 1.5 + (double)l3;
            i7 = i4 * 2 + (int)f5;
            l5 = l4 / 3 + (long)d5;
            
            f6 = sqrtf(fabsf(f5));
            d6 = sqrt(fabs(d5));
            i8 = i5 ^ i6 ^ i7;
            l6 = l5 * 7 - 13;
            
            /* More mixed operations */
            f7 = (float)i8 / (float)(l6 + 1);
            d7 = (double)i7 / (double)(l5 + 1);
            i9 = (int)(f6 * 100.0f) + (int)(d6 * 100.0);
            l7 = (long)(f7 * 1000.0f) + (long)(d7 * 1000.0);
            
            f8 = sinf(f7) + cosf((float)d7);
            d8 = sin(d7) + cos((double)f7);
            i10 = (i9 << 3) | (i8 & 0xFF);
            l8 = (l7 << 2) | (l6 & 0xFFFF);
            
            /* Inline assembly that clobbers many registers */
            __asm__ volatile (
#if defined(__aarch64__)
                /* Clobber many ARM registers */
                "mov x0, %0\n\t"
                "mov x1, %1\n\t"
                "mov x2, %2\n\t"
                "mov x3, %3\n\t"
                "mov x4, %4\n\t"
                "mov x5, %5\n\t"
                "mov x6, %6\n\t"
                "mov x7, %7\n\t"
                "mov v0.d[0], x0\n\t"
                "mov v1.d[0], x1\n\t"
                "mov v2.d[0], x2\n\t"
                "mov v3.d[0], x3\n\t"
                :
                : "r" (i1), "r" (i2), "r" (l1), "r" (l2),
                  "r" (f1), "r" (f2), "r" (d1), "r" (d2)
                : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                  "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                  "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
                  "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
                  "memory"
#elif defined(__x86_64__)
                /* Clobber many x86_64 registers */
                "mov %0, %%rax\n\t"
                "mov %1, %%rbx\n\t"
                "mov %2, %%rcx\n\t"
                "mov %3, %%rdx\n\t"
                "mov %4, %%rsi\n\t"
                "mov %5, %%rdi\n\t"
                "movq %6, %%xmm0\n\t"
                "movq %7, %%xmm1\n\t"
                :
                : "r" (i1), "r" (i2), "r" (l1), "r" (l2),
                  "r" (f1), "r" (f2), "m" (d1), "m" (d2)
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                  "xmm12", "xmm13", "xmm14", "xmm15",
                  "memory"
#else
                /* Generic clobber */
                "nop\n\t"
                :
                :
                : "memory"
#endif
            );
            
            /* Continue computation after assembly */
            f9 = f8 * 2.0f + (float)i10;
            d9 = d8 * 2.0 + (double)l8;
            i10 = i10 + (int)f9;
            l9 = l8 + (long)d9;
            
            f10 = f9 / (float)(i10 + 1);
            d10 = d9 / (double)(l9 + 1);
            
            /* Call function with many arguments */
            double result = many_args_function(
                i1, l1, f1, d1,
                i2, l2, f2, d2,
                i3, l3, f3, d3,
                i4, l4, f4, d4,
                &vi1, &vd1
            );
            
            /* Use result in computation */
            f10 += (float)result;
            d10 += result;
            
            /* Complex store with addressing */
            int store_idx = (i * 17 + outer * 3) % 8;
            output[i].arr[store_idx] = i10;
            output[i].l = l9;
            output[i].f = f10;
            output[i].d = d10 + (double)global_volatile_int;
            
            /* More volatile operations */
            vi3 = vi1 + vi2;
            vd3 = vd1 + vd2;
            global_volatile_float = f10;
        }
    }
    
    /* Final volatile store to prevent optimization */
    global_volatile_int = vi3;
    global_volatile_double = vd3;
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* input = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    struct MixedData* output = (struct MixedData*)malloc(ARRAY_SIZE * sizeof(struct MixedData));
    
    if (!input || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        input[i].i = rand();
        input[i].l = (long)rand() * rand();
        input[i].f = (float)rand() / (float)RAND_MAX * 100.0f;
        input[i].d = (double)rand() / (double)RAND_MAX * 1000.0;
        
        for (int j = 0; j < 8; j++) {
            input[i].arr[j] = rand() % 1000;
        }
        
        /* Initialize output */
        output[i].i = 0;
        output[i].l = 0;
        output[i].f = 0.0f;
        output[i].d = 0.0;
        for (int j = 0; j < 8; j++) {
            output[i].arr[j] = 0;
        }
    }
    
    /* Perform heavy computation */
    compute_heavy(input, output, ARRAY_SIZE);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += output[i].i;
        checksum += output[i].l;
        checksum += (long long)output[i].f;
        checksum += (long long)output[i].d;
        for (int j = 0; j < 8; j++) {
            checksum += output[i].arr[j];
        }
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Volatile globals: %d, %f, %f\n", 
           global_volatile_int, 
           global_volatile_double,
           global_volatile_float);
    
    free(input);
    free(output);
    
    return 0;
}

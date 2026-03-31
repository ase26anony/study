/* reload_coverage.c - Program to exercise GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Packed struct to force unaligned accesses and secondary reloads */
struct __attribute__((packed)) PackedStruct {
    double d;
    int i;
    float f;
    long l;
    char c;
    short s;
};

/* Volatile variables to prevent optimization */
volatile int global_flag = 1;
volatile int *volatile volatile_ptr;

/* Function with high register pressure and complex operations */
__attribute__((noinline))
static int64_t trigger_reloads(int N, int seed) {
    /* Declare many variables to exceed available registers */
    int a = seed + 1, b = seed + 2, c = seed + 3, d = seed + 4;
    int e = seed + 5, f = seed + 6, g = seed + 7, h = seed + 8;
    int i = seed + 9, j = seed + 10, k = seed + 11, l = seed + 12;
    float fa = seed * 1.1f, fb = seed * 1.2f, fc = seed * 1.3f, fd = seed * 1.4f;
    double da = seed * 2.1, db = seed * 2.2, dc = seed * 2.3, dd = seed * 2.4;
    long la = seed * 3L, lb = seed * 4L, lc = seed * 5L, ld = seed * 6L;
    
    /* Multi-dimensional array for address reloads */
    int arr[128][128];
    
    /* Packed struct for unaligned accesses */
    struct PackedStruct ps = {.d = seed * 1.5, .i = seed, .f = seed * 1.7f, 
                              .l = seed * 7L, .c = seed & 0xFF, .s = seed & 0xFFFF};
    
    /* Initialize array with non-constant values */
    for (int idx = 0; idx < 128; idx++) {
        for (int jdx = 0; jdx < 128; jdx++) {
            arr[idx][jdx] = (idx * 7919 + jdx * 65537 + seed) & 0x7FFF;
        }
    }
    
    /* Complex nested loops with many live variables */
    for (int iter = 0; iter < N; iter++) {
        /* Create data dependencies between variables */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        f = g + h;
        g = h + a;
        h = a + b;
        
        /* Floating point chain */
        fa = fb + fc;
        fb = fc + fd;
        fc = fd + fa;
        fd = fa + fb;
        
        /* Double precision chain */
        da = db + dc;
        db = dc + dd;
        dc = dd + da;
        dd = da + db;
        
        /* Long integer chain */
        la = lb + lc;
        lb = lc + ld;
        lc = ld + la;
        ld = la + lb;
        
        /* Inline assembly with conflicting constraints to force reloads */
        /* Tied operand constraint (output tied to input) */
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(a)
            : "r"(b), "0"(c)  /* '0' means same as output operand 0 */
            : "cc"
        );
        
        /* Different register class constraints */
        asm volatile (
            "imul %0, %1\n\t"
            : "+r"(d)
            : "r"(e)
            : "cc"
        );
        
        /* Memory constraint forcing spill */
        asm volatile (
            "addl %1, %0\n\t"
            : "+m"(arr[iter % 128][0])
            : "r"(f)
            : "cc"
        );
        
        /* Complex array access with computed indices - forces address reloads */
        int idx1 = (iter * 17) % 128;
        int idx2 = (iter * 23) % 128;
        int idx3 = (iter * 37) % 128;
        int idx4 = (iter * 53) % 128;
        
        /* Swapping array elements with many live variables */
        int temp = arr[idx1][idx2];
        arr[idx1][idx2] = arr[idx3][idx4] + a;
        arr[idx3][idx4] = temp + b;
        
        /* Access packed struct through volatile pointer - may need secondary reload */
        volatile_ptr = (volatile int*)&ps.i;
        int packed_val = *volatile_ptr;
        
        /* Conditional block for optional reloads */
        if (global_flag & 1) {
            /* Use different subset of variables */
            asm volatile (
                "sub %0, %1, %2\n\t"
                : "=r"(g)
                : "r"(h), "r"(packed_val)
                : "cc"
            );
            
            /* More complex floating point in conditional path */
            double temp_d = da + db;
            asm volatile (
                "faddp %%st(1), %%st(0)\n\t"
                : "=t"(da)
                : "0"(da), "u"(temp_d)
            );
        } else {
            /* Alternative path with different variables */
            asm volatile (
                "xor %0, %1, %2\n\t"
                : "=r"(i)
                : "r"(j), "r"(k)
                : "cc"
            );
            
            fa = fb - fc;
        }
        
        /* Access packed struct members directly - unaligned access */
        ps.i = ps.i + iter;
        ps.d = ps.d * 1.01;
        
        /* More inline asm with specific register constraints (x86-64) */
        #ifdef __x86_64__
        register long r12 asm("r12") = la;
        register long r13 asm("r13") = lb;
        asm volatile (
            "add %0, %1, %2\n\t"
            : "=r"(r12)
            : "r"(r13), "0"(r12)
            : "cc"
        );
        la = r12;
        #endif
        
        /* Chain all variables together to keep them live */
        a = a + b + c + d + e + f + g + h + i + j + k + l;
        fa = fa + fb + fc + fd;
        da = da + db + dc + dd;
        la = la + lb + lc + ld + packed_val;
        
        /* Prevent loop invariant code motion */
        arr[iter % 128][(iter * 7) % 128] = a + (int)fa + (int)da + (int)la;
    }
    
    /* Compute checksum using all variables */
    int64_t checksum = 0;
    checksum += a + b + c + d + e + f + g + h + i + j + k + l;
    checksum += (int64_t)(fa * 1000) + (int64_t)(fb * 1000) + 
                (int64_t)(fc * 1000) + (int64_t)(fd * 1000);
    checksum += (int64_t)(da * 1000) + (int64_t)(db * 1000) + 
                (int64_t)(dc * 1000) + (int64_t)(dd * 1000);
    checksum += la + lb + lc + ld;
    
    /* Include array in checksum */
    for (int idx = 0; idx < 128; idx += 8) {
        for (int jdx = 0; jdx < 128; jdx += 8) {
            checksum += arr[idx][jdx];
        }
    }
    
    checksum += ps.i + (int64_t)(ps.d * 1000) + (int64_t)(ps.f * 1000) + ps.l;
    
    return checksum;
}

/* Another function to increase register pressure through calling convention */
__attribute__((noinline))
static double complex_math(double x, double y, float f1, float f2, 
                          int i1, int i2, long l1, long l2) {
    /* Many intermediate variables */
    double t1 = x * y;
    double t2 = x / (y + 1.0);
    double t3 = t1 + t2;
    double t4 = t1 - t2;
    double t5 = t3 * t4;
    
    float ft1 = f1 * f2;
    float ft2 = f1 + f2;
    float ft3 = ft1 - ft2;
    float ft4 = ft1 / (ft2 + 0.1f);
    
    int it1 = i1 * i2;
    int it2 = i1 + i2;
    int it3 = it1 - it2;
    int it4 = it1 ^ it2;
    
    long lt1 = l1 * l2;
    long lt2 = l1 + l2;
    long lt3 = l1 - l2;
    long lt4 = l1 & l2;
    
    /* Inline asm with memory operand */
    asm volatile (
        "faddp %%st(1), %%st(0)\n\t"
        : "+t"(t5)
        : "u"(t3)
    );
    
    /* Mix all types */
    return t5 + ft3 + ft4 + it3 + it4 + lt3 + lt4;
}

int main(int argc, char *argv[]) {
    /* Use command line argument for loop count */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int seed = (argc > 2) ? atoi(argv[2]) : 12345;
    
    if (N <= 0) N = 50;
    if (N > 1000) N = 1000; /* Limit for reasonable runtime */
    
    srand(seed);
    
    /* Call the reload-intensive function multiple times */
    int64_t total_checksum = 0;
    for (int repeat = 0; repeat < 3; repeat++) {
        total_checksum += trigger_reloads(N, seed + repeat * 1000);
    }
    
    /* Additional calls with different signatures */
    double math_result = 0.0;
    for (int i = 0; i < N / 10; i++) {
        math_result += complex_math(
            seed * 1.234 + i,
            seed * 0.987 - i,
            seed * 1.5f + i,
            seed * 0.5f - i,
            seed + i * 3,
            seed - i * 7,
            seed * 11L + i,
            seed * 13L - i
        );
    }
    
    /* Mix in the math result */
    total_checksum += (int64_t)(math_result * 1000);
    
    printf("Checksum: %ld\n", (long)total_checksum);
    return 0;
}

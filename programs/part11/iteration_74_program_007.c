/* Test program to trigger early rematerialization validation logic */
#include <stdio.h>
#include <stdint.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Global arrays to prevent optimization */
volatile int global_seed = 12345;
int results[100];
volatile int sink;

/* Force register pressure with many live variables */
NOINLINE int test_int_remat(int iterations) {
    /* Declare many integer variables to consume registers */
    volatile int v0 = global_seed + 1;
    volatile int v1 = global_seed + 2;
    volatile int v2 = global_seed + 3;
    volatile int v3 = global_seed + 4;
    volatile int v4 = global_seed + 5;
    volatile int v5 = global_seed + 6;
    volatile int v6 = global_seed + 7;
    volatile int v7 = global_seed + 8;
    volatile int v8 = global_seed + 9;
    volatile int v9 = global_seed + 10;
    
    int a = v0, b = v1, c = v2, d = v3, e = v4;
    int f = v5, g = v6, h = v7, i = v8, j = v9;
    
    int sum = 0;
    
    /* Complex expression with many independent computations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many live values simultaneously */
        int t1 = a + b * c;      /* Remat candidate: b * c */
        int t2 = d - e * f;      /* Remat candidate: e * f */
        int t3 = g << (h & 3);   /* Remat candidate: h & 3 */
        int t4 = i ^ j ^ t1;     /* Uses t1, keeps it live */
        int t5 = t2 | t3;        /* Uses t2, t3 */
        int t6 = t4 + t5 * a;    /* Complex expression */
        int t7 = b + c + d + e;  /* Many operands */
        int t8 = f * g * h;      /* Chain of multiplies */
        int t9 = i - j + t6;     /* Mix of values */
        int t10 = t7 ^ t8 ^ t9;  /* Final combine */
        
        /* Force all values to be live through this point */
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Modify some values to prevent complete elimination */
        a += t10 & 1;
        b += t9 & 2;
        c += t8 & 4;
        
        /* Inline assembly to clobber registers and force remat */
        asm volatile ("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5");
    }
    
    return sum;
}

NOINLINE double test_fp_remat(int iterations) {
    /* Mix float and double computations */
    volatile float fv0 = global_seed * 0.1f;
    volatile float fv1 = global_seed * 0.2f;
    volatile float fv2 = global_seed * 0.3f;
    volatile double dv0 = global_seed * 0.01;
    volatile double dv1 = global_seed * 0.02;
    volatile double dv2 = global_seed * 0.03;
    
    float f1 = fv0, f2 = fv1, f3 = fv2;
    double d1 = dv0, d2 = dv1, d3 = dv2;
    
    double sum = 0.0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Multiple independent FP computations */
        float ft1 = f1 * f2 + f3;        /* Remat candidate: f1 * f2 */
        float ft2 = f2 / f3 - f1;        /* Remat candidate: f2 / f3 */
        double dt1 = d1 * d2 + d3;       /* Remat candidate: d1 * d2 */
        double dt2 = d2 / d3 - d1;       /* Remat candidate: d2 / d3 */
        
        /* Mix types to increase register class pressure */
        double mixed1 = ft1 * dt1;       /* Requires conversion */
        double mixed2 = ft2 * dt2;       /* Another conversion */
        double mixed3 = mixed1 / mixed2; /* Division keeps values live */
        
        /* Complex expression with many live values */
        double result = (ft1 + ft2) * (dt1 - dt2) / mixed3;
        
        sum += result + mixed1 + mixed2 + mixed3;
        
        /* Modify values slightly */
        f1 += 0.1f * (iter & 1);
        d1 += 0.01 * (iter & 1);
        
        /* Clobber FP registers */
        asm volatile ("" : : : "memory", "d0", "d1", "d2", "d3", "d4", "d5");
    }
    
    return sum;
}

NOINLINE int test_addr_remat(int iterations) {
    /* Array access with complex address calculations */
    static int array[1024];
    
    /* Initialize array */
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 3 + global_seed;
    }
    
    volatile int idx_seed = global_seed;
    int idx1 = idx_seed & 1023;
    int idx2 = (idx_seed * 7) & 1023;
    int idx3 = (idx_seed * 13) & 1023;
    int idx4 = (idx_seed * 31) & 1023;
    
    int sum = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex address calculations - good remat candidates */
        int offset1 = (iter * 7) & 63;
        int offset2 = (iter * 13) & 63;
        int offset3 = (iter * 31) & 63;
        
        /* Multiple array accesses with different base calculations */
        int val1 = array[(idx1 + offset1) & 1023];  /* Remat: idx1 + offset1 */
        int val2 = array[(idx2 + offset2) & 1023];  /* Remat: idx2 + offset2 */
        int val3 = array[(idx3 + offset3) & 1023];  /* Remat: idx3 + offset3 */
        int val4 = array[(idx4 + iter) & 1023];     /* Remat: idx4 + iter */
        
        /* More complex addressing modes */
        int val5 = array[((idx1 * offset1) + idx2) & 1023];
        int val6 = array[((idx3 * offset2) + idx4) & 1023];
        int val7 = array[((offset1 * offset3) + iter) & 1023];
        
        /* Use all values in computation */
        int result = val1 * val2 + val3 * val4 - val5 * val6 + val7;
        
        sum += result;
        
        /* Update indices to prevent constant propagation */
        idx1 = (idx1 * 3 + 1) & 1023;
        idx2 = (idx2 * 5 + 1) & 1023;
        
        /* Clobber address registers */
        asm volatile ("" : : : "memory", "r6", "r7", "r8", "r9", "r10", "r11");
    }
    
    return sum;
}

/* Function with inline assembly that forces specific register allocation */
NOINLINE int test_asm_remat(int iterations) {
    volatile int a = global_seed + 100;
    volatile int b = global_seed + 200;
    volatile int c = global_seed + 300;
    
    int x = a, y = b, z = c;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly that uses specific registers */
        int tmp1, tmp2, tmp3;
        
        asm volatile (
            "mov %[t1], %[x]\n\t"
            "add %[t1], %[t1], %[y]\n\t"
            "mov %[t2], %[z]\n\t"
            "mul %[t2], %[t2], %[x]\n\t"
            "mov %[t3], %[y]\n\t"
            "sub %[t3], %[t3], %[z]"
            : [t1] "=&r" (tmp1), [t2] "=&r" (tmp2), [t3] "=&r" (tmp3)
            : [x] "r" (x), [y] "r" (y), [z] "r" (z)
            : "cc", "memory"
        );
        
        /* Use the results in further computations */
        int r1 = tmp1 * tmp2;
        int r2 = tmp2 + tmp3;
        int r3 = tmp3 - tmp1;
        
        sum += r1 + r2 + r3;
        
        /* Modify values */
        x += (i & 1);
        y += (i & 2);
        z += (i & 4);
    }
    
    return sum;
}

int main() {
    int result_idx = 0;
    
    printf("Starting early rematerialization test...\n");
    
    /* Run tests with different iteration counts to vary register pressure */
    results[result_idx++] = test_int_remat(1000);
    results[result_idx++] = (int)test_fp_remat(500);
    results[result_idx++] = test_addr_remat(800);
    results[result_idx++] = test_asm_remat(600);
    
    /* Additional variant with more aggressive parameters */
    results[result_idx++] = test_int_remat(2000);
    results[result_idx++] = (int)test_fp_remat(1000);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}

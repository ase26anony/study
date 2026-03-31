/* early-remat-test.c
 * Test program to trigger GCC's early rematerialization pass,
 * specifically targeting the validation logic in early-remat.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global arrays to store results and prevent optimization */
volatile int global_seed = 12345;
int results[100];
volatile int sink;

/* Non-inlineable functions to prevent optimization */
__attribute__((noinline, noipa)) int get_value(int idx) {
    return global_seed + idx * 1103515245;
}

__attribute__((noinline, noipa)) float get_float(int idx) {
    return (global_seed + idx) * 0.001f;
}

__attribute__((noinline, noipa)) double get_double(int idx) {
    return (global_seed - idx) * 0.0001;
}

/* Integer-intensive test with many live variables */
int test_int_remat(int iterations) {
    /* Declare many integer variables to create register pressure */
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    int c0, c1, c2, c3, c4, c5, c6, c7, c8, c9;
    int d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    
    volatile int vseed = global_seed;
    
    /* Initialize with non-trivial values */
    a0 = vseed + 1; a1 = vseed + 2; a2 = vseed + 3; a3 = vseed + 4; a4 = vseed + 5;
    a5 = vseed + 6; a6 = vseed + 7; a7 = vseed + 8; a8 = vseed + 9; a9 = vseed + 10;
    
    b0 = get_value(0); b1 = get_value(1); b2 = get_value(2); b3 = get_value(3); b4 = get_value(4);
    b5 = get_value(5); b6 = get_value(6); b7 = get_value(7); b8 = get_value(8); b9 = get_value(9);
    
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create many independent computations with overlapping live ranges */
        c0 = a0 + b0 * i;  /* Rematerialization candidate: b0 * i */
        c1 = a1 + b1 * i;
        c2 = a2 + b2 * i;
        c3 = a3 + b3 * i;
        c4 = a4 + b4 * i;
        c5 = a5 + b5 * i;
        c6 = a6 + b6 * i;
        c7 = a7 + b7 * i;
        c8 = a8 + b8 * i;
        c9 = a9 + b9 * i;
        
        /* More computations keeping many values live */
        d0 = c0 * c1 - c2;  /* c0, c1, c2 all live here */
        d1 = c1 * c2 - c3;
        d2 = c2 * c3 - c4;
        d3 = c3 * c4 - c5;
        d4 = c4 * c5 - c6;
        d5 = c5 * c6 - c7;
        d6 = c6 * c7 - c8;
        d7 = c7 * c8 - c9;
        d8 = c8 * c9 - c0;
        d9 = c9 * c0 - c1;
        
        /* Complex expression with many operands */
        sum += d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
        
        /* Inline assembly to clobber registers and increase pressure */
        asm volatile (
            "# Clobber registers to force spills\n"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r14"
        );
        
        /* Update some values to create new live ranges */
        a0 = b0 ^ i;
        a1 = b1 ^ (i + 1);
        a2 = b2 ^ (i + 2);
        a3 = b3 ^ (i + 3);
        a4 = b4 ^ (i + 4);
    }
    
    return sum;
}

/* Floating-point test mixing float and double */
float test_fp_remat(int iterations) {
    volatile float fseed = global_seed * 0.01f;
    volatile double dseed = global_seed * 0.001;
    
    /* Many floating-point variables */
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    double d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;
    
    /* Initialize with volatile reads to prevent constant folding */
    f0 = fseed + get_float(0);
    f1 = fseed + get_float(1);
    f2 = fseed + get_float(2);
    f3 = fseed + get_float(3);
    f4 = fseed + get_float(4);
    f5 = fseed + get_float(5);
    f6 = fseed + get_float(6);
    f7 = fseed + get_float(7);
    f8 = fseed + get_float(8);
    f9 = fseed + get_float(9);
    
    d0 = dseed + get_double(0);
    d1 = dseed + get_double(1);
    d2 = dseed + get_double(2);
    d3 = dseed + get_double(3);
    d4 = dseed + get_double(4);
    d5 = dseed + get_double(5);
    d6 = dseed + get_double(6);
    d7 = dseed + get_double(7);
    d8 = dseed + get_double(8);
    d9 = dseed + get_double(9);
    
    float fsum = 0.0f;
    double dsum = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed float/double computations */
        float tf0 = f0 * i + f1;  /* f0 * i is a remat candidate */
        float tf1 = f1 * i + f2;
        float tf2 = f2 * i + f3;
        float tf3 = f3 * i + f4;
        float tf4 = f4 * i + f5;
        
        double td0 = d0 * i + d1;
        double td1 = d1 * i + d2;
        double td2 = d2 * i + d3;
        double td3 = d3 * i + d4;
        double td4 = d4 * i + d5;
        
        /* Cross-type computations */
        fsum += tf0 + tf1 + tf2 + tf3 + tf4 +
                (float)(td0 * 0.1 + td1 * 0.2 + td2 * 0.3);
        
        dsum += td0 + td1 + td2 + td3 + td4 +
                (double)(tf0 * 0.01f + tf1 * 0.02f + tf2 * 0.03f);
        
        /* Update with complex expressions */
        f0 = f1 * 0.9f + fseed;
        f1 = f2 * 0.8f + fseed;
        f2 = f3 * 0.7f + fseed;
        f3 = f4 * 0.6f + fseed;
        f4 = f5 * 0.5f + fseed;
        
        d0 = d1 * 0.95 + dseed;
        d1 = d2 * 0.85 + dseed;
        d2 = d3 * 0.75 + dseed;
        d3 = d4 * 0.65 + dseed;
        d4 = d5 * 0.55 + dseed;
    }
    
    return fsum + (float)dsum;
}

/* Address calculation intensive test */
int test_addr_remat(int size) {
    /* Large array to force address calculations */
    static int array[1024 * 1024];
    
    /* Initialize array */
    for (int i = 0; i < 1024; i++) {
        array[i] = i * i;
    }
    
    int sum = 0;
    volatile int offset = global_seed & 0xFF;
    
    /* Many index variables */
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9;
    int *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7, *p8, *p9;
    
    i0 = offset + 0; i1 = offset + 1; i2 = offset + 2; 
    i3 = offset + 3; i4 = offset + 4; i5 = offset + 5;
    i6 = offset + 6; i7 = offset + 7; i8 = offset + 8; i9 = offset + 9;
    
    for (int iter = 0; iter < size; iter++) {
        /* Complex address calculations - good remat candidates */
        p0 = &array[(i0 * 7 + iter) & 1023];  /* &array[...] is rematerializable */
        p1 = &array[(i1 * 11 + iter) & 1023];
        p2 = &array[(i2 * 13 + iter) & 1023];
        p3 = &array[(i3 * 17 + iter) & 1023];
        p4 = &array[(i4 * 19 + iter) & 1023];
        p5 = &array[(i5 * 23 + iter) & 1023];
        p6 = &array[(i6 * 29 + iter) & 1023];
        p7 = &array[(i7 * 31 + iter) & 1023];
        p8 = &array[(i8 * 37 + iter) & 1023];
        p9 = &array[(i9 * 41 + iter) & 1023];
        
        /* Use all pointers in a complex expression */
        sum += *p0 + *p1 * 2 + *p2 * 3 + *p3 * 4 + *p4 * 5 +
               *p5 * 6 + *p6 * 7 + *p7 * 8 + *p8 * 9 + *p9 * 10;
        
        /* Update indices with complex expressions */
        i0 = (i0 * 3 + 1) & 1023;
        i1 = (i1 * 5 + 2) & 1023;
        i2 = (i2 * 7 + 3) & 1023;
        i3 = (i3 * 11 + 4) & 1023;
        i4 = (i4 * 13 + 5) & 1023;
        i5 = (i5 * 17 + 6) & 1023;
        i6 = (i6 * 19 + 7) & 1023;
        i7 = (i7 * 23 + 8) & 1023;
        i8 = (i8 * 29 + 9) & 1023;
        i9 = (i9 * 31 + 10) & 1023;
        
        /* Force register pressure with inline asm */
        asm volatile (
            "# More register clobbering\n"
            "nop\n"
            : 
            : 
            : "memory", "cc", 
              "r0", "r1", "r2", "r3", "r4", "r5"
        );
    }
    
    return sum;
}

/* Combined test with nested loops and complex control flow */
int test_complex_remat(int outer, int inner) {
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Variables that span the outer loop */
        int base = o * 1000 + global_seed;
        int mod = (o % 17) + 1;
        
        /* Inner loop with many live variables */
        for (int i = 0; i < inner; i++) {
            /* Many computations with overlapping live ranges */
            int a = base + i * 3;
            int b = base + i * 5;
            int c = base + i * 7;
            int d = base + i * 11;
            int e = base + i * 13;
            
            /* Complex expression tree - creates many temporary values */
            int t1 = a * b + c;
            int t2 = b * c + d;
            int t3 = c * d + e;
            int t4 = d * e + a;
            int t5 = e * a + b;
            
            /* Even more computations */
            int u1 = t1 ^ t2;
            int u2 = t2 ^ t3;
            int u3 = t3 ^ t4;
            int u4 = t4 ^ t5;
            int u5 = t5 ^ t1;
            
            /* Final combination */
            total += (u1 * mod + u2) * (u3 - u4) + u5;
            
            /* Conditional that creates different control flow paths */
            if ((i & 3) == 0) {
                /* Different computation on this path */
                total += a * 2 - b;
                
                /* Inline asm that might affect register allocation */
                asm volatile (
                    "# Conditional path clobber\n"
                    : 
                    : 
                    : "r10", "r11", "r12"
                );
            }
        }
        
        /* Update base in a way that prevents optimization */
        sink = base;  /* Use volatile sink */
    }
    
    return total;
}

int main() {
    printf("Starting early rematerialization test...\n");
    
    int idx = 0;
    
    /* Run all tests multiple times with different parameters */
    results[idx++] = test_int_remat(100);
    printf("Integer test complete: %d\n", results[idx-1]);
    
    float fp_result = test_fp_remat(50);
    results[idx++] = (int)fp_result;
    printf("Floating-point test complete: %f\n", fp_result);
    
    results[idx++] = test_addr_remat(200);
    printf("Address calculation test complete: %d\n", results[idx-1]);
    
    results[idx++] = test_complex_remat(10, 20);
    printf("Complex test complete: %d\n", results[idx-1]);
    
    /* Run additional iterations with different seeds */
    for (int i = 0; i < 5; i++) {
        global_seed += 1000;
        results[idx++] = test_int_remat(50 + i * 10);
        results[idx++] = (int)test_fp_remat(30 + i * 5);
    }
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    return checksum != 0 ? 0 : 1;
}

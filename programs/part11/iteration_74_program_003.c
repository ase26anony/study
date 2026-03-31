/* Test program to trigger early rematerialization validation logic */
#include <stdio.h>
#include <stdint.h>

/* Global arrays to prevent optimization */
volatile int global_seed = 12345;
int results[100];
volatile int sink;

/* Non-inlineable functions to force register usage */
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
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int sum = 0;
    
    /* Initialize with non-constant values */
    a = get_value(0);
    b = get_value(1);
    c = get_value(2);
    d = get_value(3);
    e = get_value(4);
    f = get_value(5);
    g = get_value(6);
    h = get_value(7);
    i = get_value(8);
    j = get_value(9);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many independent computations with overlapping live ranges */
        /* This creates many rematerialization candidates */
        int t1 = a + b * c;      /* Complex enough to be a candidate */
        int t2 = d - e * f;
        int t3 = g ^ h | i;
        int t4 = j << 2;
        int t5 = a * b + c;
        int t6 = d * e - f;
        int t7 = g | h ^ i;
        int t8 = j >> 1;
        
        /* More computations to increase pressure */
        int t9 = t1 + t2 * t3;
        int t10 = t4 - t5 * t6;
        int t11 = t7 ^ t8 | t1;
        int t12 = t2 << 3;
        int t13 = t3 * t4 + t5;
        int t14 = t6 * t7 - t8;
        int t15 = t9 | t10 ^ t11;
        int t16 = t12 >> 2;
        
        /* Force all values to be live simultaneously */
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 +
               t9 + t10 + t11 + t12 + t13 + t14 + t15 + t16;
        
        /* Inline assembly to clobber registers and force rematerialization */
        /* This creates artificial register pressure */
        asm volatile (
            "# Force register clobber\n"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Modify some variables to create new live ranges */
        a += t1;
        b ^= t2;
        c |= t3;
        d -= t4;
        e *= 3;
        f /= 2;
        g <<= 1;
        h >>= 1;
        i = ~i;
        j = -j;
    }
    
    return sum;
}

/* Floating-point intensive test */
float test_fp_remat(int iterations) {
    /* Mix float and double to use different register classes */
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh, di, dj;
    float sum = 0.0f;
    
    /* Initialize with volatile reads to prevent constant folding */
    fa = get_float(0);
    fb = get_float(1);
    fc = get_float(2);
    fd = get_float(3);
    fe = get_float(4);
    ff = get_float(5);
    fg = get_float(6);
    fh = get_float(7);
    fi = get_float(8);
    fj = get_float(9);
    
    da = get_double(0);
    db = get_double(1);
    dc = get_double(2);
    dd = get_double(3);
    de = get_double(4);
    df = get_double(5);
    dg = get_double(6);
    dh = get_double(7);
    di = get_double(8);
    dj = get_double(9);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create many FP computations that are rematerialization candidates */
        float ft1 = fa + fb * fc;
        float ft2 = fd - fe * ff;
        float ft3 = fg * fh / fi;
        float ft4 = fj * 2.0f;
        
        double dt1 = da + db * dc;
        double dt2 = dd - de * df;
        double dt3 = dg * dh / di;
        double dt4 = dj * 2.0;
        
        /* Cross-type computations to increase complexity */
        float ft5 = ft1 + (float)dt1;
        double dt5 = dt2 + (double)ft2;
        
        /* More computations to keep values live */
        float ft6 = ft3 * ft4 + ft5;
        double dt6 = dt3 * dt4 + dt5;
        
        float ft7 = ft6 / (iter + 1.0f);
        double dt7 = dt6 / (iter + 1.0);
        
        sum += ft1 + ft2 + ft3 + ft4 + ft5 + ft6 + ft7 +
               (float)(dt1 + dt2 + dt3 + dt4 + dt5 + dt6 + dt7);
        
        /* Update variables to create new live ranges */
        fa += 0.1f;
        fb -= 0.1f;
        fc *= 1.01f;
        fd /= 1.01f;
        
        da += 0.01;
        db -= 0.01;
        dc *= 1.001;
        dd /= 1.001;
    }
    
    return sum;
}

/* Address calculation intensive test */
int test_addr_remat(int iterations) {
    /* Local array to create address calculations */
    int array[256];
    int indices[32];
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        array[i] = get_value(i);
    }
    for (int i = 0; i < 32; i++) {
        indices[i] = get_value(i) & 0xFF;
    }
    
    /* Many pointer variables to create address calculation candidates */
    int *ptr1, *ptr2, *ptr3, *ptr4, *ptr5, *ptr6, *ptr7, *ptr8;
    int idx1, idx2, idx3, idx4, idx5, idx6, idx7, idx8;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex index calculations - good rematerialization candidates */
        idx1 = (iter * 7) & 0xFF;
        idx2 = (iter * 13) & 0xFF;
        idx3 = (iter * 19) & 0xFF;
        idx4 = (iter * 23) & 0xFF;
        idx5 = (iter * 29) & 0xFF;
        idx6 = (iter * 31) & 0xFF;
        idx7 = (iter * 37) & 0xFF;
        idx8 = (iter * 41) & 0xFF;
        
        /* Multiple address calculations that could be rematerialized */
        ptr1 = &array[idx1];
        ptr2 = &array[idx2];
        ptr3 = &array[idx3];
        ptr4 = &array[idx4];
        ptr5 = &array[idx5];
        ptr6 = &array[idx6];
        ptr7 = &array[idx7];
        ptr8 = &array[idx8];
        
        /* Use all pointers to keep them live */
        int val1 = *ptr1 + idx1;
        int val2 = *ptr2 + idx2;
        int val3 = *ptr3 + idx3;
        int val4 = *ptr4 + idx4;
        int val5 = *ptr5 + idx5;
        int val6 = *ptr6 + idx6;
        int val7 = *ptr7 + idx7;
        int val8 = *ptr8 + idx8;
        
        /* More complex address calculations with offsets */
        int *ptr9 = ptr1 + (iter & 0xF);
        int *ptr10 = ptr2 - (iter & 0x7);
        int *ptr11 = &array[(idx1 + idx2) & 0xFF];
        int *ptr12 = &array[(idx3 * idx4) & 0xFF];
        
        int val9 = *ptr9;
        int val10 = *ptr10;
        int val11 = *ptr11;
        int val12 = *ptr12;
        
        /* Compute using all values */
        sum += val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 +
               val9 + val10 + val11 + val12;
        
        /* Inline assembly that might trigger register reference changes */
        asm volatile (
            "# Address calculation clobber\n"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3"
        );
        
        /* Modify array to create new memory dependencies */
        array[iter & 0xFF] = sum;
    }
    
    return sum;
}

/* Combined test with all patterns */
void test_combined(int iterations) {
    int int_result = test_int_remat(iterations);
    float fp_result = test_fp_remat(iterations);
    int addr_result = test_addr_remat(iterations);
    
    results[0] = int_result;
    results[1] = *(int*)&fp_result;  /* Store float as int */
    results[2] = addr_result;
    
    /* Additional mixed computation to increase complexity */
    volatile int v1 = global_seed;
    volatile int v2 = global_seed * 2;
    volatile int v3 = global_seed / 2;
    
    for (int i = 0; i < 50; i++) {
        /* Create many temporary computations */
        int t1 = v1 + i * 3;
        int t2 = v2 - i * 5;
        int t3 = v3 | i * 7;
        int t4 = t1 ^ t2;
        int t5 = t2 & t3;
        int t6 = t3 | t4;
        int t7 = t4 ^ t5;
        int t8 = t5 & t6;
        int t9 = t6 | t7;
        int t10 = t7 ^ t8;
        
        /* Use all temporaries */
        results[3 + (i % 10)] += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Force spill/remat candidates */
        asm volatile (
            "# Combined clobber\n"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5"
        );
    }
}

int main() {
    printf("Starting early rematerialization test...\n");
    
    /* Initialize results array */
    for (int i = 0; i < 100; i++) {
        results[i] = 0;
    }
    
    /* Run tests with different iteration counts to vary register pressure */
    test_combined(100);
    test_combined(50);
    test_combined(200);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    return checksum != 0 ? 0 : 1;
}

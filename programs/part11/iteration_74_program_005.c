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

/* Integer-intensive test with many live variables */
int test_int_remat(int iterations) {
    /* Declare many integer variables to consume registers */
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    int c0, c1, c2, c3, c4, c5, c6, c7, c8, c9;
    int sum = 0;
    
    /* Initialize with non-trivial values */
    a0 = get_value(0);
    a1 = get_value(1);
    a2 = get_value(2);
    a3 = get_value(3);
    a4 = get_value(4);
    a5 = get_value(5);
    a6 = get_value(6);
    a7 = get_value(7);
    a8 = get_value(8);
    a9 = get_value(9);
    
    for (int i = 0; i < iterations; i++) {
        /* Create many independent computations with overlapping live ranges */
        b0 = a0 + a1 * 3;      /* Rematerialization candidate: a0 + a1 * 3 */
        b1 = a1 - a2 / 2;      /* Another candidate */
        b2 = a2 * a3 + i;      /* Depends on loop counter */
        b3 = a3 ^ a4;          /* Bitwise operation */
        b4 = a4 | a5;          /* Another bitwise */
        b5 = a5 & a6;          /* And another */
        b6 = a6 << 2;          /* Shift */
        b7 = a7 >> 1;          /* Another shift */
        b8 = a8 + a9 * 7;      /* Complex expression */
        b9 = a9 - a0 / 3;      /* Cross-dependency */
        
        /* More computations creating register pressure */
        c0 = b0 * b1 - b2;     /* Uses multiple b values */
        c1 = b1 + b3 * b4;     /* More combinations */
        c2 = b2 ^ b5 | b6;     /* Mixed operations */
        c3 = b3 << (b7 & 15);  /* Shift with variable amount */
        c4 = b4 + b8 * b9;     /* Another complex expression */
        c5 = b5 - b6 / 2;      /* Division */
        c6 = b6 * b7 + b8;     /* Multiplication and addition */
        c7 = b7 | b9 ^ b0;     /* More bitwise */
        c8 = b8 << 3;          /* Constant shift */
        c9 = b9 >> 2;          /* Another constant shift */
        
        /* Force all values to be live simultaneously */
        sum += c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9;
        
        /* Inline assembly to clobber registers and increase pressure */
        asm volatile (
            "# Force register clobber\n"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r14"
        );
        
        /* Update some values for next iteration */
        a0 = (a0 + 1) & 0xFFF;
        a1 = (a1 * 3) & 0xFFF;
        a2 = (a2 - 5) & 0xFFF;
        a3 = (a3 ^ i) & 0xFFF;
    }
    
    return sum;
}

/* Floating-point test mixing int and float operations */
float test_fp_remat(int iterations) {
    /* Mix float and int variables */
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9;
    float sum = 0.0f;
    
    /* Initialize with volatile reads to prevent constant folding */
    f0 = get_float(0);
    f1 = get_float(1);
    f2 = get_float(2);
    f3 = get_float(3);
    f4 = get_float(4);
    f5 = get_float(5);
    f6 = get_float(6);
    f7 = get_float(7);
    f8 = get_float(8);
    f9 = get_float(9);
    
    i0 = get_value(0);
    i1 = get_value(1);
    i2 = get_value(2);
    i3 = get_value(3);
    i4 = get_value(4);
    i5 = get_value(5);
    i6 = get_value(6);
    i7 = get_value(7);
    i8 = get_value(8);
    i9 = get_value(9);
    
    for (int i = 0; i < iterations; i++) {
        /* Mixed int/float computations */
        float t0 = f0 * f1 + (float)i0;    /* Float with int conversion */
        float t1 = f1 - f2 * (float)i1;    /* Another mixed computation */
        float t2 = f2 / f3 + (float)(i2 * i3); /* More complex */
        float t3 = f3 * (float)(i4 ^ i5);  /* Bitwise in float context */
        float t4 = f4 + (float)(i6 << 2);  /* Shift in float context */
        
        /* Integer computations that might need rematerialization */
        int j0 = i0 + (int)(f5 * 100.0f);  /* Float to int */
        int j1 = i1 * (int)(f6 / 2.0f);    /* Another conversion */
        int j2 = (int)f7 ^ i2;             /* Mixed operation */
        int j3 = i3 << (int)(f8 * 4.0f);   /* Float-controlled shift */
        int j4 = i4 + (int)f9 * i5;        /* More mixing */
        
        /* Use all values in a way that keeps them live */
        sum += t0 * t1 - t2 + t3 * t4;
        sum += (float)(j0 * j1 + j2 - j3 * j4) * 0.01f;
        
        /* Update values */
        f0 = f0 * 1.01f;
        f1 = f1 - 0.5f;
        i0 = (i0 * 3 + 1) & 0x7FF;
        i1 = (i1 ^ i) & 0x7FF;
    }
    
    return sum;
}

/* Address calculation heavy test */
int test_addr_remat(int iterations) {
    /* Local array to create address calculations */
    int array[256];
    int *ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
    int idx0, idx1, idx2, idx3, idx4, idx5, idx6, idx7, idx8, idx9;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        array[i] = get_value(i);
    }
    
    /* Initialize indices with complex expressions */
    idx0 = get_value(0) & 0xFF;
    idx1 = get_value(1) & 0xFF;
    idx2 = get_value(2) & 0xFF;
    idx3 = get_value(3) & 0xFF;
    idx4 = get_value(4) & 0xFF;
    idx5 = get_value(5) & 0xFF;
    idx6 = get_value(6) & 0xFF;
    idx7 = get_value(7) & 0xFF;
    idx8 = get_value(8) & 0xFF;
    idx9 = get_value(9) & 0xFF;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex address calculations - each is a rematerialization candidate */
        ptr1 = &array[(idx0 * 7 + idx1) & 0xFF];      /* Base + offset */
        ptr2 = &array[(idx2 ^ idx3 * 3) & 0xFF];      /* Another complex index */
        ptr3 = &array[(idx4 << 2 | idx5 >> 3) & 0xFF]; /* Bitwise combination */
        ptr4 = &array[(idx6 + idx7 * 5 - i) & 0xFF];  /* With loop counter */
        ptr5 = &array[(idx8 * 11 + idx9 * 13) & 0xFF]; /* Multiple indices */
        
        /* More address calculations using previous pointers */
        int *ptr6 = ptr1 + (idx0 & 0xF);      /* Pointer arithmetic */
        int *ptr7 = ptr2 - (idx1 & 0x7);      /* More pointer math */
        int *ptr8 = ptr3 + ((idx2 * idx3) & 0xF); /* Complex offset */
        int *ptr9 = ptr4 - ((idx4 ^ idx5) & 0x7); /* Another offset */
        int *ptr10 = ptr5 + ((idx6 | idx7) & 0xF); /* Bitwise offset */
        
        /* Use all pointers to access memory */
        sum += *ptr1 + *ptr2 * 2 - *ptr3 / 3 + *ptr4 | *ptr5;
        sum += *ptr6 ^ *ptr7 + *ptr8 * *ptr9 - *ptr10;
        
        /* Complex index updates - create many live values */
        idx0 = (idx0 + idx1 * 3) & 0xFF;
        idx1 = (idx1 ^ idx2) & 0xFF;
        idx2 = (idx2 * 5 + idx3) & 0xFF;
        idx3 = (idx3 - idx4 / 2) & 0xFF;
        idx4 = (idx4 | idx5 << 1) & 0xFF;
        idx5 = (idx5 + i * 7) & 0xFF;
        idx6 = (idx6 ^ i) & 0xFF;
        idx7 = (idx7 * 3 + 1) & 0xFF;
        idx8 = (idx8 - idx9) & 0xFF;
        idx9 = (idx9 * 11) & 0xFF;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    return sum;
}

/* Additional test with nested loops and complex control flow */
int test_nested_remat(int iterations) {
    int a = get_value(0);
    int b = get_value(1);
    int c = get_value(2);
    int d = get_value(3);
    int e = get_value(4);
    int f = get_value(5);
    int g = get_value(6);
    int h = get_value(7);
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int t1 = a * b + c;      /* Candidate for remat */
        int t2 = d - e * f;      /* Another candidate */
        int t3 = g ^ h;          /* Simple candidate */
        
        for (int j = 0; j < 3; j++) {
            /* Inner loop uses outer values - they must stay live */
            int u1 = t1 + j * 5;    /* Uses t1 */
            int u2 = t2 - j * 3;    /* Uses t2 */
            int u3 = t3 ^ j;        /* Uses t3 */
            
            /* More computations */
            int v1 = u1 * u2 + u3;
            int v2 = u1 - u2 * u3;
            int v3 = u2 ^ u3 | u1;
            
            sum += v1 + v2 * 2 - v3 / 4;
            
            /* Update some values */
            t1 = (t1 + 1) & 0xFFF;
            t2 = (t2 - j) & 0xFFF;
        }
        
        /* Update outer values */
        a = (a + b) & 0xFFF;
        b = (b ^ c) & 0xFFF;
        c = (c * 3) & 0xFFF;
        d = (d - e) & 0xFFF;
        e = (e | f) & 0xFFF;
        f = (f + i) & 0xFFF;
        g = (g * 5) & 0xFFF;
        h = (h ^ i) & 0xFFF;
    }
    
    return sum;
}

int main() {
    int result_idx = 0;
    int total = 0;
    
    printf("Starting early rematerialization test...\n");
    
    /* Run all tests with different iteration counts */
    results[result_idx++] = test_int_remat(100);
    results[result_idx++] = (int)test_fp_remat(50);
    results[result_idx++] = test_addr_remat(75);
    results[result_idx++] = test_nested_remat(60);
    
    /* Run multiple times with different parameters */
    for (int i = 0; i < 5; i++) {
        results[result_idx++] = test_int_remat(20 + i * 10);
        results[result_idx++] = (int)test_fp_remat(15 + i * 5);
        results[result_idx++] = test_addr_remat(25 + i * 8);
        global_seed += 1000;  /* Change seed for variation */
    }
    
    /* Compute checksum */
    for (int i = 0; i < result_idx; i++) {
        total += results[i];
        total = (total * 31 + 17) & 0x7FFFFFFF;
    }
    
    printf("Checksum: %d\n", total);
    printf("Test completed.\n");
    
    return total != 0 ? 0 : 1;
}

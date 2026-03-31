/* Test program to trigger early rematerialization pass in GCC */
/* Specifically targeting lines 930-937 in early-remat.cc */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Global arrays to prevent optimization */
volatile int global_seed = 12345;
int results[100];
volatile int sink;

/* Non-inlineable functions to prevent optimization */
__attribute__((noinline, noipa)) int get_value(int idx) {
    return global_seed + idx * 1103515245;
}

__attribute__((noinline, noipa)) float get_float(int idx) {
    return (float)(global_seed + idx) * 0.12345f;
}

__attribute__((noinline, noipa)) double get_double(int idx) {
    return (double)(global_seed - idx) * 0.67891;
}

/* Integer-intensive test with many live variables */
__attribute__((noinline)) int test_int_remat(int iterations) {
    /* Declare many integer variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int sum = 0;
    
    /* Initialize with volatile reads to prevent constant propagation */
    a = get_value(0);
    b = get_value(1);
    c = get_value(2);
    d = get_value(3);
    e = get_value(4);
    f = get_value(5);
    
    /* Complex sequence of independent operations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Many arithmetic operations creating rematerialization candidates */
        g = a + b * 3;      /* Candidate for remat: b * 3 */
        h = c - d / 2;      /* Candidate: d / 2 */
        i = e ^ f;          /* Candidate: e ^ f */
        j = g * h;          /* Candidate: g * h (but g,h themselves are candidates) */
        k = i << 2;         /* Candidate: i << 2 */
        l = j + k;
        m = a & b;          /* Candidate: a & b */
        n = c | d;          /* Candidate: c | d */
        o = e ^ ~f;         /* Candidate: ~f */
        p = m * n;
        q = o >> 1;
        r = p + q;
        s = l * r;
        t = s ^ (iter & 0xFF);
        
        /* More operations to increase live range */
        u = t * 17;
        v = u - 42;
        w = v / 3;
        x = w + 100;
        y = x ^ 0x55AA55AA;
        z = y & 0x00FF00FF;
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile(
            "# Clobber registers to increase pressure\n\t"
            "nop"
            : 
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r14"
        );
        
        /* Use all variables to keep them live */
        sum += z + u + v + w + x + y;
        
        /* Modify some variables to create new values */
        a = b + iter;
        b = c ^ iter;
        c = d - iter;
        d = e * (iter + 1);
        e = f / (iter + 2);
        f = get_value(iter % 10);
    }
    
    return sum;
}

/* Floating-point intensive test */
__attribute__((noinline)) float test_fp_remat(int iterations) {
    /* Mix float and double to use different register classes */
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh, di, dj;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Initialize with volatile reads */
    fa = get_float(0);
    fb = get_float(1);
    fc = get_float(2);
    fd = get_float(3);
    fe = get_float(4);
    ff = get_float(5);
    
    da = get_double(0);
    db = get_double(1);
    dc = get_double(2);
    dd = get_double(3);
    de = get_double(4);
    df = get_double(5);
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Float operations - many candidates for remat */
        fg = fa + fb * 1.5f;    /* Candidate: fb * 1.5f */
        fh = fc - fd / 2.0f;    /* Candidate: fd / 2.0f */
        fi = fe * ff;
        fj = fg * fh * fi;
        
        /* Double operations */
        dg = da + db * 1.5;
        dh = dc - dd / 2.0;
        di = de * df;
        dj = dg * dh * di;
        
        /* Type conversions create additional register pressure */
        float mixed1 = (float)dg + fg;
        double mixed2 = (double)fh + dh;
        
        /* Complex expression with many intermediates */
        float temp1 = mixed1 * 3.14159f;
        double temp2 = mixed2 * 2.71828;
        float temp3 = temp1 + (float)temp2;
        double temp4 = (double)temp3 * 1.41421;
        
        /* Inline assembly clobbering FP registers */
        asm volatile(
            "# Clobber FP registers\n\t"
            "nop"
            :
            :
            : "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
              "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15",
              "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7"
        );
        
        fsum += fj + temp1 + temp3;
        dsum += dj + temp2 + temp4;
        
        /* Modify variables */
        fa = fb + (float)iter * 0.1f;
        fb = fc * 1.1f;
        fc = get_float(iter % 8);
        
        da = db + (double)iter * 0.01;
        db = dc * 1.01;
        dc = get_double(iter % 8);
    }
    
    return fsum + (float)dsum;
}

/* Address calculation intensive test */
__attribute__((noinline)) int test_addr_remat(int iterations) {
    /* Large array to work with */
    static int array[1024];
    int indices[20];
    int *ptrs[10];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 1024; i++) {
        array[i] = i * 3 + global_seed;
    }
    
    /* Initialize indices with complex patterns */
    for (int i = 0; i < 20; i++) {
        indices[i] = (i * 7 + global_seed) & 1023;
    }
    
    /* Initialize pointers */
    for (int i = 0; i < 10; i++) {
        ptrs[i] = &array[i * 64];
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Many complex address calculations - prime candidates for remat */
        int idx1 = (indices[0] + iter * 13) & 1023;
        int idx2 = (indices[1] + iter * 17) & 1023;
        int idx3 = (indices[2] + iter * 19) & 1023;
        int idx4 = (indices[3] + iter * 23) & 1023;
        int idx5 = (indices[4] + iter * 29) & 1023;
        int idx6 = (indices[5] + iter * 31) & 1023;
        int idx7 = (indices[6] + iter * 37) & 1023;
        int idx8 = (indices[7] + iter * 41) & 1023;
        
        /* Multiple pointer arithmetic operations */
        int *p1 = ptrs[0] + idx1;
        int *p2 = ptrs[1] + idx2;
        int *p3 = ptrs[2] + idx3;
        int *p4 = ptrs[3] + idx4;
        int *p5 = ptrs[4] + idx5;
        int *p6 = ptrs[5] + idx6;
        int *p7 = ptrs[6] + idx7;
        int *p8 = ptrs[7] + idx8;
        
        /* Complex offset calculations */
        int offset1 = idx1 * 3 + 7;
        int offset2 = idx2 * 5 + 11;
        int offset3 = idx3 * 7 + 13;
        int offset4 = idx4 * 11 + 17;
        
        /* Multiple memory accesses with complex addressing */
        int val1 = *(p1 + offset1);
        int val2 = *(p2 + offset2);
        int val3 = *(p3 + offset3);
        int val4 = *(p4 + offset4);
        
        /* More address calculations */
        int base_idx = (iter * 53) & 1023;
        int stride1 = (iter % 16) + 1;
        int stride2 = (iter % 32) + 2;
        
        /* Loop with address calculations - creates many live ranges */
        int subsum = 0;
        for (int j = 0; j < 8; j++) {
            int addr_idx = (base_idx + j * stride1) & 1023;
            int *addr = &array[addr_idx];
            int val = *addr;
            
            int addr_idx2 = (addr_idx + stride2) & 1023;
            int *addr2 = &array[addr_idx2];
            int val2 = *addr2;
            
            subsum += val + val2;
            
            /* Additional address math */
            int offset_addr = (addr_idx * 3 + j * 7) & 1023;
            subsum += array[offset_addr];
        }
        
        /* Inline assembly to clobber address registers */
        asm volatile(
            "# Clobber address registers\n\t"
            "nop"
            :
            :
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5"
        );
        
        sum += val1 + val2 + val3 + val4 + subsum;
        
        /* Update indices to create new addressing patterns */
        for (int i = 0; i < 20; i++) {
            indices[i] = (indices[i] + iter * (i + 1)) & 1023;
        }
    }
    
    return sum;
}

/* Mixed test combining all patterns */
__attribute__((noinline)) int test_mixed_remat(int iterations) {
    int int_result = test_int_remat(iterations / 4);
    float fp_result = test_fp_remat(iterations / 4);
    int addr_result = test_addr_remat(iterations / 4);
    
    /* Combine results in a complex way */
    int combined = int_result + (int)fp_result + addr_result;
    
    /* Additional complex computation to create more pressure */
    int temp = combined;
    for (int i = 0; i < 50; i++) {
        temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
        temp ^= (temp >> 16);
        temp += i * 17;
    }
    
    return temp;
}

int main() {
    printf("Starting early rematerialization test...\n");
    
    int result_idx = 0;
    
    /* Run tests with different parameters to explore different paths */
    results[result_idx++] = test_int_remat(100);
    printf("Integer test complete: %d\n", results[result_idx-1]);
    
    results[result_idx++] = (int)test_fp_remat(80);
    printf("FP test complete: %d\n", results[result_idx-1]);
    
    results[result_idx++] = test_addr_remat(60);
    printf("Address test complete: %d\n", results[result_idx-1]);
    
    results[result_idx++] = test_mixed_remat(40);
    printf("Mixed test complete: %d\n", results[result_idx-1]);
    
    /* Run more iterations with different seeds */
    for (int i = 0; i < 5; i++) {
        global_seed = 54321 + i * 1000;
        results[result_idx++] = test_int_remat(50 + i * 10);
        results[result_idx++] = (int)test_fp_remat(40 + i * 8);
        results[result_idx++] = test_addr_remat(30 + i * 6);
    }
    
    /* Compute final checksum */
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum ^= results[i];
        checksum = (checksum << 1) | (checksum >> 31);
        checksum += results[i] * 3;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Test completed successfully.\n");
    
    return checksum != 0 ? 0 : 1;
}

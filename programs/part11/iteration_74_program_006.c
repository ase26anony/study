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
    return (float)(global_seed + idx) * 0.12345f;
}

__attribute__((noinline, noipa)) double get_double(int idx) {
    return (double)(global_seed - idx) * 0.67891;
}

/* Integer-intensive test with many live variables */
int test_int_remat(int iterations) {
    /* Declare many integer variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int sum = 0;
    
    volatile int seed = global_seed;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Load initial values - these create rematerialization candidates */
        a = seed + iter * 1;
        b = seed + iter * 3;
        c = seed + iter * 5;
        d = seed + iter * 7;
        e = seed + iter * 11;
        
        /* Many independent computations keeping variables live */
        f = a * b + c;      /* f depends on a, b, c */
        g = d - e * 2;      /* g depends on d, e */
        h = (a << 3) | (b & 0xFF);  /* h depends on a, b */
        i = c ^ d ^ e;      /* i depends on c, d, e */
        j = f * g - h;      /* j depends on f, g, h */
        
        /* More computations creating web of dependencies */
        k = (i + j) * 17;
        l = (f - g) * 23;
        m = (h | i) & 0x7FFFFFFF;
        n = (j ^ k) + l;
        o = (m * n) / 31;
        
        /* Even more variables to increase pressure */
        p = (a + b + c + d + e) * 19;
        q = (f + g + h + i + j) * 29;
        r = (k + l + m + n + o) * 37;
        s = p ^ q ^ r;
        t = (s * 41) & 0xFFFF;
        
        u = get_value(iter);  /* Function call forces register saves */
        v = u * t + s;
        w = v * r + q;
        x = w * p + o;
        y = x * n + m;
        z = y * l + k;
        
        /* Inline assembly to clobber registers and increase pressure */
        asm volatile(
            "# Clobber registers to force rematerialization\n"
            "mov %0, %0\n"
            : "+r" (a), "+r" (b), "+r" (c), "+r" (d), "+r" (e)
            : 
            : "memory"
        );
        
        /* Combine all results - keeps all variables live */
        sum += a + b + c + d + e + f + g + h + i + j +
               k + l + m + n + o + p + q + r + s + t +
               u + v + w + x + y + z;
        
        /* Another asm to potentially trigger validate_change */
        asm volatile(
            "# More register pressure\n"
            : 
            : "r" (sum), "r" (iter), "r" (seed)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
    }
    
    return sum;
}

/* Floating-point intensive test */
float test_fp_remat(int iterations) {
    /* Mix float and double to use different register classes */
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    double da, db, dc, dd, de, df, dg, dh, di, dj;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    volatile int seed = global_seed;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Get values from non-inlineable functions */
        fa = get_float(iter * 2);
        fb = get_float(iter * 3);
        fc = get_float(iter * 5);
        fd = get_float(iter * 7);
        fe = get_float(iter * 11);
        
        da = get_double(iter * 2);
        db = get_double(iter * 3);
        dc = get_double(iter * 5);
        dd = get_double(iter * 7);
        de = get_double(iter * 11);
        
        /* Many FP operations */
        ff = fa * fb + fc;
        fg = fd - fe * 2.0f;
        fh = fa / (fb + 1.0f);
        fi = fc * fd * fe;
        fj = ff + fg - fh + fi;
        
        df = da * db + dc;
        dg = dd - de * 2.0;
        dh = da / (db + 1.0);
        di = dc * dd * de;
        dj = df + dg - dh + di;
        
        /* Mix float and double operations */
        float fmix = (float)(df) + ff;
        double dmix = (double)(fg) + dg;
        
        /* Integer index computations mixed with FP */
        int idx1 = (int)(fa * 100.0f);
        int idx2 = (int)(da * 100.0);
        int idx3 = idx1 * idx2 + iter;
        
        /* More computations to increase pressure */
        for (int inner = 0; inner < 3; inner++) {
            float ftemp = fa + (float)inner * 0.5f;
            double dtemp = da + (double)inner * 0.5;
            
            ff += ftemp * fb;
            df += dtemp * db;
            
            /* Address calculation that could be rematerialized */
            int array_idx = (idx3 * 7 + inner * 11) & 0xFF;
            fg += results[array_idx] * 0.01f;
            dg += results[array_idx] * 0.01;
        }
        
        fsum += ff + fg + fh + fi + fj + fmix;
        dsum += df + dg + dh + di + dj + dmix;
        
        /* Inline assembly with FP clobbers */
        asm volatile(
            "# FP register pressure\n"
            : 
            : "r" (iter), "r" (seed)
            : "d0", "d1", "d2", "d3", "d4", "d5", 
              "s0", "s1", "s2", "s3", "s4", "s5", "memory"
        );
    }
    
    return fsum + (float)dsum;
}

/* Address calculation intensive test */
int test_addr_remat(int iterations) {
    int sum = 0;
    volatile int seed = global_seed;
    
    /* Local array to generate address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = seed + i * 3;
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Many index computations - potential remat candidates */
        int idx_a = (iter * 17) & 0xFF;
        int idx_b = (iter * 23) & 0xFF;
        int idx_c = (iter * 31) & 0xFF;
        int idx_d = (iter * 37) & 0xFF;
        int idx_e = (iter * 41) & 0xFF;
        int idx_f = (iter * 43) & 0xFF;
        int idx_g = (iter * 47) & 0xFF;
        int idx_h = (iter * 53) & 0xFF;
        int idx_i = (iter * 59) & 0xFF;
        int idx_j = (iter * 61) & 0xFF;
        
        /* Complex address calculations that could be rematerialized */
        int *ptr_a = &local_array[idx_a];
        int *ptr_b = &local_array[idx_b];
        int *ptr_c = &local_array[idx_c];
        int *ptr_d = &local_array[idx_d];
        int *ptr_e = &local_array[idx_e];
        int *ptr_f = &local_array[idx_f];
        int *ptr_g = &local_array[idx_g];
        int *ptr_h = &local_array[idx_h];
        int *ptr_i = &local_array[idx_i];
        int *ptr_j = &local_array[idx_j];
        
        /* Use all pointers in computation */
        int val_a = *ptr_a + idx_a;
        int val_b = *ptr_b + idx_b;
        int val_c = *ptr_c + idx_c;
        int val_d = *ptr_d + idx_d;
        int val_e = *ptr_e + idx_e;
        int val_f = *ptr_f + idx_f;
        int val_g = *ptr_g + idx_g;
        int val_h = *ptr_h + idx_h;
        int val_i = *ptr_i + idx_i;
        int val_j = *ptr_j + idx_j;
        
        /* More complex indexing with arithmetic */
        for (int offset = 0; offset < 4; offset++) {
            int complex_idx1 = (idx_a * 7 + offset * 11) & 0xFF;
            int complex_idx2 = (idx_b * 13 + offset * 17) & 0xFF;
            int complex_idx3 = (idx_c * 19 + offset * 23) & 0xFF;
            
            int *cptr1 = &local_array[complex_idx1];
            int *cptr2 = &local_array[complex_idx2];
            int *cptr3 = &local_array[complex_idx3];
            
            val_a += *cptr1 + offset;
            val_b += *cptr2 + offset * 2;
            val_c += *cptr3 + offset * 3;
            
            /* Nested address calculation */
            int nested_idx = (complex_idx1 + complex_idx2 + complex_idx3) & 0xFF;
            int *nptr = &local_array[nested_idx];
            val_d += *nptr + offset * 4;
        }
        
        /* Combine all values */
        int temp_sum = val_a + val_b + val_c + val_d + val_e +
                      val_f + val_g + val_h + val_i + val_j;
        
        /* Inline assembly that might trigger validate_change */
        asm volatile(
            "# Address calculation register pressure\n"
            "mov %[ptr1], %[ptr1]\n"
            "mov %[ptr2], %[ptr2]\n"
            : [ptr1] "+r" (ptr_a), [ptr2] "+r" (ptr_b)
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        
        sum += temp_sum;
        
        /* Force spill/reload behavior */
        sink = val_a + val_b + val_c;
    }
    
    return sum;
}

/* Mixed test with all patterns */
int test_mixed_remat(int iterations) {
    int int_result = test_int_remat(iterations / 10 + 1);
    float fp_result = test_fp_remat(iterations / 10 + 1);
    int addr_result = test_addr_remat(iterations / 10 + 1);
    
    return int_result + (int)fp_result + addr_result;
}

int main() {
    printf("Starting early rematerialization test...\n");
    
    int total_checksum = 0;
    
    /* Run tests with different iteration counts to vary register pressure */
    results[0] = test_int_remat(100);
    results[1] = test_fp_remat(50);
    results[2] = test_addr_remat(80);
    results[3] = test_mixed_remat(60);
    
    /* Additional specialized tests */
    for (int i = 0; i < 10; i++) {
        results[4 + i] = test_int_remat(20 + i * 5);
        results[14 + i] = test_addr_remat(15 + i * 3);
    }
    
    /* Compute final checksum */
    for (int i = 0; i < 24; i++) {
        total_checksum += results[i];
        total_checksum = (total_checksum << 3) | (total_checksum >> 29); /* rotate */
    }
    
    printf("Test completed. Checksum: %d\n", total_checksum);
    printf("If compiled with -fdump-rtl-early-remat, check for pass activity\n");
    
    return total_checksum != 0 ? 0 : 1;
}

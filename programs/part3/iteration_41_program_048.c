/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO: gcc -O2 -fearly-remat -flto -ffat-lto-objects -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force values to be recomputable but not constant-folded */
static volatile int global_volatile = 0;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    /* Local variables for register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    float fa, fb, fc, fd, fe, ff, fg, fh;
    double da, db, dc, dd, de;
    long la, lb, lc, ld, le;
    
    /* Local array for address calculation candidate */
    int local_array[100];
    
    /* Initialize some values */
    for (int idx = 0; idx < 100; idx++) {
        local_array[idx] = idx + global_volatile;
    }
    
    volatile int loop_counter = arg1 & 0xFF;
    if (loop_counter < 1) loop_counter = 10;
    
    volatile int result = 0;
    
    /* Main loop to create live range splits */
    for (int iter = 0; iter < loop_counter; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + 10;  /* arg1 + 10 is cheap to recompute */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[arg2 % 50];  /* &local_array[i] is recomputable */
        
        /* Candidate 3: Another simple computation */
        int cand3 = (arg3 * 2) + 5;
        
        /* Use candidates immediately in BLOCK A */
        result += cand1;
        result += *cand2;
        result += cand3;
        
        /* Control flow: conditional jump to split live range */
        /* Use volatile condition to prevent optimization */
        volatile int always_true = global_volatile >= 0;  /* Always true at runtime */
        
        if (always_true) {
            /* BLOCK B: High register pressure region */
            /* This should cause the compiler to reconsider rematerialization */
            
            /* Inline assembly to force memory clobber and spills */
            asm volatile("" ::: "memory");
            
            /* Dense independent arithmetic operations - 15+ variables */
            a = arg1 + iter;
            b = arg2 + a;
            c = arg3 + b;
            d = arg4 + c;
            e = a * b;
            f = c * d;
            g = e + f;
            h = g - a;
            i = h * 2;
            j = i / 3;
            k = j | 0xFF;
            l = k ^ 0xAA;
            m = l << 2;
            n = m >> 1;
            o = n & 0x55;
            p = o + arg1;
            q = p - arg2;
            r = q * arg3;
            s = r / (arg4 | 1);  /* Avoid division by zero */
            t = s % 256;
            
            /* Floating point operations for FP register pressure */
            fa = (float)arg1 * 1.1f;
            fb = (float)arg2 * 2.2f;
            fc = fa + fb;
            fd = fc * 3.3f;
            fe = fd - fa;
            ff = fe / 4.4f;
            fg = ff * fb;
            fh = fg + 5.5f;
            
            /* Double precision operations */
            da = (double)arg3 * 1.111;
            db = (double)arg4 * 2.222;
            dc = da + db;
            dd = dc * 3.333;
            de = dd - da;
            
            /* Long integer operations */
            la = (long)arg1 * 1000L;
            lb = (long)arg2 * 2000L;
            lc = la + lb;
            ld = lc * 3000L;
            le = ld - la;
            
#ifdef __SSE2__
            /* Vector operations for even more register pressure */
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {iter, iter+1, iter+2, iter+3};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            v4sf vf1 = {(float)arg1, (float)arg2, (float)arg3, (float)arg4};
            v4sf vf2 = {(float)iter, (float)iter+1, (float)iter+2, (float)iter+3};
            v4sf vf3 = vf1 + vf2;
            v4sf vf4 = vf1 * vf2;
#endif
            
            /* More operations to ensure high pressure */
            a = b + c + d + e + f + g;
            b = h + i + j + k + l;
            c = m + n + o + p + q;
            d = r + s + t + arg1 + arg2;
            
            /* Another memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* BLOCK C: Use rematerialization candidates again */
            /* After high pressure, compiler may need to replace old remats */
            result += cand1 * 2;
            result += *cand2 + 5;
            result += cand3 - 3;
            
            /* Use all the computed values to prevent elimination */
            result += a + b + c + d + e + f + g + h + i + j;
            result += k + l + m + n + o + p + q + r + s + t;
            result += (int)fa + (int)fb + (int)fc + (int)fd;
            result += (int)fe + (int)ff + (int)fg + (int)fh;
            result += (int)da + (int)db + (int)dc + (int)dd + (int)de;
            result += (int)la + (int)lb + (int)lc + (int)ld + (int)le;
            
#ifdef __SSE2__
            /* Use vector results */
            int *vp = (int*)&v5;
            result += vp[0] + vp[1] + vp[2] + vp[3];
            
            float *vfp = (float*)&vf3;
            result += (int)vfp[0] + (int)vfp[1] + (int)vfp[2] + (int)vfp[3];
#endif
        } else {
            /* Unreachable path, but needed for control flow */
            result += cand1 + *cand2 + cand3;
        }
        
        /* Additional loop-invariant computation that's a remat candidate */
        int loop_invariant = arg1 * arg2 + arg3 - arg4;
        result += loop_invariant;
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Call test function multiple times with different volatile args */
    for (int i = 0; i < iterations; i++) {
        /* Use different arguments each iteration to prevent constant propagation */
        volatile int arg1 = global_volatile + i * 3;
        volatile int arg2 = global_volatile + i * 5;
        volatile int arg3 = global_volatile + i * 7;
        volatile int arg4 = global_volatile + i * 11;
        
        total += test_remat(arg1, arg2, arg3, arg4);
        
        /* Modify global volatile to change recomputable expressions */
        global_volatile += 1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}

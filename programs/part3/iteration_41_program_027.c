/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions that can't be constant-folded */
static int volatile_cond = 1;
static int volatile_arg = 0;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static int __attribute__((noinline)) 
test_remat(int volatile varg1, int volatile varg2, int volatile varg3, int volatile varg4)
{
    /* Local variables for register pressure */
    int i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    long long ll1, ll2, ll3, ll4, ll5, ll6;
    
    /* Local array for address calculations */
    int local_array[100];
    for (i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    int result = 0;
    int loop_count = 100;
    
    /* Main loop where rematerialization candidates are created and used */
    for (i = 0; i < loop_count; i++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = varg1 + 10;  /* Should be rematerialized: varg1 + 10 */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[varg2 + 5];  /* &local_array[varg2 + 5] */
        
        /* Candidate 3: Another arithmetic expression */
        int cand3 = varg3 * 2 + varg4;  /* varg3 * 2 + varg4 */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (varg1 << 2) | (varg2 & 0xFF);
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand2;
        result += cand3;
        result += cand4;
        
        /* Control flow to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (volatile_cond) {
            /* BLOCK B: High register pressure region */
            /* This should cause register pressure and force reconsideration of remats */
            
            /* Many independent arithmetic operations */
            j = i * 3 + 1;
            k = j * 2 - 5;
            l = k + j * 7;
            m = l * 11 % 31;
            n = m ^ (i << 3);
            o = n * 13;
            p = o / 7;
            q = p + 19;
            r = q * 3;
            s = r - 42;
            t = s * 2;
            u = t + 100;
            v = u / 3;
            w = v * 7;
            x = w - 23;
            y = x + 456;
            z = y ^ 0x55AA;
            
            /* Floating point operations */
            f1 = i * 0.1f;
            f2 = f1 * 2.5f;
            f3 = f2 + 3.14f;
            f4 = f3 * 0.5f;
            f5 = f4 - 1.0f;
            f6 = f5 * 2.0f;
            f7 = f6 + 10.0f;
            f8 = f7 / 3.0f;
            f9 = f8 * 1.5f;
            f10 = f9 - 5.0f;
            
            /* Double precision */
            d1 = i * 0.01;
            d2 = d1 * 1.5;
            d3 = d2 + 2.71828;
            d4 = d3 * 0.25;
            d5 = d4 - 1.0;
            d6 = d5 * 3.0;
            d7 = d6 + 20.0;
            d8 = d7 / 4.0;
            
            /* Long long operations */
            ll1 = i * 1000LL;
            ll2 = ll1 + 5000LL;
            ll3 = ll2 * 3LL;
            ll4 = ll3 - 7000LL;
            ll5 = ll4 / 2LL;
            ll6 = ll5 + 12345LL;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            v4si vec1 = {i, i+1, i+2, i+3};
            v4si vec2 = {varg1, varg2, varg3, varg4};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec3 * vec1;
            v4si vec5 = vec4 - vec2;
            v4si vec6 = vec5 + vec3;
            v4si vec7 = vec6 * (v4si){2, 2, 2, 2};
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f5, f6, f7, f8};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec3 * fvec1;
            
            v2df dvec1 = {d1, d2};
            v2df dvec2 = {d3, d4};
            v2df dvec3 = dvec1 + dvec2;
            v2df dvec4 = dvec3 * dvec1;
            
            /* Use vector results */
            int *vp = (int*)&vec7;
            result += vp[0] + vp[1] + vp[2] + vp[3];
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations after memory clobber */
            j = j + k + l + m + n + o + p + q + r + s + t + u + v + w + x + y + z;
            f1 = f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10;
            d1 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
            ll1 = ll1 + ll2 + ll3 + ll4 + ll5 + ll6;
            
            result += j + (int)f1 + (int)d1 + (int)ll1;
        }
        
        /* BLOCK C: Use rematerialization candidates again after high pressure region */
        /* This should trigger filter_old_remats to replace old candidates */
        result += cand1 * 2;
        result += *cand2 + 5;
        result += cand3 / 2;
        result += cand4 ^ 0xFF;
        
        /* Additional use with different addressing modes */
        int idx = (cand1 + i) % 100;
        result += local_array[idx];
    }
    
    return result;
}

/* Second test function with different pattern */
static int __attribute__((noinline))
test_remat2(int volatile varg1, int volatile varg2)
{
    int array[50];
    for (int i = 0; i < 50; i++) {
        array[i] = i * 3;
    }
    
    int result = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Create candidates */
        int *addr1 = &array[varg1 + i];
        int *addr2 = &array[varg2 + 10];
        int val1 = varg1 * i + varg2;
        int val2 = (varg1 << i) | (varg2 >> 2);
        
        /* Use immediately */
        result += *addr1 + *addr2 + val1 + val2;
        
        /* High pressure block */
        if (volatile_cond) {
            int t1 = i * 11, t2 = i * 13, t3 = i * 17, t4 = i * 19;
            int t5 = i * 23, t6 = i * 29, t7 = i * 31, t8 = i * 37;
            int t9 = i * 41, t10 = i * 43, t11 = i * 47, t12 = i * 53;
            int t13 = i * 59, t14 = i * 61, t15 = i * 67, t16 = i * 71;
            
            double dt1 = t1 * 0.1, dt2 = t2 * 0.2, dt3 = t3 * 0.3, dt4 = t4 * 0.4;
            double dt5 = t5 * 0.5, dt6 = t6 * 0.6, dt7 = t7 * 0.7, dt8 = t8 * 0.8;
            
            asm volatile("" ::: "memory");
            
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12 + t13 + t14 + t15 + t16;
            result += (int)(dt1 + dt2 + dt3 + dt4 + dt5 + dt6 + dt7 + dt8);
        }
        
        /* Use candidates again */
        result += *addr1 * 3;
        result += *addr2 - 5;
        result += val1 / 4;
        result += val2 ^ 0xAA;
    }
    
    return result;
}

int main(int argc, char **argv)
{
    /* Use volatile to prevent constant propagation */
    volatile int v1 = (argc > 1) ? atoi(argv[1]) : 7;
    volatile int v2 = (argc > 2) ? atoi(argv[2]) : 13;
    volatile int v3 = (argc > 3) ? atoi(argv[3]) : 19;
    volatile int v4 = (argc > 4) ? atoi(argv[4]) : 23;
    
    volatile int total = 0;
    int iterations = (argc > 5) ? atoi(argv[5]) : 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        volatile_arg = i;
        
        total += test_remat(v1 + i, v2 + i, v3 + i, v4 + i);
        total += test_remat2(v1 - i, v2 + i * 2);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}

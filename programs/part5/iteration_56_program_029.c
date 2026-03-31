/* test_early_remat.c - Program to trigger virtual register creation in GCC's early rematerialization pass */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int n, int *data) {
    /* Large immediate constants that may need rematerialization */
    const long long BIG_CONST1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long BIG_CONST2 = 0x5555555555555555LL;
    const unsigned long HUGE_ADDR = 0xDEADBEEFCAFEBABEUL;
    
    /* Many local variables with overlapping live ranges */
    register int r1 asm("ebx") = n;
    register int r2 asm("esi") = data[0];
    int a = r1 + 1;
    int b = r2 * 2;
    int c = a + b;
    int d = c * 3;
    int e = d - a;
    int f = e + b;
    int g = f * c;
    int h = g / (a + 1);
    
    /* Loop with invariant address calculations */
    int *invariant_ptr = &global_array[128];  /* Invariant pointer */
    double *dbl_ptr = &global_doubles[64];    /* Another invariant */
    
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Use invariants in multiple places with expensive constants */
        int idx1 = (i * (int)(BIG_CONST1 % 1000)) % 256;
        int idx2 = (i * (int)(BIG_CONST2 % 1000)) % 128;
        
        /* Multiple uses of invariants with overlapping computations */
        int val1 = invariant_ptr[idx1 % 128] * (int)(HUGE_ADDR % 100);
        double val2 = dbl_ptr[idx2] * (double)(BIG_CONST1 % 1000);
        int val3 = global_chars[(i * 7) % 512] * val1;
        
        /* Chain of computations keeping many values live */
        a = val1 + i;
        b = val3 - a;
        c = b * (int)val2;
        d = c + val1;
        e = d - b;
        f = e * a;
        g = f / (c + 1);
        h = g + val3;
        
        sum += a + b + c + d + e + f + g + h;
        
        /* Use invariants again in condition */
        if (sum > (int)(BIG_CONST2 % 10000)) {
            sum -= invariant_ptr[0] * 2;
        }
    }
    
    /* More computations after loop */
    int result = sum;
    result += a * b;
    result -= c / (d + 1);
    result += e * f;
    result -= g / (h + 1);
    result += (int)(BIG_CONST1 % 100);
    result -= (int)(BIG_CONST2 % 50);
    
    return result;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    
    /* Many register variables to force specific allocation */
    register int r1 asm("eax") = x;
    register int r2 asm("ebx") = y;
    register int r3 asm("ecx");
    register int r4 asm("edx");
    register int r5 asm("esi");
    register int r6 asm("edi");
    
    /* Complex inline assembly with multiple outputs and clobbers */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "movl %[y], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[a]\n\t"
        "imull $0x12345678, %%eax, %%ecx\n\t"
        "movl %%ecx, %[b]\n\t"
        "leal (%%eax,%%ebx,4), %%edx\n\t"
        "movl %%edx, %[c]"
        : [a] "=&r" (a), [b] "=&r" (b), [c] "=&r" (c)
        : [x] "r" (r1), [y] "r" (r2)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Another asm with different clobbers */
    asm volatile (
        "cpuid"
        : "=a" (r3), "=b" (r4), "=c" (r5), "=d" (r6)
        : "a" (0)
        : "cc"
    );
    
    /* Use all the results in overlapping computations */
    d = a + b + c + r3;
    e = d * r4;
    f = e - r5;
    g = f / (r6 + 1);
    h = g * a;
    i = h + b;
    j = i - c;
    k = j * r3;
    l = k + r4;
    m = l - r5;
    n = m / (r6 + 2);
    o = n * d;
    p = o + e;
    
    /* Chain of dependent computations */
    for (int idx = 0; idx < 10; idx++) {
        a = b + idx;
        b = c * a;
        c = d - b;
        d = e + c;
        e = f * d;
        f = g - e;
        g = h + f;
        h = i * g;
        i = j - h;
        j = k + i;
        k = l * j;
        l = m - k;
        m = n + l;
        n = o * m;
        o = p - n;
        p = a + o;
    }
    
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_control(int x, int y, int z) {
    /* Many temporaries with overlapping lives */
    int t1 = x * y;
    int t2 = y * z;
    int t3 = z * x;
    int t4 = t1 + t2;
    int t5 = t2 + t3;
    int t6 = t3 + t1;
    int t7 = t4 * t5;
    int t8 = t5 * t6;
    int t9 = t6 * t4;
    int t10 = t7 + t8;
    int t11 = t8 + t9;
    int t12 = t9 + t7;
    
    /* Labels for computed goto */
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    
    int result = 0;
    int counter = x;
    
    /* Outer loop */
    for (int i = 0; i < 100; i++) {
        /* Inner loop with switch */
        for (int j = 0; j < 10; j++) {
            switch ((i + j) % 6) {
                case 0:
                    t1 = t2 + t3;
                    t4 = t5 * t6;
                    result += t1 * t4;
                    break;
                case 1:
                    t2 = t3 - t1;
                    t5 = t6 / (t4 + 1);
                    result += t2 * t5;
                    break;
                case 2:
                    t3 = t1 + t2;
                    t6 = t4 - t5;
                    result += t3 * t6;
                    break;
                case 3:
                    t4 = t5 * t6;
                    t7 = t8 + t9;
                    result += t4 * t7;
                    break;
                case 4:
                    t5 = t6 / (t4 + 1);
                    t8 = t9 - t7;
                    result += t5 * t8;
                    break;
                case 5:
                    /* Computed goto */
                    goto *labels[counter % 6];
                    L0: t6 = t4 - t5; t9 = t7 * t8; result += t6 * t9; break;
                    L1: t7 = t8 + t9; t10 = t11 * t12; result += t7 * t10; break;
                    L2: t8 = t9 - t7; t11 = t12 / (t10 + 1); result += t8 * t11; break;
                    L3: t9 = t7 * t8; t12 = t10 + t11; result += t9 * t12; break;
                    L4: t10 = t11 * t12; t1 = t2 + t3; result += t10 * t1; break;
                    L5: t11 = t12 / (t10 + 1); t2 = t3 - t1; result += t11 * t2; break;
            }
            
            /* More computations keeping values live */
            t12 = t10 + t11 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
            counter = (counter * 13 + 17) % 1000;
        }
        
        /* Use global data with expensive address */
        result += global_array[i % 256] * (int)(0x7FFFFFFF12345678ULL % 1000);
    }
    
    return result + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12;
}

/* Function D: Mixed patterns for maximum coverage */
__attribute__((noinline, noclone))
int func_mixed_patterns(double *data, int n) {
    /* Use target-specific builtins if available */
    unsigned long long tsc1, tsc2;
    
    /* rdtsc uses eax and edx */
    asm volatile ("rdtsc" : "=a" ((uint32_t)(tsc1)), "=d" ((uint32_t)(tsc1 >> 32)));
    
    /* Many variables with complex expressions */
    double sum = 0.0;
    const double PI = 3.14159265358979323846;
    const double E = 2.71828182845904523536;
    const double BIG = 1.0e308;
    
    register double r1 asm("xmm0") = data[0];
    register double r2 asm("xmm1") = data[1];
    
    for (int i = 0; i < n; i++) {
        /* Complex floating point computations */
        double a = r1 * PI + i;
        double b = r2 * E - i;
        double c = a * b * BIG;
        double d = c / (a + b + 1.0);
        double e = d * PI * E;
        double f = e / BIG;
        double g = f + a - b;
        double h = g * c * d;
        
        /* Chain with many live values */
        r1 = h * 0.5;
        r2 = r1 * 0.25;
        
        sum += a + b + c + d + e + f + g + h + r1 + r2;
        
        /* Use inline asm in loop */
        asm volatile (
            "addsd %[x], %[y]\n\t"
            "mulsd %[y], %[x]"
            : [x] "+x" (r1), [y] "+x" (r2)
            :
            : "cc"
        );
    }
    
    asm volatile ("rdtsc" : "=a" ((uint32_t)(tsc2)), "=d" ((uint32_t)(tsc2 >> 32)));
    
    return (int)sum + (int)(tsc2 - tsc1);
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 7;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    for (int i = 0; i < 512; i++) {
        global_chars[i] = (i % 26) + 'A';
    }
    
    int test_data[100];
    for (int i = 0; i < 100; i++) {
        test_data[i] = i * i - 3 * i + 7;
    }
    
    double dbl_data[50];
    for (int i = 0; i < 50; i++) {
        dbl_data[i] = i * 0.7 + 2.3;
    }
    
    /* Call all test functions with arguments that create register pressure */
    int result = 0;
    
    /* Use large immediate arguments */
    result += func_loop_invariants(1000, test_data);
    result += func_asm_clobber(0x12345678, 0x9ABCDEF0);
    result += func_complex_control(42, 123, 789);
    result += func_mixed_patterns(dbl_data, 50);
    
    /* Add some more calls with different arguments */
    result += func_loop_invariants(500, &global_array[0]);
    result += func_asm_clobber(result, 987654321);
    
    return result % 1000;  /* Prevent overflow, ensure live result */
}

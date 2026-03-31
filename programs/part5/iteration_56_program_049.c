/* Test program to trigger early rematerialization virtual register creation */
#include <stdint.h>
#include <stdlib.h>

#define NOINLINE __attribute__((noinline, noclone))

/* Global arrays for address calculations */
static int global_array[1024];
static double global_double[512];
static char global_char[2048];

/* Function A: Loop with invariants and expensive constants */
NOINLINE static uint64_t test_loop_invariants(int iterations, int *base_ptr) {
    /* Large immediate constants that need rematerialization */
    const long long LARGE_CONST1 = 0x7FFFFFFFFFFFFFFFLL;
    const long long LARGE_CONST2 = 0x123456789ABCDEF0LL;
    const double PI = 3.14159265358979323846;
    
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = *base_ptr;
    int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, g = 6, h = 7;
    int i = 8, j = 9, k = 10, l = 11, m = 12, n = 13, o = 14, p = 15;
    double x = PI, y = x * 2.0, z = y * 3.0;
    uint64_t result = 0;
    
    /* Loop with invariant address calculations */
    for (int idx = 0; idx < iterations; idx++) {
        /* Use invariants in multiple places with different expressions */
        int offset1 = (idx * (int)LARGE_CONST1) % 1024;
        int offset2 = (idx * (int)LARGE_CONST2) % 512;
        int offset3 = (idx * idx) % 2048;
        
        /* Overlapping live ranges of many variables */
        a = global_array[offset1] + r0;
        b = global_array[offset2] + r1;
        c = a * b + (int)LARGE_CONST1;
        d = c - (int)(LARGE_CONST2 >> 32);
        e = d + global_char[offset3];
        f = e * 2 - (int)PI;
        g = f + idx;
        h = g * 3;
        
        /* Use all variables to keep them live */
        result += a + b + c + d + e + f + g + h;
        
        /* More variables with complex expressions */
        i = (result & 0xFF) + offset1;
        j = ((result >> 8) & 0xFF) + offset2;
        k = i * j + offset3;
        l = k - (int)LARGE_CONST1;
        m = l + (int)(LARGE_CONST2 & 0xFFFFFFFF);
        n = m * 2 + idx;
        o = n / 3;
        p = o + (int)PI;
        
        result += i + j + k + l + m + n + o + p;
        
        /* Use floating point invariants */
        x = PI * idx;
        y = x + (double)LARGE_CONST1 / 1.0e12;
        z = y * (double)LARGE_CONST2 / 1.0e12;
        result += (uint64_t)(x + y + z);
    }
    
    /* Conditional branch creating more complex live ranges */
    if (result > 1000000) {
        a = b + c + d;
        e = f + g + h;
        i = j + k + l;
        m = n + o + p;
        result += a + e + i + m + (int)LARGE_CONST1 + (int)LARGE_CONST2;
    }
    
    return result;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE static uint64_t test_inline_asm(int param1, int param2) {
    uint64_t result = 0;
    int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
    
    /* Register variables forcing specific allocation */
    register int reg_var1 asm("esi") = param1;
    register int reg_var2 asm("edi") = param2;
    register int reg_var3 asm("ecx");
    register int reg_var4 asm("edx");
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ebx\n\t"
        "addl %%eax, %%ebx\n\t"
        "movl %%ebx, %[out1]\n\t"
        "imull $0x12345678, %%eax, %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "leal (%%eax, %%ebx, 4), %%edx\n\t"
        "movl %%edx, %[out3]"
        : [out1] "=&r" (tmp1), [out2] "=&r" (tmp2), [out3] "=&r" (tmp3)
        : [in1] "r" (reg_var1), [in2] "r" (reg_var2)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    result += tmp1 + tmp2 + tmp3;
    
    /* More inline asm with different clobbers */
    asm volatile (
        "cpuid\n\t"
        : "=a" (tmp4), "=b" (tmp5), "=c" (tmp6)
        : "a" (0)
        : "edx"
    );
    
    result += tmp4 + tmp5 + tmp6;
    
    /* Complex expression using register variables and asm results */
    for (int i = 0; i < 100; i++) {
        reg_var3 = (reg_var1 * i) + (reg_var2 * (i + 1));
        reg_var4 = (tmp1 * i) + (tmp2 * (i + 1));
        
        /* Use builtins that use specific registers */
        unsigned int lo, hi;
        asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
        uint64_t tsc = ((uint64_t)hi << 32) | lo;
        
        result += reg_var3 + reg_var4 + (tsc & 0xFFF);
        
        /* More variables to increase register pressure */
        int t1 = reg_var3 * 2;
        int t2 = reg_var4 * 3;
        int t3 = t1 + t2;
        int t4 = t3 * i;
        int t5 = t4 + (int)tsc;
        int t6 = t5 - reg_var1;
        
        result += t1 + t2 + t3 + t4 + t5 + t6;
    }
    
    return result;
}

/* Function C: Complex control flow with switch and computed goto */
NOINLINE static uint64_t test_complex_control(int selector, int iterations) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5, &&label6, &&label7
    };
    
    /* Many local variables with overlapping scopes */
    int vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = i * selector;
    }
    
    uint64_t result = 0;
    int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Nested loops with switch inside */
    for (int outer = 0; outer < iterations; outer++) {
        for (int inner = 0; inner < 10; inner++) {
            /* Use computed goto for unpredictable control flow */
            if (inner % 3 == 0) {
                goto *jump_table[selector % 8];
            }
            
            label0:
            a = vars[0] + outer;
            b = vars[1] + inner;
            c = a * b + 0x7FFFFFFF;
            /* fall through */
            
            label1:
            d = c + vars[2];
            e = d * 2 - 0x12345678;
            /* fall through */
            
            label2:
            f = e + vars[3];
            g = f / 3 + 0xABCDEF;
            /* fall through */
            
            label3:
            h = g + vars[4];
            result += a + b + c + d + e + f + g + h;
            
            /* Switch statement creating complex CFG */
            switch ((outer + inner) % 5) {
                case 0:
                    a = b + c;
                    b = c + d;
                    c = d + e;
                    result += a * b * c;
                    break;
                case 1:
                    d = e + f;
                    e = f + g;
                    f = g + h;
                    result += d * e * f;
                    break;
                case 2:
                    g = h + a;
                    h = a + b;
                    a = b + c;
                    result += g * h * a;
                    break;
                case 3:
                    b = c + d;
                    c = d + e;
                    d = e + f;
                    result += b * c * d;
                    break;
                case 4:
                    e = f + g;
                    f = g + h;
                    g = h + a;
                    result += e * f * g;
                    break;
            }
            
            /* More arithmetic to extend live ranges */
            for (int k = 0; k < 5; k++) {
                int t1 = vars[k] + outer;
                int t2 = vars[k + 5] + inner;
                int t3 = t1 * t2 + 0xFFFFFFFF;
                int t4 = t3 - 0x88888888;
                int t5 = t4 * k + 0x44444444;
                result += t1 + t2 + t3 + t4 + t5;
            }
            
            continue;
            
            label4:
            label5:
            label6:
            label7:
            result += 0xDEADBEEF;
        }
    }
    
    return result;
}

/* Main function that calls all test cases */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 1024; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 512; i++) {
        global_double[i] = i * 1.5;
    }
    for (int i = 0; i < 2048; i++) {
        global_char[i] = i & 0xFF;
    }
    
    uint64_t total_result = 0;
    
    /* Call function A with large immediate and global address */
    total_result += test_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 1000,
        &global_array[500]
    );
    
    /* Call function B with register pressure */
    total_result += test_inline_asm(0x12345678, 0x9ABCDEF0);
    
    /* Call function C with complex control flow */
    total_result += test_complex_control(argc, 100);
    
    /* Use result to prevent optimization */
    return (int)(total_result % 0x7FFFFFFF);
}

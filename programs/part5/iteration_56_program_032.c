/* test_early_remat.c - Target specific patterns to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA};
static volatile int sink; /* Prevent dead code elimination */

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = data[0];
    register int r2 asm("ecx") = data[1];
    int sum = 0;
    int temp1, temp2, temp3, temp4, temp5;
    int counter1, counter2, counter3;
    
    /* Large immediate constants that need rematerialization */
    const long invariant1 = 0x7FFFFFFF12345678L;
    const long invariant2 = 0xFFFFFFFF87654321L;
    const int invariant3 = 0x1234ABCD;
    const int invariant4 = 0x5678EF90;
    
    /* Loop with invariant values used in multiple places */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in address calculations */
        temp1 = data[(i + invariant3) % 256];
        temp2 = data[(i * 2 + invariant4) % 256];
        
        /* Complex arithmetic with invariants */
        temp3 = (temp1 * invariant3) >> 3;
        temp4 = (temp2 * invariant4) >> 5;
        temp5 = (temp3 + temp4) * (invariant3 & 0xFFFF);
        
        /* Use invariants in conditions */
        if (temp5 > (invariant1 & 0xFFFFFFFF)) {
            sum += temp1 + (invariant4 % 100);
        } else {
            sum += temp2 + (invariant3 % 50);
        }
        
        /* More register pressure with temporaries */
        counter1 = (i * invariant3) % 100;
        counter2 = (i * invariant4) % 200;
        counter3 = (counter1 + counter2) * (invariant1 % 1000);
        
        /* Use all temporaries to keep them live */
        sum += counter1 + counter2 + counter3;
        
        /* Force register variable usage */
        asm volatile("" : "+r"(r0), "+r"(r1), "+r"(r2));
        r0 = sum % 1000;
        r1 = data[i % 256];
        r2 = (r1 * r0) + invariant3;
    }
    
    /* Overlapping live ranges continue */
    temp1 = sum + r0 + r1 + r2;
    temp2 = temp1 * invariant4;
    temp3 = temp2 / (invariant3 + 1);
    
    return temp3 + (invariant1 % 100);
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
long func_asm_clobber(int a, int b, int c) {
    long result1, result2, result3;
    register long reg_var1 asm("esi") = a * 1000L;
    register long reg_var2 asm("edi") = b * 2000L;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[input1], %%eax\n\t"
        "movl %[input2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "addl $0x12345678, %%ecx\n\t"
        "movl %%ecx, %[out3]"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2),
          [out3] "=&r" (result3)
        : [input1] "r" (a), 
          [input2] "r" (b)
        : "eax", "ebx", "ecx", "edx", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    long temp = result1 * 0x98765432L;
    temp += result2 * 0xFEDCBA09L;
    temp += result3 * 0x55555555L;
    
    /* Force register variable usage with assembly */
    asm volatile (
        "addl %%esi, %%eax\n\t"
        "addl %%edi, %%ebx"
        : "+a" (temp)
        : "S" (reg_var1), "D" (reg_var2)
        : "ebx", "cc"
    );
    
    /* Chain of operations keeping many values live */
    result1 = temp + reg_var1 + reg_var2;
    result2 = result1 * 0x7FFFFFFFFFFFFFFFL;
    result3 = result2 / (c + 1);
    
    return result1 + result2 + result3;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_control(int seed, int *data) {
    static void *labels[] = {&&L0, &&L1, &&L2, &&L3, &&L4};
    
    register int rval asm("ebp") = seed;
    int a = seed * 2, b = seed * 3, c = seed * 5;
    int d = seed * 7, e = seed * 11, f = seed * 13;
    int g = seed * 17, h = seed * 19, i = seed * 23;
    int result = 0;
    
    /* Large immediate constants */
    const long big_const = 0x123456789ABCDEF0L;
    const int med_const = 0x87654321;
    
    /* Nested loops with many temporaries */
    for (int outer = 0; outer < 10; outer++) {
        /* Switch inside loop creates complex control flow */
        switch (outer % 5) {
            case 0:
                a = (b + c) * (med_const % 100);
                d = (e + f) * (big_const % 200);
                break;
            case 1:
                b = (c + d) * (med_const % 150);
                e = (f + g) * (big_const % 250);
                break;
            case 2:
                c = (d + e) * (med_const % 200);
                f = (g + h) * (big_const % 300);
                break;
            case 3:
                d = (e + f) * (med_const % 250);
                g = (h + i) * (big_const % 350);
                break;
            case 4:
                /* Computed goto for unpredictable control flow */
                goto *labels[outer % 5];
        }
        
        L0: a += data[outer % 256] + (big_const & 0xFF);
        L1: b += data[(outer + 1) % 256] + (med_const & 0xFF);
        L2: c += data[(outer + 2) % 256] + ((big_const >> 8) & 0xFF);
        L3: d += data[(outer + 3) % 256] + ((med_const >> 8) & 0xFF);
        L4: e += data[(outer + 4) % 256] + ((big_const >> 16) & 0xFF);
        
        /* Use all temporaries to keep them live across basic blocks */
        result += a + b + c + d + e + f + g + h + i;
        
        /* Force register variable usage */
        asm volatile("" : "+r"(rval));
        rval = (rval * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Inner loop with more pressure */
        for (int inner = 0; inner < 5; inner++) {
            int t1 = a * inner + (big_const % 1000);
            int t2 = b * inner + (med_const % 500);
            int t3 = c * inner + ((big_const >> 24) & 0xFF);
            result += t1 + t2 + t3 + rval;
        }
    }
    
    return result;
}

/* Function D: Builtin usage for specific hard registers */
__attribute__((noinline, noclone))
uint64_t func_builtin_chain(int iterations) {
    uint64_t total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Use RDTSC which uses eax/edx hard registers */
        uint32_t eax, edx;
        asm volatile("rdtsc" : "=a"(eax), "=d"(edx));
        
        /* Chain operations keeping hard register results live */
        uint64_t ts1 = ((uint64_t)edx << 32) | eax;
        
        /* More builtin-like operations */
        register uint32_t reg_eax asm("eax");
        register uint32_t reg_edx asm("edx");
        reg_eax = eax * 0x9E3779B9;
        reg_edx = edx * 0x9E3779B9;
        
        /* Complex use of hard register results */
        uint64_t ts2 = ((uint64_t)reg_edx << 32) | reg_eax;
        uint64_t ts3 = ts1 * 0xDEADBEEFCAFEBABEULL;
        uint64_t ts4 = ts2 * 0x1234567890ABCDEFULL;
        
        /* Large immediate constants */
        const uint64_t magic1 = 0xF0F0F0F0F0F0F0F0ULL;
        const uint64_t magic2 = 0x0F0F0F0F0F0F0F0FULL;
        
        total += (ts1 & magic1) | (ts2 & magic2);
        total += ts3 ^ ts4;
        total += (reg_eax * magic1) + (reg_edx * magic2);
    }
    
    return total;
}

/* Main function to drive all patterns */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int result = 0;
    
    /* Call all test functions to trigger different patterns */
    result += func_loop_invariants(iterations, global_array);
    sink = result; /* Use sink to prevent optimization */
    
    result += func_asm_clobber(iterations % 100, 
                              (iterations * 2) % 100,
                              (iterations * 3) % 100);
    sink = result;
    
    result += func_complex_control(iterations, global_array);
    sink = result;
    
    uint64_t big_result = func_builtin_chain(iterations / 10);
    result += (int)(big_result & 0xFFFFFFFF) + (int)(big_result >> 32);
    
    /* Additional pressure with more calls */
    for (int i = 0; i < 5; i++) {
        result += func_loop_invariants(10, &global_array[i * 50]);
    }
    
    return result & 0xFF; /* Return non-zero result */
}

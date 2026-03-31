/* test-early-remat.c */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Prevent optimizations from simplifying our patterns */
#define NOINLINE __attribute__((noinline, noclone))
#define KEEP_ALIVE asm volatile("" : : "r"(result) : "memory")

/* Function A: Loop with invariants and expensive constants */
NOINLINE
unsigned long func_loop_invariants(int start, int end, int *data) {
    /* Large immediate constants that need rematerialization */
    const long EXPENSIVE_CONST1 = 0x7FFFFFFF12345678UL;
    const long EXPENSIVE_CONST2 = 0xFFFFFFFF87654321UL;
    const double PI = 3.14159265358979323846;
    
    /* Many local variables with overlapping live ranges */
    register long r1 asm("ebx") = start;
    register long r2 asm("esi") = end;
    long sum = 0;
    long temp1, temp2, temp3, temp4, temp5;
    double dtemp1, dtemp2;
    int *ptr1, *ptr2;
    
    /* Use invariants in address calculations */
    ptr1 = &global_array[0];
    ptr2 = &global_array[128];
    
    /* Complex loop with many live values */
    for (int i = start; i < end; i++) {
        /* Use expensive constants multiple times */
        temp1 = (r1 * EXPENSIVE_CONST1) >> 3;
        temp2 = (r2 * EXPENSIVE_CONST2) >> 5;
        
        /* Use invariants in different places */
        temp3 = ptr1[i & 0x7F] * temp1;
        temp4 = ptr2[i & 0x7F] * temp2;
        
        /* More computations keeping values live */
        dtemp1 = PI * i;
        dtemp2 = dtemp1 * global_doubles[i & 0x7F];
        
        temp5 = (long)(dtemp2 * 1000.0);
        
        /* Overlapping live ranges */
        sum += temp1 + temp2 + temp3 + temp4 + temp5;
        
        /* Modify register variables to prevent optimization */
        r1 = (r1 * 1103515245 + 12345) & 0x7FFFFFFF;
        r2 = (r2 * 1103515245 + 54321) & 0x7FFFFFFF;
    }
    
    /* Use all variables again to extend live ranges */
    sum += r1 + r2 + (long)(PI * 1000);
    
    return sum;
}

/* Function B: Inline assembly with clobbered registers */
NOINLINE
uint64_t func_asm_clobber(uint64_t input) {
    uint64_t result = 0;
    uint32_t low, high;
    
    /* Register variables forcing specific allocation */
    register uint32_t r_eax asm("eax");
    register uint32_t r_ebx asm("ebx");
    register uint32_t r_ecx asm("ecx");
    register uint32_t r_edx asm("edx");
    register uint32_t r_esi asm("esi");
    register uint32_t r_edi asm("edi");
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[in_low], %%eax\n\t"
        "movl %[in_high], %%edx\n\t"
        "xorl %%ebx, %%ebx\n\t"
        "xorl %%ecx, %%ecx\n\t"
        "xorl %%esi, %%esi\n\t"
        "xorl %%edi, %%edi\n\t"
        "1:\n\t"
        "addl $0x12345678, %%eax\n\t"
        "adcl $0x9ABCDEF0, %%edx\n\t"
        "movl %%eax, %%ebx\n\t"
        "movl %%edx, %%ecx\n\t"
        "xorl %%esi, %%ebx\n\t"
        "xorl %%edi, %%ecx\n\t"
        "roll $5, %%ebx\n\t"
        "roll $13, %%ecx\n\t"
        "addl %%ebx, %%esi\n\t"
        "addl %%ecx, %%edi\n\t"
        "decl %%eax\n\t"
        "jnz 1b\n\t"
        "movl %%esi, %[out_low]\n\t"
        "movl %%edi, %[out_high]"
        : [out_low] "=&r" (low), [out_high] "=&r" (high)
        : [in_low] "r" ((uint32_t)input), [in_high] "r" ((uint32_t)(input >> 32))
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc", "memory"
    );
    
    /* Use the results in subsequent expressions */
    r_eax = low;
    r_edx = high;
    r_ebx = r_eax ^ 0xDEADBEEF;
    r_ecx = r_edx ^ 0xCAFEBABE;
    r_esi = r_ebx + r_ecx;
    r_edi = r_ebx - r_ecx;
    
    /* Chain of hard register references */
    result = ((uint64_t)r_edx << 32) | r_eax;
    result += ((uint64_t)r_edi << 32) | r_esi;
    result += ((uint64_t)r_ecx << 32) | r_ebx;
    
    return result;
}

/* Function C: Complex control flow with switch */
NOINLINE
int func_complex_control(int seed, int iterations) {
    /* Many scalar temporaries with overlapping lives */
    int a = seed * 1103515245 + 12345;
    int b = seed * 1664525 + 1013904223;
    int c = seed * 214013 + 2531011;
    int d = seed * 134775813 + 1;
    int e = seed * 32767 + 65537;
    int f = seed * 16807 + 0;
    int g = seed * 48271 + 0;
    int h = seed * 69621 + 0;
    
    int result = 0;
    
    /* Labels for computed goto */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3, 
                     &&label4, &&label5, &&label6, &&label7 };
    
    for (int i = 0; i < iterations; i++) {
        /* Use all variables to keep them live */
        int selector = (a + b + c + d + e + f + g + h) & 0x7;
        
        /* Computed goto creates complex control flow */
        goto *labels[selector];
        
    label0:
        a = (a ^ b) + c;
        b = (b ^ c) + d;
        c = (c ^ d) + e;
        result += a;
        continue;
        
    label1:
        d = (d ^ e) + f;
        e = (e ^ f) + g;
        f = (f ^ g) + h;
        result += b;
        continue;
        
    label2:
        g = (g ^ h) + a;
        h = (h ^ a) + b;
        a = (a ^ b) + c;
        result += c;
        continue;
        
    label3:
        b = (b ^ c) + d;
        c = (c ^ d) + e;
        d = (d ^ e) + f;
        result += d;
        continue;
        
    label4:
        e = (e ^ f) + g;
        f = (f ^ g) + h;
        g = (g ^ h) + a;
        result += e;
        continue;
        
    label5:
        h = (h ^ a) + b;
        a = (a ^ b) + c;
        b = (b ^ c) + d;
        result += f;
        continue;
        
    label6:
        c = (c ^ d) + e;
        d = (d ^ e) + f;
        e = (e ^ f) + g;
        result += g;
        continue;
        
    label7:
        f = (f ^ g) + h;
        g = (g ^ h) + a;
        h = (h ^ a) + b;
        result += h;
        continue;
    }
    
    /* Final use of all variables */
    result += a + b + c + d + e + f + g + h;
    
    return result;
}

/* Main function to drive everything */
int main(int argc, char **argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 0.12345;
    }
    for (int i = 0; i < 512; i++) {
        global_chars[i] = i & 0xFF;
    }
    
    /* Call test functions with arguments that create register pressure */
    unsigned long result1 = func_loop_invariants(0, 100, global_array);
    uint64_t result2 = func_asm_clobber(0x123456789ABCDEF0UL);
    int result3 = func_complex_control(42, 1000);
    
    /* Combine results to prevent dead code elimination */
    unsigned long final_result = result1 + (result2 & 0xFFFFFFFF) + 
                                 (result2 >> 32) + result3;
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(final_result));
    
    return (int)(final_result & 0x7FFFFFFF);
}

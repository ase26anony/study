/* test-early-remat.c */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x12345678, 0x9ABCDEF0, 0x11112222, 0x33334444,
    0x55556666, 0x77778888, 0x9999AAAA, 0xBBBBCCCC
};

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Large immediate values that need rematerialization */
    const long expensive_const1 = 0x7FFFFFFF12345678UL;
    const long expensive_const2 = 0xFFFFFFFF87654321UL;
    const int *array_end = data + 256;  /* Invariant pointer */
    
    int sum = 0;
    volatile int *volatile_ptr = data;  /* Force memory accesses */
    
    /* Loop with invariant values used in multiple places */
    for (int i = 0; i < iterations; i++) {
        /* Use expensive constants in different, non-adjacent calculations */
        long temp1 = expensive_const1 + i;
        sum += (int)(temp1 & 0xFFFF);
        
        /* Use invariant pointer in address calculation */
        int val = volatile_ptr[i % 256];
        sum += val;
        
        /* Another use of expensive constant */
        long temp2 = expensive_const2 - i;
        sum += (int)(temp2 & 0xFFFF);
        
        /* Use invariant in condition */
        if (data + (i % 256) < array_end) {
            sum += volatile_ptr[(i + 1) % 256];
        }
        
        /* More uses of the same constants */
        sum += (int)((expensive_const1 >> 32) + (expensive_const2 >> 32));
    }
    
    return sum;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b) {
    int result1, result2, result3;
    
    /* Use register variables to encourage hard register allocation */
    register int reg_var1 asm("ebx") = a;
    register int reg_var2 asm("esi") = b;
    register int reg_var3 asm("edi") = a * b;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %[in1], %%eax\n\t"
        "movl %[in2], %%ecx\n\t"
        "imull %%ecx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ecx, 2), %%edx\n\t"
        "movl %%edx, %[out2]\n\t"
        "addl %%ebx, %%esi\n\t"
        "movl %%esi, %[out3]\n\t"
        : [out1] "=&r" (result1), 
          [out2] "=&r" (result2),
          [out3] "=&r" (result3)
        : [in1] "r" (reg_var1),
          [in2] "r" (reg_var2)
        : "eax", "ecx", "edx", "ebx", "esi", "edi", 
          "memory", "cc"
    );
    
    /* Use results in complex expressions to extend live ranges */
    int sum = result1;
    for (int i = 0; i < 100; i++) {
        sum += result2 * i - result3;
        sum += reg_var1 + reg_var2;  /* Use register variables */
    }
    
    return sum;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int func_complex_control(int selector, int count) {
    /* Many local variables with overlapping lifetimes */
    int a = selector * 2;
    int b = selector + 1000;
    int c = selector - 500;
    int d = selector * selector;
    int e = selector | 0xFF00;
    int f = selector & 0x0F0F;
    int g = selector ^ 0x1234;
    int h = ~selector;
    
    /* Use computed goto for unpredictable control flow */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    goto *labels[selector % 4];
    
label0:
    a = b + c;
    /* Fall through */
label1:
    d = e * f;
    for (int i = 0; i < count; i++) {
        /* All variables live across loop iterations */
        a += i;
        b -= i;
        c *= (i + 1);
        d /= (i + 2);
        e |= i;
        f &= i;
        g ^= i;
        h = ~i;
    }
    goto end;
    
label2:
    /* Switch inside loop creates complex dataflow */
    for (int i = 0; i < count; i++) {
        switch ((a + i) % 5) {
            case 0: a += b; break;
            case 1: b += c; break;
            case 2: c += d; break;
            case 3: d += e; break;
            case 4: e += f; break;
        }
        g = h + a + b + c + d + e + f;
    }
    goto end;
    
label3:
    /* Nested loops with many live values */
    for (int i = 0; i < count; i++) {
        int temp = a;
        for (int j = 0; j < 10; j++) {
            temp += b + c + d + e + f + g + h + i + j;
        }
        a = temp;
    }
    
end:
    /* Combine all variables to ensure they're live */
    return a + b + c + d + e + f + g + h;
}

/* Function D: Target-specific builtins and hard register references */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t func_builtins(void) {
    /* Use RDTSC which returns in eax:edx */
    uint32_t eax_low, edx_high;
    asm volatile ("rdtsc" : "=a" (eax_low), "=d" (edx_high));
    
    /* Create chains of hard register references */
    uint64_t cycle1 = ((uint64_t)edx_high << 32) | eax_low;
    
    /* Use the result in subsequent expressions */
    uint64_t sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Force use of the timestamp in calculations */
        sum += cycle1 + i;
        
        /* Another RDTSC to create more hard register pressure */
        asm volatile ("rdtsc" : "=a" (eax_low), "=d" (edx_high));
        uint64_t cycle2 = ((uint64_t)edx_high << 32) | eax_low;
        sum += cycle2 - i;
    }
    
    return sum;
}
#endif

/* Main function to call all test patterns */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    int total = 0;
    
    /* Call each function with arguments that create register pressure */
    total += func_loop_invariants(1000, global_array);
    total += func_asm_clobber(0x123456, 0x789ABCD);
    total += func_complex_control(42, 500);
    
    #ifdef __i386__
    total += (int)func_builtins();
    #endif
    
    /* Use large immediate values as function arguments */
    total += func_loop_invariants(100, &global_array[128]);
    total += func_asm_clobber(0x7FFFFFFF, 0x80000000);
    
    return total & 0xFF;  /* Prevent dead code elimination */
}

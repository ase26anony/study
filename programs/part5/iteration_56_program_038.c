/* test-early-remat.c */
/* Compile with: gcc -O2 -m32 -fno-optimize-sibling-calls -fdump-rtl-early-remat test-early-remat.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA};

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping lifetimes */
    register int a asm("eax") = iterations * 2;
    register int b asm("ebx") = iterations + 0x7FFFFFFF; /* Large immediate */
    int c = *data;
    int d = c + 0x1234567; /* Non-encodable immediate */
    int e = d * 3;
    int f = e - 0x89ABCDEF; /* Another large immediate */
    int g = f / 5;
    int h = g ^ 0x55555555;
    int i = h | 0xAAAAAAAA;
    int j = i & 0x33333333;
    int k = j << 3;
    int l = k >> 1;
    int m = l + 0x11111111;
    int n = m - 0x22222222;
    int o = n * 0x33333333; /* Will overflow, but creates register pressure */
    
    /* Loop with invariant address calculation using global data */
    int sum = 0;
    const int *invariant_ptr = &global_array[128]; /* Invariant pointer */
    const long *invariant_const = &large_constants[2]; /* Another invariant */
    
    for (int idx = 0; idx < iterations; idx++) {
        /* Use invariants in multiple places with different operations */
        int addr1 = (int)(invariant_ptr + idx);
        int addr2 = (int)(invariant_ptr - idx);
        
        /* Use the invariants in complex expressions */
        sum += *(invariant_ptr + (idx % 64)) * (int)(*invariant_const);
        sum += *(invariant_ptr - (idx % 32)) * (idx & 0xFF);
        
        /* More register pressure inside loop */
        int t1 = a + b + c;
        int t2 = d + e + f;
        int t3 = g + h + i;
        int t4 = j + k + l;
        int t5 = m + n + o;
        
        sum += t1 + t2 + t3 + t4 + t5;
        
        /* Modify some variables to extend live ranges */
        a += idx;
        b -= idx;
        c ^= idx;
        d |= addr1;
        e &= addr2;
    }
    
    /* Use all variables again to keep them live */
    return sum + a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    int result1, result2, result3;
    
    /* Multi-output inline assembly with specific register constraints */
    asm volatile (
        "movl %3, %%eax\n\t"
        "movl %4, %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "leal (%%eax, %%ebx, 2), %%ecx\n\t"
        "movl %%ecx, %1\n\t"
        "addl $0x12345678, %%ecx\n\t"  /* Large immediate */
        "movl %%ecx, %2\n\t"
        : "=&r" (result1), "=&r" (result2), "=&r" (result3)  /* Early clobber outputs */
        : "r" (x), "r" (y)
        : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "memory", "cc"
    );
    
    /* Use results in complex expressions with more register variables */
    register int r1 asm("esi") = result1;
    register int r2 asm("edi") = result2;
    
    /* Chain of operations using register variables */
    for (int i = 0; i < 16; i++) {
        /* Force spilling by using many temporaries */
        int t1 = r1 + 0x11111111;
        int t2 = r2 - 0x22222222;
        int t3 = t1 * t2;
        int t4 = t3 / (i + 1);
        int t5 = t4 ^ 0x33333333;
        int t6 = t5 | 0x44444444;
        int t7 = t6 & 0x55555555;
        
        r1 = t3 + t4;
        r2 = t5 + t6 + t7;
        
        /* Use global data address */
        r1 += (int)(&global_array[i]) & 0xFFFF;
    }
    
    return r1 + r2 + result3;
}

/* Function C: Complex control flow with switch */
__attribute__((noinline, noclone))
int func_complex_cf(int seed, int *data) {
    int a = seed + 0x12345678;
    int b = a * 0x9ABCDEF;
    int c = b / 3;
    int d = c ^ 0x55555555;
    int e = d | 0xAAAAAAAA;
    int f = e & 0x33333333;
    int g = f << 4;
    int h = g >> 2;
    
    int total = 0;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 16; j++) {
            int index = (i * 16 + j) % 256;
            
            /* Switch creates complex control flow */
            switch (index % 8) {
                case 0:
                    total += a + global_array[index];
                    a += 0x11111111;  /* Large immediate */
                    break;
                case 1:
                    total += b - global_array[index];
                    b -= 0x22222222;
                    break;
                case 2:
                    total += c * global_array[index];
                    c ^= 0x33333333;
                    break;
                case 3:
                    total += d | global_array[index];
                    d |= 0x44444444;
                    break;
                case 4:
                    total += e & global_array[index];
                    e &= 0x55555555;
                    break;
                case 5:
                    total += f + (int)(&global_array[index]);
                    f += 0x66666666;
                    break;
                case 6:
                    total += g - (int)(&global_array[index]);
                    g -= 0x77777777;
                    break;
                case 7:
                    total += h * (int)(&global_array[index]);
                    h *= 0x88888888;
                    break;
            }
            
            /* Use all variables in each iteration to keep them live */
            total += (a + b + c + d + e + f + g + h) % 256;
        }
    }
    
    return total;
}

/* Function D: Using builtins for specific register references */
#ifdef __i386__
__attribute__((noinline, noclone))
uint64_t func_builtin_register(void) {
    /* rdtsc uses eax and edx specifically */
    uint64_t tsc1 = __builtin_ia32_rdtsc();
    
    /* Chain operations using the result */
    uint32_t low = (uint32_t)tsc1;
    uint32_t high = (uint32_t)(tsc1 >> 32);
    
    /* Force register pressure with many operations */
    uint32_t x1 = low + 0x12345678;
    uint32_t x2 = high - 0x9ABCDEF;
    uint32_t x3 = x1 * x2;
    uint32_t x4 = x3 / 0x1111;
    uint32_t x5 = x4 ^ 0x55555555;
    uint32_t x6 = x5 | 0xAAAAAAAA;
    uint32_t x7 = x6 & 0x33333333;
    uint32_t x8 = x7 << 3;
    uint32_t x9 = x8 >> 1;
    uint32_t x10 = x9 + 0x77777777;
    
    /* Another rdtsc to create more hard register references */
    uint64_t tsc2 = __builtin_ia32_rdtsc();
    
    /* Mix results */
    uint64_t result = ((uint64_t)(x10 + (uint32_t)tsc2) << 32) | 
                     (x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10);
    
    return result + tsc1;
}
#endif

/* Main function to call all test cases */
int main(void) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    int test_data[64];
    for (int i = 0; i < 64; i++) {
        test_data[i] = i * 5 - 2;
    }
    
    int result = 0;
    
    /* Call all test functions to trigger different patterns */
    result += func_loop_invariants(100, test_data);
    result += func_asm_clobber(0x12345678, 0x9ABCDEF);
    result += func_complex_cf(42, test_data);
    
    #ifdef __i386__
    result += (int)func_builtin_register();
    #endif
    
    /* Use computed goto for additional control flow complexity */
    void* labels[] = { &&label1, &&label2, &&label3 };
    
    int goto_var = result % 3;
    goto *labels[goto_var];
    
label1:
    result += 0x11111111;
    goto end;
    
label2:
    result += 0x22222222;
    goto end;
    
label3:
    result += 0x33333333;
    goto end;
    
end:
    return result & 0x7FFFFFFF; /* Ensure positive return */
}

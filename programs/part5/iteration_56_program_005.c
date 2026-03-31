/* test-early-remat.c
 * Designed to trigger virtual register creation in GCC's early rematerialization pass
 * Compile with: gcc -O2 -m32 -fno-optimize-sibling-calls -fdump-rtl-early-remat test-early-remat.c -o test
 */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static double global_doubles[128];
static char global_chars[512];

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int function_a(int iterations, int* data) {
    /* Large immediate constants that may need rematerialization */
    const long long expensive_const1 = 0x123456789ABCDEF0LL;
    const long long expensive_const2 = 0xFEDCBA9876543210LL;
    const double expensive_float = 3.14159265358979323846 * 1e6;
    
    /* Loop invariants that will be used in multiple places */
    int* invariant_ptr1 = &global_array[64];
    double* invariant_ptr2 = &global_doubles[32];
    char* invariant_ptr3 = &global_chars[128];
    
    int sum = 0;
    
    /* Complex loop with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in multiple calculations */
        int val1 = *invariant_ptr1 + i;
        double val2 = *invariant_ptr2 * expensive_float;
        char val3 = *(invariant_ptr3 + i);
        
        /* Use expensive constants in non-adjacent operations */
        if ((i & 0xFF) == (expensive_const1 & 0xFF)) {
            sum += val1 * (expensive_const2 & 0xFFFF);
        }
        
        if (val2 > 1000.0) {
            sum += (int)val2 + (expensive_const1 >> 32);
        }
        
        sum += val3 * ((expensive_const2 >> 16) & 0xFF);
        
        /* More operations to extend live ranges */
        sum += data[i] * (expensive_const1 & 0xFFFFFFFF);
        sum += (int)(*invariant_ptr2 * i);
    }
    
    /* Additional basic block with same invariants */
    if (sum > 1000000) {
        sum += *invariant_ptr1 * (expensive_const2 & 0xFF);
        sum += (int)(*invariant_ptr2 * expensive_float);
    }
    
    return sum;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_b(int x, int y) {
    /* Register variables to force specific allocation */
    register int r1 asm("eax") = x;
    register int r2 asm("ebx") = y;
    register int r3 asm("ecx");
    register int r4 asm("edx");
    register int r5 asm("esi");
    register int r6 asm("edi");
    
    int result = 0;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "imull %%ebx, %%eax\n\t"
        : "=&r" (r3), "=&r" (r4)
        : "r" (r1), "r" (r2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    r5 = r3 * r4 + 0x7FFFFFFF;
    r6 = r5 - 0x80000000;
    
    /* More inline assembly with different constraints */
    asm volatile (
        "cpuid\n\t"
        : "=a" (r1), "=b" (r2), "=c" (r3), "=d" (r4)
        : "a" (0)
    );
    
    /* Chain hard register references */
    result = r1 + r2 + r3 + r4 + r5 + r6;
    
    /* Use builtins that return in specific registers */
    uint64_t tsc = __builtin_ia32_rdtsc();
    result += (tsc & 0xFFFFFFFF) + (tsc >> 32);
    
    return result;
}

/* Function C: Complex control flow with many temporaries */
__attribute__((noinline, noclone))
int function_c(int mode, int count) {
    /* Many local variables with overlapping live ranges */
    int a = 0x12345678;
    int b = 0x9ABCDEF0;
    int c = 0x13579BDF;
    int d = 0x2468ACE0;
    int e = 0x55555555;
    int f = 0xAAAAAAAA;
    int g = 0x33333333;
    int h = 0xCCCCCCCC;
    
    int result = 0;
    
    /* Nested loops with switch inside */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 10; j++) {
            /* Switch creates complex control flow */
            switch ((i + j) % 8) {
                case 0:
                    result += a * b + c;
                    a = (a << 1) | (a >> 31);
                    break;
                case 1:
                    result += b * c + d;
                    b = (b >> 1) | (b << 31);
                    break;
                case 2:
                    result += c * d + e;
                    c ^= d ^ e;
                    break;
                case 3:
                    result += d * e + f;
                    d += e + f;
                    break;
                case 4:
                    result += e * f + g;
                    e = ~e;
                    break;
                case 5:
                    result += f * g + h;
                    f = f * 0x5A827999;
                    break;
                case 6:
                    result += g * h + a;
                    g = g ^ h ^ a;
                    break;
                case 7:
                    result += h * a + b;
                    h = h - a + b;
                    break;
            }
            
            /* More operations to extend live ranges */
            result += (a & b) | (c & d);
            result += (e ^ f) ^ (g ^ h);
        }
        
        /* Conditional with many live values */
        if (i % 3 == 0) {
            result += a + b + c + d;
        } else if (i % 3 == 1) {
            result += e + f + g + h;
        } else {
            result += (a + e) * (b + f);
        }
    }
    
    /* Computed goto for even more complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    
    for (int i = 0; i < 4; i++) {
        goto *labels[mode % 4];
        
    label1:
        result += a * 0x10001;
        continue;
    label2:
        result += b * 0x20002;
        continue;
    label3:
        result += c * 0x30003;
        continue;
    label4:
        result += d * 0x40004;
        continue;
    }
    
    return result;
}

/* Function D: Mixed patterns for maximum coverage */
__attribute__((noinline, noclone))
int function_d(int seed) {
    /* Use all patterns together */
    int temp = seed;
    
    /* Loop with invariants */
    const long long big_const = 0x1234567890ABCDEFLL;
    int* ptr = &global_array[0];
    
    for (int i = 0; i < 100; i++) {
        temp += *ptr + (big_const & 0xFFFFFFFF);
        temp += (big_const >> 32) * i;
    }
    
    /* Inline assembly */
    register int r asm("eax") = temp;
    asm volatile (
        "roll $13, %0\n\t"
        : "+r" (r)
        :
        : "cc"
    );
    
    /* Complex control flow */
    switch (r % 5) {
        case 0: temp = r * 0x11111111; break;
        case 1: temp = r * 0x22222222; break;
        case 2: temp = r * 0x33333333; break;
        case 3: temp = r * 0x44444444; break;
        case 4: temp = r * 0x55555555; break;
    }
    
    return temp;
}

/* Main function to drive everything */
int main(int argc, char** argv) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    for (int i = 0; i < 128; i++) {
        global_doubles[i] = i * 1.5;
    }
    
    for (int i = 0; i < 512; i++) {
        global_chars[i] = i & 0xFF;
    }
    
    /* Call test functions with arguments that create register pressure */
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    
    int result = 0;
    
    /* Each function creates different patterns that might trigger
       early rematerialization's virtual register creation */
    result += function_a(iterations, global_array);
    result += function_b(0x12345678, 0x9ABCDEF0);
    result += function_c(iterations % 10, 50);
    result += function_d(result);
    
    /* Use result to prevent dead code elimination */
    return result & 0x7FFFFFFF;
}

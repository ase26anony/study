/* test-early-remat.c */
#include <stdint.h>
#include <stdlib.h>

/* Global arrays to force address calculations */
static int global_array1[1024];
static int global_array2[1024];
static int global_array3[1024];
static const long large_constants[] = {0x7FFFFFFF, 0x80000000, 0x12345678, 0x9ABCDEF0};

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int offset) {
    volatile int sum = 0;
    /* Use invariant pointers and large constants */
    int *invariant_ptr1 = global_array1;
    int *invariant_ptr2 = global_array2;
    const long invariant_const = 0x123456789ABCDEF0LL;
    
    /* Many local variables with overlapping live ranges */
    int a = offset * 0x10001;
    int b = offset * 0x20002;
    int c = offset * 0x30003;
    int d = offset * 0x40004;
    int e = offset * 0x50005;
    int f = offset * 0x60006;
    int g = offset * 0x70007;
    int h = offset * 0x80008;
    
    for (int i = 0; i < iterations; i++) {
        /* Use invariants in address calculations */
        int idx1 = (i * invariant_const) % 1024;
        int idx2 = (i * 0x7FFFFFFF) % 1024;  /* Large immediate */
        int idx3 = (i * 0x80000000) % 1024;  /* Another large immediate */
        
        /* Complex expressions with many live values */
        a = invariant_ptr1[idx1] + b + 0x12345678;
        b = invariant_ptr2[idx2] + c + 0x9ABCDEF0;
        c = global_array3[idx3] + d + large_constants[i % 4];
        d = a * b + 0x11111111;
        e = b * c + 0x22222222;
        f = c * d + 0x33333333;
        g = d * e + 0x44444444;
        h = e * f + 0x55555555;
        
        /* Use all variables to keep them live */
        sum += a + b + c + d + e + f + g + h;
        
        /* More operations to extend live ranges */
        a = (a << 3) | 0xFF;
        b = (b >> 2) + 0xAAAA;
        c = (c & 0xFFFF) * 0x10001;
        d = (d ^ 0x55555555) + i;
    }
    
    /* Force all variables to be used at the end */
    return sum + a + b + c + d + e + f + g + h;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int x, int y) {
    int result1, result2, result3;
    /* Declare register variables to encourage hard register allocation */
    register int r1 asm("eax") = x;
    register int r2 asm("ebx") = y;
    register int r3 asm("ecx") = x * y;
    register int r4 asm("edx") = x + y;
    
    /* Multi-output inline assembly with many clobbered registers */
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "movl %[r2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "addl $0x12345678, %%ecx\n\t"
        "movl %%ecx, %[out3]"
        : [out1] "=&r" (result1),
          [out2] "=&r" (result2),
          [out3] "=&r" (result3)
        : [r1] "r" (r1),
          [r2] "r" (r2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use register variables in complex expressions */
    r3 = (r1 * 0x10001) + (r2 * 0x20002);
    r4 = (r1 & 0xFFFF0000) | (r2 & 0x0000FFFF);
    
    /* More inline asm with different constraints */
    asm volatile (
        "cpuid"
        : "=a" (r1), "=b" (r2), "=c" (r3), "=d" (r4)
        : "a" (0)
        : "cc"
    );
    
    return result1 + result2 + result3 + r1 + r2 + r3 + r4;
}

/* Function C: Complex control flow with switch and computed goto */
__attribute__((noinline, noclone))
int func_complex_flow(int start, int count) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    
    int val = start;
    int total = 0;
    
    /* Many temporary variables with overlapping lives */
    int t1 = val * 0x101;
    int t2 = val * 0x202;
    int t3 = val * 0x303;
    int t4 = val * 0x404;
    int t5 = val * 0x505;
    int t6 = val * 0x606;
    int t7 = val * 0x707;
    int t8 = val * 0x808;
    
    for (int i = 0; i < count; i++) {
        /* Switch with multiple cases - creates complex control flow */
        switch (i % 5) {
            case 0:
                t1 = (t1 << 1) + 0x11111111;
                t2 = (t2 >> 1) | 0x80000000;
                break;
            case 1:
                t3 = t1 * t2 + 0x22222222;
                t4 = t2 * t3 + 0x33333333;
                break;
            case 2:
                t5 = (t3 ^ t4) & 0xAAAAAAAA;
                t6 = (t4 ^ t5) | 0x55555555;
                break;
            case 3:
                t7 = t5 + t6 * 0x10001;
                t8 = t6 + t7 * 0x20002;
                break;
            case 4:
                /* Computed goto for even more complex flow */
                goto *labels[i % 5];
                label0:
                    t1 += large_constants[0];
                    break;
                label1:
                    t2 += large_constants[1];
                    break;
                label2:
                    t3 += large_constants[2];
                    break;
                label3:
                    t4 += large_constants[3];
                    break;
                label4:
                    t5 += 0xFFFFFFFF;
                    break;
        }
        
        /* Use all temporaries to keep them live across iterations */
        total += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
        
        /* Additional nested loop to create more pressure */
        for (int j = 0; j < 3; j++) {
            int inner = (t1 + j) * (t2 - j);
            inner += (t3 * j) / (j + 1);
            inner |= (t4 << j) & (0xFF << (j * 8));
            total += inner + 0x12345678;
        }
    }
    
    return total;
}

/* Function D: Mix of all patterns */
__attribute__((noinline, noclone))
int func_mixed_patterns(int x) {
    /* Register variables with specific registers */
    register int acc1 asm("esi") = x;
    register int acc2 asm("edi") = x * 2;
    
    /* Large constants used multiple times */
    const long big_const1 = 0x7FFFFFFFFFFFFFFFLL;
    const long big_const2 = 0x8000000000000000LL;
    
    int array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = i * 0x10000001;
    }
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* Use invariants and large constants */
        int idx = (i * big_const1) % 8;
        int idx2 = (i * big_const2) % 8;
        
        /* Complex expressions */
        acc1 = (acc1 * 0x12345678) + array[idx];
        acc2 = (acc2 / 0x10001) - array[idx2];
        
        /* Inline asm in the middle */
        int temp;
        asm volatile (
            "movl %[acc1], %%esi\n\t"
            "movl %[acc2], %%edi\n\t"
            "addl %%esi, %%edi\n\t"
            "movl %%edi, %[temp]"
            : [temp] "=r" (temp)
            : [acc1] "r" (acc1), [acc2] "r" (acc2)
            : "esi", "edi", "cc"
        );
        
        sum += temp + acc1 + acc2 + i * 0x11111111;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 1024; i++) {
        global_array1[i] = i * 3;
        global_array2[i] = i * 5;
        global_array3[i] = i * 7;
    }
    
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int result = 0;
    
    /* Call all test functions to trigger different patterns */
    result += func_loop_invariants(iterations, 1);
    result += func_asm_clobber(iterations, 2);
    result += func_complex_flow(iterations, 50);
    result += func_mixed_patterns(iterations);
    
    /* Use result to prevent optimization */
    return result > 0 ? 0 : 1;
}

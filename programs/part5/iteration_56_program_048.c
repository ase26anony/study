/* test_early_remat.c - Target specific patterns to trigger virtual register creation in early rematerialization */

#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA};
static volatile int sink; /* Prevent dead code elimination */

/* Function A: Loop with invariants and high register pressure */
__attribute__((noinline, noclone))
int function_loop_invariants(int iterations, int *data) {
    /* Many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = large_constants[0] & 0xFFFF;
    register int r2 asm("ecx") = large_constants[1] & 0xFFFF;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int sum = 0;
    
    /* Loop with invariant address calculation using non-encodable immediates */
    for (int i = 0; i < iterations; i++) {
        /* Multiple uses of invariants in different expressions */
        temp1 = data[i] + (int)large_constants[0];  /* Large immediate */
        temp2 = data[i + 128] + (int)large_constants[1];  /* Another large immediate */
        temp3 = temp1 * r1;  /* Use register variable */
        temp4 = temp2 * r2;  /* Use another register variable */
        
        /* More temporaries to increase pressure */
        temp5 = (temp3 << 3) | (temp4 >> 2);
        temp6 = (temp4 << 5) & 0x7FFFFFFF;
        temp7 = temp5 ^ temp6;
        temp8 = temp7 + (int)((uintptr_t)global_array >> 16);  /* Symbolic address part */
        
        /* Complex condition with invariant */
        if (temp8 > (int)large_constants[2]) {
            sum += temp8 * r0;
        } else {
            sum += temp8 / (r0 | 1);
        }
        
        /* Use invariants in address calculation */
        global_array[i & 255] = sum;
        global_array[(i + 64) & 255] = temp8;
    }
    
    /* Cross-loop live ranges */
    r0 = sum + r1 + r2;
    return r0;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int function_asm_clobber(int x, int y) {
    int result1, result2, result3;
    register int a asm("esi") = x;
    register int b asm("edi") = y;
    
    /* Multi-output inline assembly creating hard register references */
    asm volatile (
        "movl %[input1], %%eax\n\t"
        "movl %[input2], %%ebx\n\t"
        "imull %%ebx, %%eax\n\t"
        "addl $0x12345678, %%eax\n\t"  /* Non-encodable immediate */
        "movl %%eax, %[out1]\n\t"
        "leal (%%eax, %%ebx, 4), %%ecx\n\t"
        "movl %%ecx, %[out2]\n\t"
        "shrl $8, %%ecx\n\t"
        "movl %%ecx, %[out3]"
        : [out1] "=&r" (result1),  /* Early clobber */
          [out2] "=&r" (result2),
          [out3] "=r" (result3)
        : [input1] "r" (a),
          [input2] "r" (b)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Use results in complex expressions */
    int t1 = result1 + (int)large_constants[3];
    int t2 = result2 * 0x9ABCDEF;  /* Another large immediate */
    int t3 = result3 & 0x55555555;
    
    /* Force register pressure with many live values */
    for (int i = 0; i < 16; i++) {
        t1 = (t1 << 1) | (t2 >> 31);
        t2 = (t2 << 2) + t3;
        t3 = t3 ^ (int)((uintptr_t)&global_array[i] >> 4);
        
        /* Use inline asm again to clobber more registers */
        asm volatile ("" : : "r"(t1), "r"(t2), "r"(t3) : "memory");
    }
    
    return t1 + t2 + t3 + a + b;
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int function_complex_cf(int selector, int base) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int r4 asm("ebp") = base;  /* Use frame pointer register */
    register int r5 asm("esp") = base * 2;  /* Use stack pointer register */
    
    int values[8];
    for (int i = 0; i < 8; i++) {
        values[i] = base + i * 0x1000;  /* Large offsets */
    }
    
    /* Many temporaries with overlapping lives */
    int t1 = values[0] + 0x7FFFFFFF;  /* Large immediate */
    int t2 = values[1] - 0x80000000;  /* Another large immediate */
    int t3 = t1 * t2;
    int t4 = t2 / (t1 | 1);
    int t5 = t3 ^ t4;
    int t6 = t4 | t3;
    int t7 = t5 & t6;
    int t8 = t6 + t7;
    
    /* Computed goto creating complex control flow */
    if (selector >= 0 && selector < 4) {
        goto *labels[selector];
    }
    
label0:
    /* Use all temporaries in different expressions */
    for (int i = 0; i < 4; i++) {
        t1 = (t1 << i) + (int)large_constants[i % 4];
        t2 = (t2 >> i) ^ values[i];
        t3 = t3 * (t4 + i);
        t4 = t4 / ((t5 - i) | 1);
        
        /* Switch inside loop for more complexity */
        switch (i) {
            case 0: t5 = r4 + t1; break;
            case 1: t5 = r5 + t2; break;
            case 2: t5 = t3 + (int)((uintptr_t)&global_array[0] >> 8); break;
            case 3: t5 = t4 + 0x12345678; break;
        }
        
        t6 = t5 * 0x11111111;
        t7 = t6 & 0xAAAAAAAA;
        t8 = t7 | 0x55555555;
    }
    goto end;

label1:
    t1 = r4 * 0x33333333;
    t2 = r5 / 0x44444444;
    goto label0;

label2:
    /* Use builtin for hard register reference */
    {
        uint64_t ts = __builtin_ia32_rdtsc();  /* Uses eax, edx */
        t3 = (int)(ts >> 32) + (int)ts;
        t4 = (int)(ts & 0xFFFFFFFF);
    }
    goto label1;

label3:
    t5 = (int)((uintptr_t)global_array >> 12);
    t6 = (int)((uintptr_t)large_constants >> 8);
    goto label2;

end:
    /* Consume all values to keep them live */
    sink = t1; sink = t2; sink = t3; sink = t4;
    sink = t5; sink = t6; sink = t7; sink = t8;
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + r4 + r5;
}

/* Main function to drive everything */
int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    int result = 0;
    
    /* Call each function with arguments that create register pressure */
    result += function_loop_invariants(
        argc > 1 ? atoi(argv[1]) : 100,
        global_array
    );
    
    result += function_asm_clobber(
        0x12345678,  /* Large immediate argument */
        0x9ABCDEF0   /* Another large immediate */
    );
    
    result += function_complex_cf(
        argc > 2 ? atoi(argv[2]) % 4 : 0,
        0x10000000   /* Large base value */
    );
    
    /* Use result to prevent optimization */
    return result & 0xFF;
}

/* test-early-remat.c */
#include <stdint.h>
#include <stdlib.h>

/* Global data for address calculations */
static int global_array[256];
static const long large_constants[] = {
    0x12345678, 0x9ABCDEF0, 0x55555555, 0xAAAAAAAA,
    0x11111111, 0x22222222, 0x33333333, 0x44444444
};

/* Function A: Loop with invariants and expensive constants */
__attribute__((noinline, noclone))
int func_loop_invariants(int iterations, int *data) {
    /* Use many local variables with overlapping live ranges */
    register int r0 asm("eax") = iterations;
    register int r1 asm("ebx") = large_constants[0] & 0xFFFF;
    register int r2 asm("ecx") = large_constants[1] & 0xFFFF;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int sum = 0;
    
    /* Loop with invariant address calculation using expensive constants */
    for (int i = 0; i < iterations; i++) {
        /* Multiple uses of large immediate constants in non-adjacent operations */
        temp1 = data[i] + (int)(large_constants[2] >> 16);
        temp2 = temp1 * (int)(large_constants[3] & 0xFFFF);
        temp3 = temp2 - (int)(large_constants[4] >> 8);
        temp4 = temp3 ^ (int)(large_constants[5] & 0xFF);
        
        /* More overlapping computations */
        temp5 = data[i + 1] * 0x123456;  /* Non-encodable immediate */
        temp6 = temp5 + 0x9ABCDE;        /* Another large immediate */
        temp7 = temp6 / 0x5555;          /* Yet another */
        temp8 = temp7 | 0xAAAAAA;        /* And another */
        
        /* Use register variables in complex expressions */
        r0 = (r0 * 0x98765432) ^ r1;     /* Large constant multiplication */
        r1 = (r1 + 0xFEDCBA98) | r2;     /* More large constants */
        r2 = (r2 * 0x13579BDF) + r0;     /* Chain of dependencies */
        
        /* Mix everything together */
        sum += temp4 + temp8 + r0 + r1 + r2;
        
        /* Conditional that uses invariants */
        if (i & 0x1) {
            sum += (int)(large_constants[6] >> 24);  /* Expensive constant */
        } else {
            sum -= (int)(large_constants[7] & 0xFF); /* Different expensive constant */
        }
    }
    
    /* More computations after loop to extend live ranges */
    temp1 = sum * 0x76543210;
    temp2 = temp1 / 0x2468ACE;
    temp3 = temp2 + 0xDB97531;
    temp4 = temp3 ^ 0x8F8F8F8F;
    
    return sum + temp4 + r0 + r1 + r2;
}

/* Function B: Inline assembly with clobbered registers */
__attribute__((noinline, noclone))
int func_asm_clobber(int a, int b) {
    int result1, result2, result3, result4;
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    
    /* Initialize many temporaries */
    temp1 = a + 0x11111111;
    temp2 = b + 0x22222222;
    temp3 = temp1 * 0x33333333;
    temp4 = temp2 * 0x44444444;
    temp5 = temp3 ^ 0x55555555;
    temp6 = temp4 ^ 0x66666666;
    temp7 = temp5 | 0x77777777;
    temp8 = temp6 | 0x88888888;
    
    /* Multi-output inline assembly that clobbers many registers */
    asm volatile (
        "movl %[t1], %%eax\n\t"
        "movl %[t2], %%ebx\n\t"
        "imull $0x12345678, %%eax, %%ecx\n\t"
        "addl $0x9ABCDEF0, %%ebx\n\t"
        "movl %%ecx, %[r1]\n\t"
        "movl %%ebx, %[r2]\n\t"
        : [r1] "=&r" (result1), [r2] "=&r" (result2)
        : [t1] "r" (temp7), [t2] "r" (temp8)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* More computations between asm blocks */
    temp1 = result1 * 0xAAAAAAAA;
    temp2 = result2 * 0x55555555;
    temp3 = temp1 + 0xCCCCCCCC;
    temp4 = temp2 + 0xDDDDDDDD;
    
    /* Another asm with different clobbers */
    asm volatile (
        "movl %[t3], %%esi\n\t"
        "movl %[t4], %%edi\n\t"
        "leal (%%esi,%%edi,4), %%eax\n\t"
        "movl %%eax, %[r3]\n\t"
        "movl $0x87654321, %[r4]\n\t"
        : [r3] "=&r" (result3), [r4] "=r" (result4)
        : [t3] "r" (temp3), [t4] "r" (temp4)
        : "eax", "esi", "edi", "cc"
    );
    
    /* Complex expression using all results */
    return ((result1 + result2) * 0x2468ACE0) ^ 
           ((result3 - result4) * 0x13579BDF) +
           (temp1 * temp2) - (temp3 / temp4);
}

/* Function C: Complex control flow with register variables */
__attribute__((noinline, noclone))
int func_complex_control(int selector, int count) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    
    int array[16];
    int sum = 0;
    
    /* Initialize array with expensive constants */
    for (int i = 0; i < 16; i++) {
        array[i] = i * (int)(large_constants[i % 8] >> (i * 2));
    }
    
    reg_a = selector;
    reg_b = count;
    reg_c = 0;
    
    /* Computed goto creates complex control flow */
    goto *labels[selector % 4];
    
label0:
    for (int i = 0; i < count; i++) {
        int t1 = reg_a * 0x11111111;
        int t2 = reg_b * 0x22222222;
        int t3 = reg_c * 0x33333333;
        
        /* Switch inside loop */
        switch (i % 4) {
            case 0:
                reg_a = (t1 + 0x44444444) ^ array[i % 16];
                break;
            case 1:
                reg_b = (t2 - 0x55555555) | array[(i + 1) % 16];
                break;
            case 2:
                reg_c = (t3 * 0x66666666) & array[(i + 2) % 16];
                break;
            case 3:
                reg_a = reg_b ^ reg_c;
                reg_b = reg_c + array[i % 16];
                reg_c = reg_a * 0x77777777;
                break;
        }
        
        sum += reg_a + reg_b + reg_c;
    }
    goto end;
    
label1:
    /* Different computation pattern */
    while (reg_b-- > 0) {
        reg_a = (reg_a << 3) | 0x88888888;
        reg_c = (reg_c >> 2) & 0x99999999;
        sum += reg_a * reg_c * array[reg_b % 16];
    }
    goto end;
    
label2:
    /* Nested loops */
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 4; j++) {
            int t = (reg_a + i) * (reg_b + j) * (reg_c + (i * j));
            t ^= 0xAAAAAAAA;
            t += 0xBBBBBBBB;
            t *= 0xCCCCCCCC;
            sum += t;
        }
        reg_a = (reg_a * 0xDDDDDDDD) >> 3;
        reg_b = (reg_b + 0xEEEEEEEE) & 0x7FFFFFFF;
    }
    goto end;
    
label3:
    /* Mixed pattern */
    do {
        reg_a = __builtin_ia32_rdtsc() & 0xFFFFFFFF;  /* Hard register reference */
        reg_b = (reg_b * 0xFFFFFFFF) + 1;
        reg_c = reg_a ^ reg_b;
        
        if (reg_c & 1) {
            sum += array[reg_c % 16] * 0x12345678;
        } else {
            sum -= array[reg_c % 16] * 0x9ABCDEF0;
        }
    } while (reg_b < count);
    
end:
    return sum + reg_a + reg_b + reg_c;
}

/* Main function that calls all test patterns */
int main(int argc, char **argv) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    int iterations = argc > 1 ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    int sum = 0;
    
    /* Call all test functions with arguments that create register pressure */
    sum += func_loop_invariants(iterations, global_array);
    sum += func_asm_clobber(iterations, iterations * 2);
    sum += func_complex_control(iterations % 4, iterations / 4);
    
    /* Use the result to prevent dead code elimination */
    return sum == 0 ? 1 : 0;
}

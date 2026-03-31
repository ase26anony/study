/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    unsigned int u1 = 1000, u2 = 2000;
    double dbl1 = 1.01, dbl2 = 2.02;
    
    /* Complex irreducible control flow using goto */
    volatile int jump_target = selector % 5;
    
    /* Clobber registers to increase pressure */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    
    /* Large switch with many cases */
    switch (selector % 12) {
        case 0:
            a = b + c;
            f1 = f2 * f3;
            if (jump_target == 0) goto label1;
            break;
        case 1:
            b = c - d;
            dbl1 = dbl2 / 2.0;
            if (jump_target == 1) goto label2;
            break;
        case 2:
            c = d * e;
            s1 = s2 + s3;
            if (jump_target == 2) goto label3;
            break;
        case 3:
            d = e / a;
            ch1 = ch2 + 1;
            if (jump_target == 3) goto label4;
            break;
        case 4:
            e = a ^ b;
            l1 = l2 - l3;
            if (jump_target == 4) goto label5;
            break;
        case 5:
            s2 = s3 * s1;
            u1 = u2 >> 1;
            goto label1;
        case 6:
            s3 = s1 + s2;
            u2 = u1 << 1;
            goto label2;
        case 7:
            ch2 = ch3 - 1;
            f2 = f3 + f1;
            goto label3;
        case 8:
            ch3 = ch1 * 2;
            f3 = f1 * f2;
            goto label4;
        case 9:
            f1 = f2 - f3;
            l2 = l3 + l1;
            goto label5;
        case 10:
            f2 = f3 / f1;
            l3 = l1 - l2;
            goto label1;
        case 11:
            f3 = f1 + f2;
            dbl2 = dbl1 * 3.0;
            goto label2;
    }
    
    /* Labels for goto jumps creating irreducible flow */
label1:
    a += s1 + ch1;
    asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    goto label6;
    
label2:
    b += s2 + ch2;
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    goto label7;
    
label3:
    c += s3 + ch3;
    asm volatile("" : : : "xmm4", "xmm5", "xmm6", "xmm7");
    goto label8;
    
label4:
    d += u1 + l1;
    asm volatile("" : : : "mm0", "mm1", "mm2");
    goto label9;
    
label5:
    e += u2 + l2;
    asm volatile("" : : : "st", "st(1)", "st(2)", "st(3)");
    goto label10;
    
label6:
    f1 += dbl1;
    if (selector & 1) goto label4;
    goto label11;
    
label7:
    f2 += dbl2;
    if (selector & 2) goto label5;
    goto label12;
    
label8:
    f3 += (float)dbl1;
    if (selector & 4) goto label1;
    goto label11;
    
label9:
    l1 += (long)f1;
    if (selector & 8) goto label2;
    goto label12;
    
label10:
    l2 += (long)f2;
    if (selector & 16) goto label3;
    goto label11;
    
label11:
    l3 += (long)f3;
    /* More register clobbering */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
    return a + b + c + d + e + s1 + s2 + s3 + ch1 + ch2 + ch3 + 
           (int)f1 + (int)f2 + (int)f3 + (int)l1 + (int)l2 + (int)l3 + 
           u1 + u2 + (int)dbl1 + (int)dbl2;
    
label12:
    /* Alternative return path */
    asm volatile("" : : : "rsi", "rdi", "rbp", "rsp");
    return a * b * c * d * e + s1 * s2 * s3 + ch1 * ch2 * ch3;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int total = 0;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling the register pressure function */
    for (int i = 0; i < iterations; i++) {
        volatile int selector = rand();
        int result = register_pressure_function(selector);
        total += result;
        
        /* Occasionally change control flow pattern */
        if (i % 1000 == 0) {
            asm volatile("" : : "r"(total) : "memory");
        }
    }
    
    printf("Total result: %d\n", total);
    
    /* Additional test with different optimization patterns */
    {
        volatile int test_vals[] = {0, ENTRY_BLOCK, EXIT_BLOCK, 100, 255};
        for (int i = 0; i < 5; i++) {
            total += register_pressure_function(test_vals[i]);
        }
    }
    
    return total > 0 ? 0 : 1;
}

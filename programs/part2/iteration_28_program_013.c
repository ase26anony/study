/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function to create high register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Create irreducible control flow with goto */
    volatile int jump_target = selector % 5;
    
    /* Complex switch with many cases */
    switch (selector % 12) {
        case 0:
            a = b + c;
            f1 = f2 * f3;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            if (jump_target == 0) goto label1;
            break;
            
        case 1:
            c = d - e;
            s1 = s2 + s3;
            asm volatile("" : : : "esi", "edi");
            if (jump_target == 1) goto label2;
            break;
            
        case 2:
            d = e * a;
            ch1 = ch2 + 1;
            asm volatile("" : : : "r8", "r9", "r10");
            goto label3;
            
        case 3:
            e = a / (b ? b : 1);
            f2 = f3 - f1;
            asm volatile("" : : : "r11", "r12", "r13");
            if (jump_target == 2) goto label4;
            break;
            
        case 4:
            s2 = s3 - s1;
            l1 = l2 + l3;
            asm volatile("" : : : "xmm0", "xmm1");
            goto label5;
            
        case 5:
            ch2 = ch3 * 2;
            d1 = d2 * 2.0;
            asm volatile("" : : : "xmm2", "xmm3", "xmm4");
            if (jump_target == 3) goto label1;
            break;
            
        case 6:
            u1 = u2 - 500;
            f3 = f1 + f2;
            asm volatile("" : : : "xmm5", "xmm6");
            goto label2;
            
        case 7:
            a = b * c * d;
            s3 = s1 + s2;
            asm volatile("" : : : "xmm7", "xmm8");
            if (jump_target == 4) goto label3;
            break;
            
        case 8:
            b = c + d + e;
            ch3 = ch1 - 10;
            asm volatile("" : : : "xmm9", "xmm10");
            goto label4;
            
        case 9:
            c = a ^ b ^ d;
            l2 = l3 - l1;
            asm volatile("" : : : "xmm11", "xmm12");
            if (selector & 1) goto label5;
            break;
            
        case 10:
            d = (a << 2) | (b >> 1);
            u2 = u1 * 2;
            asm volatile("" : : : "xmm13", "xmm14");
            goto label1;
            
        case 11:
            e = (a + b + c + d) / 4;
            d2 = d1 / 2.0;
            asm volatile("" : : : "xmm15");
            if (selector & 2) goto label2;
            break;
    }
    
    /* Labels for goto jumps creating irreducible flow */
    label1:
    a += s1 + ch1;
    
    label2:
    b += s2 + ch2;
    if (selector % 3 == 0) goto label4;
    
    label3:
    c += s3 + ch3;
    f1 += 0.5f;
    
    label4:
    d += l1 % 100;
    f2 *= 1.1f;
    if (selector % 7 == 0) goto label1;
    
    label5:
    e += l2 % 50;
    f3 -= 0.2f;
    
    /* More arithmetic to increase register pressure */
    a = a * 2 + b;
    b = b * 3 + c;
    c = c * 4 + d;
    d = d * 5 + e;
    e = e * 6 + a % 10;
    
    s1 = s1 + s2 + s3;
    s2 = s2 - s1 + s3;
    s3 = s3 * s1 - s2;
    
    ch1 = ch1 + ch2 + ch3;
    ch2 = ch2 - ch1;
    ch3 = ch3 * 2;
    
    f1 = f1 + f2 + f3;
    f2 = f2 * f1;
    f3 = f3 / (f1 + 1.0f);
    
    l1 = l1 + l2 + l3;
    l2 = l2 - l1;
    l3 = l3 * 2;
    
    d1 = d1 + d2;
    d2 = d2 * d1;
    
    u1 = u1 + u2;
    u2 = u2 ^ u1;
    
    /* Final clobber */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                               "rsi", "rdi", "r8", "r9", "r10",
                               "r11", "r12", "r13", "r14", "r15",
                               "xmm0", "xmm1", "xmm2", "xmm3",
                               "xmm4", "xmm5", "xmm6", "xmm7");
    
    /* Return value using all variables to prevent optimization */
    return a + b + c + d + e + s1 + s2 + s3 + ch1 + ch2 + ch3 + 
           (int)f1 + (int)f2 + (int)f3 + (int)l1 + (int)l2 + (int)l3 +
           (int)d1 + (int)d2 + u1 + u2;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int total = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    /* Hot loop calling the high-pressure function */
    for (int i = 0; i < iterations; i++) {
        /* Volatile selector to prevent optimization */
        volatile int selector = rand() % 100;
        
        /* Call the function that should trigger MCF */
        total += register_pressure_function(selector);
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            selector = rand() % 50;
        }
    }
    
    /* Print result to create observable side effect */
    printf("Total: %d\n", total);
    
    return 0;
}

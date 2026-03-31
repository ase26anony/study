/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Clobber registers to increase pressure (x86 version) */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    
    /* Complex irreducible control flow using goto */
    volatile int jump_target = selector % 5;
    
    if (jump_target == 0) goto case_0;
    if (jump_target == 1) goto case_1;
    if (jump_target == 2) goto case_2;
    if (jump_target == 3) goto case_3;
    
    /* Large switch statement with many cases */
    switch (selector % 12) {
        case_0:
        case 0:
            a = b + c * d - e;
            s1 = s2 + s3;
            f1 = f2 * f3;
            /* Jump to another case */
            if (a > 10) goto case_5;
            break;
            
        case_1:
        case 1:
            b = a * 2 + c / 3;
            ch1 = ch2 + 1;
            d1 = d2 * 1.5;
            /* Clobber more registers */
            asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            break;
            
        case_2:
        case 2:
            c = d * e - a;
            s3 = s1 - s2;
            l1 = l2 + l3;
            goto case_7;
            
        case_3:
        case 3:
            d = e + a * b;
            ch2 = ch3 - 1;
            f2 = f1 + f3;
            break;
            
        case 4:
            e = c * d / a;
            u1 = u2 * 2;
            f3 = f1 * f2;
            /* Another goto creating irreducible region */
            if (e < 100) goto case_1;
            break;
            
        case_5:
        case 5:
            s1 = s2 * s3;
            ch3 = ch1 + ch2;
            l2 = l1 * 2;
            /* Register clobbering */
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
            break;
            
        case 6:
            s2 = s3 + s1;
            f1 = f2 / f3;
            u2 = u1 + 100;
            goto case_10;
            
        case_7:
        case 7:
            s3 = s1 - s2;
            ch1 = ch2 * ch3;
            d2 = d1 * 2.0;
            break;
            
        case 8:
            ch2 = ch1 + 3;
            l3 = l1 + l2;
            f2 = f3 - f1;
            /* Force register pressure */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
            break;
            
        case 9:
            ch3 = ch2 - ch1;
            u1 = u2 / 2;
            d1 = d2 + 1.0;
            if (ch3 > 0) goto case_2;
            break;
            
        case_10:
        case 10:
            f3 = f1 + f2;
            l1 = l2 - l3;
            a = b * c;
            break;
            
        case 11:
            /* Mix all operations */
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + a;
            e = a + b;
            s1 = s2 + s3;
            s2 = s3 + s1;
            s3 = s1 + s2;
            ch1 = ch2 + ch3;
            ch2 = ch3 + ch1;
            ch3 = ch1 + ch2;
            f1 = f2 + f3;
            f2 = f3 + f1;
            f3 = f1 + f2;
            l1 = l2 + l3;
            l2 = l3 + l1;
            l3 = l1 + l2;
            d1 = d2 * 1.1;
            d2 = d1 * 1.2;
            u1 = u2 + 500;
            u2 = u1 + 1000;
            /* Maximum register pressure */
            asm volatile("" : : : 
                "eax", "ebx", "ecx", "edx", "esi", "edi",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
            break;
    }
    
    /* Use all variables in return value to prevent elimination */
    return a + b + c + d + e + 
           s1 + s2 + s3 + 
           ch1 + ch2 + ch3 + 
           (int)f1 + (int)f2 + (int)f3 + 
           (int)l1 + (int)l2 + (int)l3 + 
           (int)d1 + (int)d2 + 
           u1 + u2;
}

/* Another complex function to create more flow graph edges */
__attribute__((noinline, noipa))
static int secondary_pressure_function(volatile int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    volatile int counter = 0;
    
    /* Nested loops with conditionals */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            if (mode % 3 == 0) {
                x1 = x2 * x3;
                x2 = x3 + x4;
                asm volatile("" : : : "eax", "ebx");
            } else if (mode % 3 == 1) {
                x3 = x4 - x5;
                x4 = x5 * x1;
                goto inner_jump;
            } else {
                x5 = x1 + x2;
            inner_jump:
                x1 = x5 / 2;
            }
            counter++;
        }
        
        /* Switch inside loop */
        switch (i % 4) {
            case 0: x1 += x2; break;
            case 1: x2 += x3; break;
            case 2: x3 += x4; break;
            case 3: x4 += x5; x5 = x1; break;
        }
    }
    
    return x1 + x2 + x3 + x4 + x5 + counter;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int total = 0;
    
    /* Use command line argument for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        volatile int selector = rand() % 100;
        
        /* Call both pressure functions */
        total += register_pressure_function(selector);
        
        if (i % 7 == 0) {
            total += secondary_pressure_function(selector);
        }
        
        /* Occasionally add more complexity */
        if (i % 13 == 0) {
            volatile int temp = 0;
            for (int j = 0; j < 5; j++) {
                temp += j * selector;
            }
            total += temp;
        }
    }
    
    printf("Total result: %d\n", total);
    return 0;
}

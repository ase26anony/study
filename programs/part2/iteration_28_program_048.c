/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Complex function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Use inline assembly to clobber registers (x86 version) */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    
    /* Complex irreducible control flow using goto */
    if (selector < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (selector % 12) {
        case 0:
            a = b + c;
            f1 = f2 * f3;
            /* Jump to another case block */
            if (a > 10) goto case_5;
            break;
            
        case 1:
            b = c * d;
            s1 = s2 + s3;
            ch1 = ch2 ^ ch3;
            goto case_common;
            
        case 2:
            c = d - e;
            l1 = l2 >> 1;
            asm volatile("" : : : "memory");
            break;
            
        case 3:
            d = e / (a ? a : 1);
            f2 = f1 + f3;
            goto case_negative;
            
        case 4:
            e = a ^ b ^ c;
            d1 = d2 * 2.0;
            /* Another register clobber */
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
            
        case 5:
        case_5:
            s2 = s1 * s3;
            u1 = u2 - 500;
            if (s2 > 100) goto case_7;
            break;
            
        case 6:
            ch2 = ch1 + 1;
            l2 = l1 * 2;
            f3 = f1 - f2;
            goto case_common;
            
        case 7:
        case_7:
            ch3 = ch2 - 1;
            l3 = l2 / 2;
            d2 = d1 + 1.0;
            break;
            
        case 8:
            u2 = u1 * 2;
            a = b << 2;
            /* Force spill pressure */
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case_negative;
            
        case 9:
            f1 = f2 / f3;
            c = d | e;
            break;
            
        case 10:
            b = c & d;
            s3 = s1 - s2;
            goto case_common;
            
        case 11:
            a = ~b;
            f2 = f3 * 4.0f;
            break;
            
        default:
            goto case_negative;
    }
    
    /* Common code block reachable from multiple places */
    case_common:
    d = e + a;
    u1 = u2 >> 1;
    asm volatile("" : : : "r12", "r13", "r14", "r15");
    
    /* Negative case label */
    case_negative:
    f3 = f1 + f2;
    l1 = l2 + l3;
    
    /* More complex operations mixing types */
    a = (int)f1 + (int)d1;
    b = s1 * ch1;
    c = (u1 & 0xFF) + ch2;
    
    /* Final register clobber */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                               "rsi", "rdi", "r8", "r9",
                               "r10", "r11", "r12", "r13",
                               "r14", "r15", "xmm0", "xmm1",
                               "xmm2", "xmm3", "xmm4", "xmm5");
    
    /* Return value using all variables to prevent elimination */
    return a + b + c + d + e + s1 + s2 + s3 + 
           ch1 + ch2 + ch3 + (int)f1 + (int)f2 + (int)f3 +
           (int)l1 + (int)l2 + (int)l3 + (int)d1 + (int)d2 +
           u1 + u2;
}

/* Another complex function to create more flow graph edges */
NOINLINE static int nested_control_flow(volatile int x) {
    int result = 0;
    
    /* Nested loops with conditions */
    for (int i = 0; i < 5; i++) {
        if (x & (1 << i)) {
            for (int j = 0; j < 3; j++) {
                if (j % 2 == 0) {
                    result += i * j;
                    goto inner_label;
                } else {
                    result -= i + j;
                }
                inner_label:
                asm volatile("" : : : "eax", "ebx");
            }
        } else {
            switch (i % 3) {
                case 0: result += x; break;
                case 1: result -= x; goto switch_exit;
                case 2: result *= 2; break;
                switch_exit:
                default: result /= (x ? x : 1);
            }
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector;
    long long total = 0;
    
    /* Use command line argument or default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    
    printf("Running MCF stress test for %d iterations...\n", iterations);
    
    /* Hot loop calling complex functions */
    for (int i = 0; i < iterations; i++) {
        /* Volatile to prevent optimization */
        selector = (rand() % 100) - 25;
        
        /* Call the high-pressure function */
        total += register_pressure_function(selector);
        
        /* Call nested control flow function every 10 iterations */
        if (i % 10 == 0) {
            total += nested_control_flow(selector);
        }
        
        /* Additional pressure with inline asm */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Total result: %lld\n", total);
    
    return 0;
}

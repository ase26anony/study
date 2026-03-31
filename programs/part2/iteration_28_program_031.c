/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.01, d2 = 2.02;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Use goto to create irreducible control flow */
    if (selector < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (selector % 12) {
        case 0:
            a = b + c;
            d = e * 2;
            /* Clobber registers with inline asm */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_common;
            
        case 1:
            s1 = s2 - s3;
            ch1 = ch2 + 1;
            asm volatile("" : : : "esi", "edi");
            f1 = f2 * f3;
            goto case_common;
            
        case 2:
            l1 = l2 + l3;
            d1 = d2 / 2.0;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case_3;
            
        case 3:
        case_3:
            u1 = u2 * 3;
            a = c - d;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            break;
            
        case 4:
            b = a * c;
            s3 = s1 + s2;
            goto case_5;
            
        case 5:
        case_5:
            ch3 = ch1 - ch2;
            f3 = f1 + f2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case_common;
            
        case 6:
            l2 = l1 * 2;
            d2 = d1 * 3.14;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            break;
            
        case 7:
            u2 = u1 / 2;
            e = d + a;
            goto case_8;
            
        case 8:
        case_8:
            s2 = s3 * s1;
            ch2 = ch3 + ch1;
            asm volatile("" : : : "xmm6", "xmm7");
            goto case_common;
            
        case 9:
            f2 = f3 - f1;
            l3 = l2 + l1;
            asm volatile("" : : : "rax", "rbx", "rcx");
            break;
            
        case 10:
            d = e * c;
            u1 = u2 + 100;
            goto case_negative;
            
        case 11:
            a = b * d;
            s1 = s2 / 2;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            break;
            
        default:
            goto case_common;
    }
    
    goto after_switch;
    
case_negative:
    /* Another basic block reachable from multiple places */
    a = -a;
    b = -b;
    c = -c;
    asm volatile("" : : : "r8", "r9", "r10", "r11");
    
case_common:
    /* Common block reachable from multiple cases */
    f1 = f1 * 2.0f;
    d1 = d1 + 1.0;
    asm volatile("" : : : "xmm8", "xmm9", "xmm10");
    
after_switch:
    
    /* Complex arithmetic using all variables to prevent optimization */
    int result = a + b + c + d + e;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)l1 + (int)l2 + (int)l3;
    result += (int)d1 + (int)d2;
    result += u1 + u2;
    
    /* More control flow with goto */
    if (result > 1000) {
        goto large_result;
    } else {
        goto small_result;
    }
    
large_result:
    result = result / 2;
    asm volatile("" : : : "r12", "r13", "r14");
    goto final;
    
small_result:
    result = result * 2;
    asm volatile("" : : : "r15", "rbp");
    
final:
    /* Final computation using all variables */
    return result + (a * b) - (c * d) + (e % 7) + 
           (s1 * s2) + (ch3 - ch1) + (int)(f2 * 10) + 
           (l1 >> 2) + (int)(d1 * 2) + (u2 - u1);
}

/* Another function to increase overall complexity */
NOINLINE static int helper_function(int x, int y) {
    volatile int v = x * y;
    int result = 0;
    
    /* Loop with switch inside */
    for (int i = 0; i < 5; i++) {
        switch ((v + i) % 4) {
            case 0: result += x; break;
            case 1: result += y; break;
            case 2: result += x * y; break;
            case 3: result += x - y; break;
        }
        asm volatile("" : : : "eax", "ebx");
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int base_selector = (argc > 2) ? atoi(argv[2]) : rand();
    
    int total = 0;
    
    /* Hot loop calling the register pressure function */
    for (int i = 0; i < iterations; i++) {
        volatile int selector = base_selector + i;
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call helper to add more complexity */
        int result2 = helper_function(selector % 100, i % 100);
        
        total += result1 + result2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            base_selector = (base_selector * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}

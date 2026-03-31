/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and complex control flow */
NOINLINE int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'a', ch2 = 'b', ch3 = 'c';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases */
    switch (control % 12) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case1_label;
        
        case1_label:
        case 1:
            g = h - i;
            fl1 = fl2 + fl3;
            s1 = s2 * s3;
            goto case5_label;
        
        case 2:
            ch1 = ch2 + 1;
            ch3 = ch1 - ch2;
            result = a + b;
            /* Irreducible control flow with goto */
            if (result > 10) goto case7_label;
            else goto case3_label;
        
        case3_label:
        case 3:
            j = a * b * c;
            fl2 = fl1 * 2.0f;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 4:
            d = e + f + g;
            s2 = s1 - s3;
            goto case6_label;
        
        case5_label:
        case 5:
            h = i / 2;
            fl3 = fl1 + fl2;
            ch2 = ch3 * 2;
            /* Another register clobber */
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case8_label;
        
        case6_label:
        case 6:
            a = b * c * d;
            result = e + f + g;
            if (result < 50) goto case2_label;
            break;
        
        case7_label:
        case 7:
            s3 = s1 + s2;
            fl1 = fl3 - fl2;
            ch3 = ch1 + ch2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case11_label;
        
        case8_label:
        case 8:
            i = j * 2;
            fl2 = fl1 / 2.0f;
            result = h + i + j;
            goto case10_label;
        
        case 9:
            a = b + c + d + e;
            s1 = s2 * 3;
            /* Force register pressure */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
            break;
        
        case10_label:
        case 10:
            f = g * h;
            ch1 = ch2 + ch3;
            fl3 = fl1 * fl2;
            goto case4_label;
        
        case4_label:
        case 11:
        case11_label:
            j = a + b + c + d + e;
            s2 = s3 / 2;
            result = f + g + h + i + j;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            break;
        
        case2_label:
        default:
            result = a + b + c + d + e + f + g + h + i + j;
            fl1 = fl2 + fl3;
            s3 = s1 + s2;
            ch3 = ch1 + ch2;
            break;
    }
    
    /* Use all variables to prevent optimization */
    result += a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)(fl1 + fl2 + fl3);
    
    return result;
}

/* Another function to create more control flow complexity */
NOINLINE int nested_control_flow(volatile int x) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    volatile int y = x;
    
    if (y > 100) {
        for (int i = 0; i < 5; i++) {
            r1 += i * 2;
            asm volatile("" : : : "eax");
            if (i % 2 == 0) {
                r2 += register_pressure_function(y + i);
                goto loop_mid;
            } else {
                r3 += y * i;
            }
        loop_mid:
            r4 += i * i;
        }
    } else {
        int z = y;
        while (z > 0) {
            r5 += z;
            z--;
            if (z % 3 == 0) {
                r1 += register_pressure_function(z);
                goto while_continue;
            }
            r2 += z * 2;
        while_continue:
            r3 += z * 3;
        }
    }
    
    return r1 + r2 + r3 + r4 + r5;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total = 0;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Vary the selector to hit different switch cases */
        selector = rand() % 1000;
        
        /* Call the high-pressure function */
        int result = register_pressure_function(selector);
        
        /* Also call nested function for more complexity */
        if (i % 3 == 0) {
            result += nested_control_flow(selector);
        }
        
        total += result;
        
        /* Occasionally change control flow pattern */
        if (i % 1000 == 0) {
            selector = i;
        }
    }
    
    printf("Total result: %lld\n", total);
    
    return 0;
}

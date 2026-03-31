/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function to create high register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            d = e * f;
            fl1 = fl2 + fl3;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 2:
            g = h - i;
            s1 = s2 * s3;
            goto case_8;
        
        case 3:
            ch1 = ch2 + 1;
            fl2 = fl1 * 2.0f;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 4:
            j = a * b * c;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            goto case_1;
        
        case 5:
        case_5:
            s3 = s1 + s2;
            fl3 = fl1 / fl2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case 6:
            a = d + e + f;
            ch3 = ch1 - ch2;
            goto case_10;
        
        case 7:
            b = g * h;
            fl1 = fl3 - fl2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            break;
        
        case 8:
        case_8:
            c = i / j;
            s2 = s3 - s1;
            goto case_12;
        
        case 9:
            d = a * b * c * d;
            asm volatile("" : : : "xmm6", "xmm7");
            break;
        
        case 10:
        case_10:
            e = f + g + h;
            ch2 = ch3 * 2;
            asm volatile("" : : : "rax", "rbx", "rcx");
            goto case_14;
        
        case 11:
            f = e - d;
            fl2 = fl1 * 3.14f;
            break;
        
        case 12:
        case_12:
            g = h * i * j;
            s1 = s2 / 2;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            goto case_16;
        
        case 13:
            h = a + b + c + d;
            ch1 = ch2 + ch3;
            break;
        
        case 14:
        case_14:
            i = j * 2;
            fl3 = fl1 + fl2;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case_18;
        
        case 15:
            j = a - b - c;
            s3 = s1 * s2;
            break;
        
        case 16:
        case_16:
            a = b * c * d * e;
            ch3 = ch1 - 32;
            asm volatile("" : : : "xmm8", "xmm9", "xmm10");
            goto case_0_alt;
        
        case 17:
            b = c + d + e + f;
            fl1 = fl2 / fl3;
            break;
        
        case 18:
        case_18:
            c = d * e * f;
            s2 = s3 + 100;
            asm volatile("" : : : "xmm11", "xmm12", "xmm13");
            goto case_end;
        
        case 19:
            d = e - f - g;
            ch2 = ch3 + 1;
            break;
        
        default:
            goto case_negative;
    }
    
    goto after_switch;
    
case_0_alt:
    a = a * 2;
    b = b * 3;
    goto after_switch;

case_negative:
    a = -a;
    b = -b;
    c = -c;
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
    goto after_switch;

case_end:
    /* More operations to increase basic blocks */
    fl1 = fl1 * 2.0f;
    fl2 = fl2 / 2.0f;
    asm volatile("" : : : "xmm14", "xmm15");

after_switch:
    /* Use all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)(fl1 + fl2 + fl3);
    
    /* Additional irreducible loop */
    {
        int k = 0;
        volatile int loop_ctrl = result % 5;
    loop_start:
        if (k >= loop_ctrl) goto loop_end;
        result += k * k;
        k++;
        
        /* Nested conditional with goto */
        if (k % 2 == 0) {
            goto even_iteration;
        } else {
            goto odd_iteration;
        }
        
    even_iteration:
        result += 100;
        goto loop_start;
        
    odd_iteration:
        result -= 50;
        goto loop_start;
        
    loop_end:
        /* Do nothing */
        ;
    }
    
    return result;
}

/* Another complex function to increase overall complexity */
NOINLINE static int secondary_pressure_function(int base) {
    int x1 = base, x2 = base + 1, x3 = base + 2;
    int x4 = base + 3, x5 = base + 4, x6 = base + 5;
    volatile int branch = base % 7;
    
    /* Complex if-else chain */
    if (branch == 0) {
        x1 = x2 * x3;
        asm volatile("" : : : "eax", "ebx");
        goto label_a;
    } else if (branch == 1) {
        x2 = x3 + x4;
        goto label_b;
    } else if (branch == 2) {
        x3 = x4 - x5;
        asm volatile("" : : : "ecx", "edx");
        goto label_c;
    } else if (branch == 3) {
        x4 = x5 * x6;
        goto label_a;
    } else if (branch == 4) {
        x5 = x6 / x1;
        asm volatile("" : : : "esi", "edi");
        goto label_b;
    } else if (branch == 5) {
        x6 = x1 + x2 + x3;
        goto label_c;
    } else {
        x1 = x2 * x3 * x4;
        asm volatile("" : : : "r8", "r9");
    }
    
    goto after_labels;

label_a:
    x1 += 100;
    goto after_labels;

label_b:
    x2 += 200;
    goto after_labels;

label_c:
    x3 += 300;
    /* Fall through */

after_labels:
    return x1 + x2 + x3 + x4 + x5 + x6;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total = 0;
    int i;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (i = 0; i < iterations; i++) {
        /* Vary selector to exercise different paths */
        selector = rand() % 100;
        
        /* Call main pressure function */
        total += register_pressure_function(selector);
        
        /* Call secondary function every 10 iterations */
        if (i % 10 == 0) {
            total += secondary_pressure_function(selector);
        }
        
        /* Occasionally add more complexity */
        if (i % 37 == 0) {
            volatile int temp = selector;
            asm volatile("" : "+r" (temp) : : "memory");
            selector = temp;
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional test with different optimization patterns */
    {
        int j, k;
        volatile int matrix[10][10];
        
        /* Initialize matrix */
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 10; k++) {
                matrix[j][k] = j * 10 + k;
            }
        }
        
        /* Complex nested loops with conditionals */
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 10; k++) {
                if ((j + k) % 3 == 0) {
                    total += matrix[j][k];
                    asm volatile("" : : : "xmm0");
                } else if ((j + k) % 3 == 1) {
                    total -= matrix[j][k];
                    asm volatile("" : : : "xmm1");
                } else {
                    total ^= matrix[j][k];
                    asm volatile("" : : : "xmm2");
                }
            }
        }
    }
    
    printf("Final total: %lld\n", total);
    return 0;
}

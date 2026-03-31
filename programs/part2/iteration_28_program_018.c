/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no inlining to preserve complex control flow */
__attribute__((noinline, noipa))
static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    
    /* Volatile to prevent optimization */
    volatile int control = selector;
    int result = 0;
    
    /* Complex irreducible control flow with goto */
    if (control < 0) goto negative_case;
    
    /* Large switch with many cases */
    switch (control % 10) {
        case 0:
            a = b + c;
            b = c * d;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            c = d - e;
            f1 = f2 + f3;
            asm volatile("" : : : "r0", "r1", "r2", "r3");
            break;
        
        case 2:
            d = e / (a ? a : 1);
            s1 = s2 + s3;
            goto case_8;
        
        case 3:
            e = a * b * c;
            ch1 = ch2 + 1;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 4:
            f2 = f1 * f3;
            l1 = l2 - l3;
            goto negative_case;
        
        case_5:
        case 5:
            f3 = f1 / f2;
            d1 = d2 * 2.0;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 6:
            l2 = l1 * 2;
            ch2 = ch3 - 1;
            goto case_0_alt;
        
        case 7:
            l3 = l1 + l2;
            s2 = s1 * 2;
            asm volatile("" : : : "r11", "r12");
            break;
        
        case_8:
        case 8:
            d2 = d1 + 1.0;
            ch3 = ch1 + ch2;
            goto case_9;
        
        case_9:
        case 9:
            s3 = s1 + s2;
            a = b - c + d - e;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case_0_alt:
            /* Alternative entry point for case 0 */
            a = a * 2;
            f1 = f1 + 1.0f;
            break;
        
        default:
            goto negative_case;
    }
    
    /* More arithmetic to use all variables */
    result = a + b + c + d + e;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)(f1 + f2 + f3);
    result += (int)(d1 + d2);
    result += (int)(l1 + l2 + l3);
    
    return result;

negative_case:
    /* Different computation path */
    a = -a; b = -b; c = -c;
    result = a * b * c;
    asm volatile("" : : : "memory");
    return result;
}

/* Another function with loops to create more control flow edges */
__attribute__((noinline, noipa))
static int nested_loop_function(int iterations) {
    int i, j, k, sum = 0;
    volatile int control = iterations % 7;
    
    /* Nested loops with conditionals */
    for (i = 0; i < iterations % 100; i++) {
        if (control == 0) {
            for (j = i; j > 0; j--) {
                sum += j;
                if (j % 3 == 0) goto skip_inner;
            }
        } else if (control == 1) {
            j = 0;
            while (j < i) {
                sum -= j;
                j += 2;
                asm volatile("" : : : "ebx", "ecx");
            }
        } else {
            do {
                sum += i * 2;
                k = sum % 5;
                switch (k) {
                    case 0: sum += 1; break;
                    case 1: sum += 2; goto loop_end;
                    case 2: sum += 3; break;
                    default: sum += 4;
                }
            } while (--i > 0);
        }
        
    skip_inner:
        /* Empty but creates a basic block */
        asm volatile("" : : : "memory");
    }
    
loop_end:
    return sum;
}

int main(int argc, char *argv[]) {
    int i, total_iterations;
    volatile int selector;
    int total_result = 0;
    
    /* Use command line argument or default */
    if (argc > 1) {
        total_iterations = atoi(argv[1]);
        if (total_iterations <= 0) total_iterations = 100000;
    } else {
        total_iterations = 100000;
    }
    
    /* Seed random for variability */
    srand(time(NULL));
    
    printf("Running %d iterations to stress register allocator...\n", total_iterations);
    
    /* Hot loop calling high-pressure functions */
    for (i = 0; i < total_iterations; i++) {
        /* Vary selector to exercise different control paths */
        selector = rand() % 20 - 5;  /* Range -5 to 14 */
        
        /* Call the register pressure function */
        total_result += register_pressure_function(selector);
        
        /* Periodically call nested loop function */
        if (i % 100 == 0) {
            total_result += nested_loop_function(i);
        }
        
        /* More register pressure with inline asm */
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    }
    
    printf("Total result: %d\n", total_result);
    
    /* Additional test with different optimization patterns */
    {
        int array[100];
        for (i = 0; i < 100; i++) {
            array[i] = register_pressure_function(i % 15);
        }
        
        /* Force computation to prevent elimination */
        int check = 0;
        for (i = 0; i < 100; i++) {
            check ^= array[i];
        }
        printf("Array checksum: %d\n", check);
    }
    
    return 0;
}

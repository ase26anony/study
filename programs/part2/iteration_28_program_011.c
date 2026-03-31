/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with extreme register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector, volatile int iter) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Use inline assembly to clobber registers (x86 version) */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                 "xmm0", "xmm1", "xmm2", "xmm3");
    
    /* Complex switch with irreducible control flow via goto */
    volatile int local_selector = selector + iter;
    
    /* Label for goto jumps to create irreducible CFG */
    start_switch:
    switch (local_selector % 12) {
        case 0:
            a = b + c;
            b = c * d;
            /* Jump to another case */
            if (a > 10) goto case_5;
            break;
        case 1:
            c = d - e;
            d = e / (a ? a : 1);
            f1 = f2 + f3;
            goto case_10;
        case 2:
            s1 = s2 + s3;
            s2 = s3 - s1;
            ch1 = ch2 + 1;
            /* Another goto to create loop in CFG */
            if (iter % 3 == 0) goto case_8;
            break;
        case 3:
            l1 = l2 * l3;
            l2 = l3 + l1;
            d1 = d2 * 2.0;
            goto end_computation;
        case 4:
            u1 = u2 ^ iter;
            u2 = u1 | 0xFF;
            f2 = f3 * 1.5f;
            break;
        case_5:
        case 5:
            a = a * b + c;
            b = b - c * d;
            /* Clobber more registers */
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            if (b < 0) goto case_11;
            break;
        case 6:
            c = (d << 2) | e;
            d = (e >> 1) & a;
            ch2 = ch3 - 1;
            goto case_3;
        case 7:
            s3 = s1 * s2;
            f3 = f1 / f2;
            /* Complex floating point operations */
            f1 = f2 * f3 - f1;
            break;
        case_8:
        case 8:
            l3 = l1 ^ l2;
            d2 = d1 + 3.14;
            /* Jump back to start */
            if (iter % 7 == 0) goto start_switch;
            break;
        case 9:
            u1 = u1 + u2;
            u2 = u2 - u1;
            ch3 = ch1 ^ ch2;
            goto case_2;
        case_10:
        case 10:
            a = b ^ c ^ d;
            b = c | d & e;
            /* More register clobbering */
            asm volatile("" : : : "xmm4", "xmm5", "xmm6", "xmm7");
            break;
        case_11:
        case 11:
            /* Mix all types */
            a = (int)f1 + (int)d1 + s1 + ch1;
            b = (int)f2 * (int)l1 + u1;
            f3 = (float)(a * b) / 100.0f;
            d2 = (double)(c * d) / 1000.0;
            break;
        default:
            /* Should never reach here with modulo 12 */
            a = iter;
            break;
    }
    
    end_computation:
    
    /* Force use of all variables to prevent optimization */
    volatile int result = a + b + c + d + e;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)l1 + (int)l2 + (int)l3;
    result += (int)d1 + (int)d2;
    result += u1 + u2;
    
    /* Final register clobber */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another function with different control flow pattern */
NOINLINE static int nested_loop_function(volatile int base) {
    int i, j, k, sum = 0;
    volatile int mod = base % 7 + 1;
    
    /* Nested loops with irregular bounds */
    for (i = 0; i < mod * 2; i++) {
        if (i % 3 == 0) {
            for (j = i; j < mod * 3; j += 2) {
                sum += j;
                /* goto creating irreducible loop */
                if (j % 5 == 0) goto skip_inner;
                for (k = 0; k < (j % 4); k++) {
                    sum -= k;
                    if (sum > 1000) goto outer_loop;
                }
                skip_inner:
                sum += i * j;
            }
        } else {
            /* Different path */
            j = i;
            while (j > 0) {
                sum += j;
                j -= (i % 2) + 1;
                if (sum % 11 == 0) break;
            }
        }
        outer_loop:
        sum += i;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int total = 0;
    int i, result;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Seed random for variability */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high register pressure functions */
    for (i = 0; i < iterations; i++) {
        volatile int selector = rand() % 100;
        
        /* Call the main register pressure function */
        result = register_pressure_function(selector, i);
        total += result;
        
        /* Periodically call the nested loop function */
        if (i % 37 == 0) {
            result = nested_loop_function(selector + i);
            total += result;
        }
        
        /* Add some branching to create more control flow edges */
        if (i % 13 == 0) {
            /* Inline some computation */
            volatile int temp = 0;
            for (int j = 0; j < (i % 20); j++) {
                temp += j * (selector % 5);
                /* Use goto for irreducible flow */
                if (temp > 100) goto skip_add;
            }
            total += temp;
            skip_add:
            total += 1;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}

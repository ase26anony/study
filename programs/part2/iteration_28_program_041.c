/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Use volatile to prevent optimization */
    volatile int control = selector;
    
    /* Complex switch with many cases */
    switch (control & 0xF) {
        case 0:
            a = b + c;
            s1 = s2 - s3;
            /* Irreducible control flow with goto */
            goto case_5_jump;
            
        case 1:
            d = e * a;
            f1 = f2 + f3;
            /* Clobber registers with inline asm */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_8_jump;
            
        case 2:
            ch1 = ch2 ^ ch3;
            l1 = l2 | l3;
            break;
            
        case 3:
            d1 = d2 * 2.0;
            u1 = u2 >> 1;
            /* Another goto creating irreducible flow */
            goto case_0_jump;
            
        case 4:
            a = b * c - d;
            s3 = s1 + s2;
            break;
            
        case_5_jump:
        case 5:
            f3 = f1 * f2;
            ch3 = ch1 + ch2;
            asm volatile("" : : : "esi", "edi");
            goto case_10_jump;
            
        case 6:
            l3 = l1 ^ l2;
            d2 = d1 / 2.0;
            break;
            
        case 7:
            u2 = u1 << 2;
            a = b & c;
            goto case_2_jump;
            
        case_8_jump:
        case 8:
            s2 = s3 * s1;
            f2 = f3 - f1;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
            
        case 9:
            ch2 = ch3 | ch1;
            l2 = l3 & l1;
            /* Create loop-like goto */
            if (a > 0) goto case_12_jump;
            break;
            
        case_10_jump:
        case 10:
            d1 = d2 + 1.0;
            u1 = u2 - 100;
            break;
            
        case 11:
            a = c % b;
            s1 = s2 / 2;
            goto case_7_jump;
            
        case_12_jump:
        case 12:
            f1 = f2 * f3;
            ch1 = ch2 ^ 0x55;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
            
        case_13:
            l1 = l2 + l3;
            d2 = d1 * 3.0;
            goto case_4_jump;
            
        case_14:
            u2 = u1 | 0xFF;
            a = ~b;
            break;
            
        case_15:
            s3 = s1 - s2;
            f3 = f1 / f2;
            /* Final goto back to earlier case */
            goto case_1_jump;
            
        case_0_jump:
            b = a + d;
            ch2 = ch1 * 2;
            break;
            
        case_2_jump:
            c = d - e;
            l3 = l1 << 1;
            break;
            
        case_4_jump:
            e = a ^ b;
            u1 = u2 >> 2;
            break;
            
        case_7_jump:
            d = c * 2;
            s2 = s1 + 10;
            break;
            
        case_1_jump:
            f2 = f1 + 5.0f;
            ch3 = ch2 - 1;
            break;
            
        default:
            /* Mix all operations */
            a = b + c + d + e;
            s1 = s2 + s3;
            ch1 = ch2 + ch3;
            f1 = f2 + f3;
            l1 = l2 + l3;
            d1 = d2 + 1.5;
            u1 = u2 + 500;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                           "rsi", "rdi", "r8", "r9");
            break;
    }
    
    /* More irreducible control flow with nested loops */
    volatile int loop_ctrl = selector % 5;
    
    outer_loop:
    for (int i = 0; i < loop_ctrl; i++) {
        inner_loop:
        for (int j = 0; j < 3; j++) {
            if (j == 1) goto skip_point;
            a += i * j;
            skip_point:
            b -= j;
            
            /* Conditional goto creating irreducible region */
            if ((i + j) % 2 == 0) {
                goto inner_loop;
            }
        }
        
        /* Another goto target */
        if (i == 2) {
            goto outer_loop;
        }
        
        /* Mix variables to force register usage */
        c = a * b;
        d = c / (i + 1);
        e = d ^ 0xAA;
        
        /* More inline asm to clobber registers */
        asm volatile("" : : : "xmm3", "xmm4", "xmm5", "xmm6");
    }
    
    /* Final computation using all variables */
    int result = a + b + c + d + e + 
                 s1 + s2 + s3 + 
                 ch1 + ch2 + ch3 + 
                 (int)f1 + (int)f2 + (int)f3 + 
                 (int)l1 + (int)l2 + (int)l3 + 
                 (int)d1 + (int)d2 + 
                 u1 + u2;
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Parse iteration count from command line or use default */
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Use volatile to prevent optimization */
    volatile int seed = (argc > 2) ? atoi(argv[2]) : time(NULL);
    srand(seed);
    
    printf("Running MCF stress test with %d iterations (seed: %d)\n", 
           iterations, seed);
    
    long long total = 0;
    
    /* Hot loop calling the register pressure function */
    for (int i = 0; i < iterations; i++) {
        /* Volatile selector to prevent constant propagation */
        volatile int selector = rand() % 100;
        
        /* Call the complex function */
        int result = register_pressure_function(selector);
        
        /* Accumulate results to create observable side effect */
        total += result;
        
        /* Occasionally call with special values */
        if (i % 1000 == 0) {
            total += register_pressure_function(0);
            total += register_pressure_function(15);
            total += register_pressure_function(31);
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional test with different optimization patterns */
    if (iterations > 1000) {
        printf("Running additional MCF patterns...\n");
        
        /* Test with different control flow patterns */
        for (int pattern = 0; pattern < 10; pattern++) {
            volatile int special_selector = pattern * 7;
            total += register_pressure_function(special_selector);
        }
        
        printf("Final total: %lld\n", total);
    }
    
    return 0;
}

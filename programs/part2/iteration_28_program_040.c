/* test_mcf.c - Program to trigger MCF algorithm special block printing */
#include <stdio.h>
#include <stdlib.h>
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
    double d1 = 1.11, d2 = 2.22;
    
    /* Use inline assembly to clobber registers (x86-64 example) */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    
    /* Complex switch with irreducible control flow via goto */
    volatile int local_selector = selector;
    int result = 0;
    
switch_start:
    switch (local_selector % 12) {
        case 0:
            a = b + c;
            b = c * d;
            goto case_5;  /* Jump to another case's code */
            
        case 1:
            s1 = s2 - s3;
            ch1 = ch2 + 1;
            f1 = f2 * f3;
            goto case_10;
            
        case 2:
            l1 = l2 / 2;
            d1 = d2 + 1.0;
            /* Fall through */
            
        case 3:
            a = a * 2;
            b = b / 2;
            goto switch_start;  /* Create loop in control flow */
            
        case 4:
            c = d ^ e;
            s3 = s1 | s2;
            goto case_end;
            
        case_5:  /* Label for goto target */
        case 5:
            f3 = f1 + f2;
            ch3 = ch1 - ch2;
            local_selector++;
            goto switch_start;
            
        case 6:
            l3 = l1 << 2;
            d2 = d1 * 2.0;
            goto case_8;
            
        case 7:
            e = a & b;
            s2 = s3 ^ s1;
            /* Complex arithmetic sequence */
            for (int i = 0; i < 5; i++) {
                a += i;
                b -= i;
            }
            break;
            
        case_8:
        case 8:
            ch2 = ch3 * 2;
            f2 = f1 / 2.0f;
            goto case_11;
            
        case 9:
            l2 = l3 >> 1;
            d1 = d2 - 0.5;
            /* Nested conditionals */
            if (a > b) {
                if (c < d) {
                    e = a + b;
                } else {
                    e = a - b;
                }
            }
            break;
            
        case_10:
        case 10:
            s3 = s1 + s2;
            ch1 = ch2 * ch3;
            /* Another goto creating cross-connections */
            goto case_2;
            
        case_11:
        case 11:
            f1 = f3 - f2;
            l1 = l2 + l3;
            /* Multiple operations */
            a = b + c + d + e;
            b = a * 2 / 3;
            break;
            
        case_end:
        default:
            /* Mix all variables */
            result = a + b + c + d + e + s1 + s2 + s3 + 
                    ch1 + ch2 + ch3 + (int)f1 + (int)f2 + 
                    (int)f3 + (int)l1 + (int)l2 + (int)l3 + 
                    (int)d1 + (int)d2;
            break;
    }
    
    /* More register pressure */
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                 "xmm4", "xmm5", "xmm6", "xmm7");
    
    /* Ensure all variables are used */
    return result + a + b + c + d + e + s1 + s2 + s3 + 
           ch1 + ch2 + ch3 + (int)f1 + (int)f2 + (int)f3 + 
           (int)l1 + (int)l2 + (int)l3 + (int)d1 + (int)d2;
}

/* Another complex function to increase graph complexity */
NOINLINE static int secondary_pressure_function(int base) {
    volatile int x = base;
    int arr[20];
    
    /* Initialize array with values */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
    }
    
    /* Complex loop with conditionals */
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            sum += arr[i];
            goto loop_mid;
        } else if (i % 3 == 1) {
            sum -= arr[i];
            goto loop_end;
        } else {
            sum *= arr[i];
        }
        
    loop_mid:
        if (i % 2 == 0) {
            sum >>= 1;
        }
        
    loop_end:
        if (i % 5 == 0) {
            x++;
            goto loop_start_skip;
        }
        
        continue;
        
    loop_start_skip:
        /* Empty */
        ;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int selector = rand();
    
    long long total = 0;
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call main pressure function */
        total += register_pressure_function(selector);
        
        /* Call secondary function occasionally */
        if (i % 7 == 0) {
            total += secondary_pressure_function(selector % 100);
        }
        
        /* More variation */
        if (i % 13 == 0) {
            selector ^= i;
        }
    }
    
    printf("Result: %lld\n", total);
    return 0;
}

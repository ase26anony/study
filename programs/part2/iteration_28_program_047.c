/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
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
    double d1 = 1.01, d2 = 2.02;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Use inline assembly to clobber registers (x86 version) */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    
    /* Complex switch with many cases */
    switch (selector % 12) {
        case 0:
            a = b + c;
            b = d * e;
            /* Jump to another case */
            goto case_5;
            
        case 1:
            s1 = s2 - s3;
            ch1 = ch2 + 1;
            f1 = f2 * f3;
            break;
            
        case 2:
            l1 = l2 / 2;
            d1 = d2 * 2.0;
            /* Another goto creating irreducible flow */
            goto case_8;
            
        case 3:
            u1 = u2 ^ 0xFF;
            a = b | c;
            break;
            
        case 4:
            ch3 = ch1 + ch2;
            s3 = s1 * s2;
            /* Clobber more registers */
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case_end;
            
        case_5:
        case 5:
            f3 = f1 + f2;
            e = d - c;
            /* Complex arithmetic chain */
            a = ((b * c) + (d / e)) % 7;
            break;
            
        case 6:
            l3 = l1 << 2;
            u2 = u1 >> 1;
            d2 = d1 / 3.14;
            break;
            
        case 7:
            /* Nested conditionals */
            if (a > b) {
                c = d + e;
                goto case_9;
            } else {
                c = d - e;
            }
            s1 = s2 + s3;
            break;
            
        case_8:
        case 8:
            ch2 = ch3 - 1;
            f2 = f1 * 3.0f;
            /* Another register clobber */
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
            
        case_9:
        case 9:
            l2 = l3 ^ l1;
            u1 = u2 & 0x0F;
            /* More arithmetic */
            a = (b * c) + (d * e) - (s1 * s2);
            break;
            
        case 10:
            d1 = d2 + 1.5;
            f3 = f1 - f2;
            /* Jump back */
            goto case_3_label;
            
        case_3_label:
        case 11:
            ch1 = ch2 * 2;
            s3 = s1 / 2;
            l1 = l2 + l3;
            break;
            
        case_end:
        default:
            a = b + c + d;
            break;
    }
    
    /* More irreducible control flow with gotos */
    if (a > 100) {
        goto label1;
    } else if (a < 50) {
        goto label2;
    }
    
    label1:
    b = c * d;
    goto label3;
    
    label2:
    b = c / d;
    /* Fall through */
    
    label3:
    /* Final register clobber */
    asm volatile("" : : : "r12", "r13", "r14", "r15", "xmm3", "xmm4", "xmm5");
    
    /* Combine all variables to prevent elimination */
    return a + b + c + d + e + s1 + s2 + s3 + 
           ch1 + ch2 + ch3 + (int)f1 + (int)f2 + (int)f3 + 
           (int)l1 + (int)l2 + (int)l3 + (int)d1 + (int)d2 + 
           u1 + u2;
}

/* Another function to create additional pressure */
NOINLINE static int secondary_pressure(volatile int x) {
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += arr[i];
        /* More register clobbering */
        asm volatile("" : : : "rax", "rbx", "rcx");
    }
    
    return sum;
}

int main(int argc, char **argv) {
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
        selector = (selector * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call main pressure function */
        total += register_pressure_function(selector);
        
        /* Call secondary function occasionally */
        if (i % 7 == 0) {
            total += secondary_pressure(selector % 19);
        }
        
        /* More variation in control flow */
        if (i % 100 == 0) {
            selector ^= 0x5555;
        }
    }
    
    printf("Total: %lld\n", total);
    return 0;
}

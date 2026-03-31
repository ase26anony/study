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
    double db1 = 4.4, db2 = 5.5;
    volatile int control = selector;
    
    /* Complex switch with many cases */
    switch (control & 0xF) {
        case 0:
            a = b + c;
            b = d * e;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto label1;
        
        case 1:
            c = d - e;
            f = g / (h + 1);
            fl1 = fl2 * fl3;
            goto label3;
        
        case 2:
        label1:
            s1 = s2 + s3;
            ch1 = ch2 + 1;
            asm volatile("" : : : "esi", "edi");
            goto label2;
        
        case 3:
            db1 = db2 * 2.0;
            i = j << 2;
            /* Fall through */
        
        case 4:
            a = (b & c) | d;
            goto label4;
        
        case 5:
        label2:
            fl2 = fl1 + fl3;
            e = f ^ g;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 6:
            h = i % (j + 1);
            s2 = s1 - s3;
            goto label5;
        
        case 7:
        label3:
            ch3 = ch1 * 2;
            db2 = db1 / 2.0;
            asm volatile("" : : : "r11", "r12");
            /* Fall through */
        
        case 8:
            fl3 = fl1 - fl2;
            c = d >> 1;
            break;
        
        case 9:
        label4:
            a = b * c * d;
            fl1 = (float)a / 10.0f;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto label6;
        
        case 10:
            s3 = s1 + s2;
            ch2 = ch3 - 1;
            /* Fall through */
        
        case 11:
        label5:
            db1 = a + b + c;
            e = f | g;
            asm volatile("" : : : "xmm3", "xmm4");
            break;
        
        case 12:
            h = i & j;
            fl2 = fl3 * 2.0f;
            goto label7;
        
        case 13:
        label6:
            a = ~b;
            s1 = s2 * s3;
            asm volatile("" : : : "xmm5", "xmm6", "xmm7");
            break;
        
        case 14:
            ch1 = ch2 ^ ch3;
            db2 = db1 - 1.0;
            /* Fall through */
        
        case 15:
        label7:
            fl3 = fl1 + fl2 + fl3;
            j = i ^ h;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
            break;
        
        default:
            /* Create irreducible loop with goto */
            if (a > 0) {
                goto label8;
            } else {
                goto label9;
            }
    }
    
    /* More irreducible control flow */
    if (a % 2 == 0) {
        goto label10;
    }
    
label8:
    b = c + d;
    goto label11;

label9:
    e = f - g;
    goto label12;

label10:
    h = i * j;
    /* Another register clobber */
    asm volatile("" : : : "r13", "r14", "r15");

label11:
    s1 = s2 + s3;
    if (ch1 > 'M') {
        goto label13;
    }

label12:
    ch2 = ch3 + 1;
    fl1 = fl2 * 3.14f;

label13:
    /* Final computation using all variables */
    int result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)fl1 + (int)fl2 + (int)fl3;
    result += (int)db1 + (int)db2;
    
    return result;
}

/* Another complex function to increase graph complexity */
NOINLINE static int nested_control_flow(volatile int x) {
    int sum = 0;
    
    /* Nested loops with gotos */
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            goto loop2_start;
        }
        
        for (int j = 0; j < 3; j++) {
            sum += i * j;
            if (sum > 100) {
                goto loop1_end;
            }
        }
        
    loop2_start:
        for (int k = 0; k < 4; k++) {
            sum -= k;
            asm volatile("" : : : "eax", "ebx");
        }
    }
    
loop1_end:
    /* Switch inside switch */
    switch (x & 0x3) {
        case 0:
            switch (sum & 0x1) {
                case 0: sum += 10; break;
                case 1: sum += 20; goto switch_exit;
            }
            break;
        case 1:
            sum *= 2;
            goto switch_exit;
        case 2:
            sum /= 2;
            break;
    }
    
switch_exit:
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different control paths */
        selector = rand() & 0xFF;
        
        /* Call the high register pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call nested control flow function */
        int result2 = nested_control_flow(selector >> 1);
        
        /* Mix results to prevent optimization */
        total += result1 + result2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            selector = i;
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Prevent dead code elimination */
    volatile int dummy = total;
    asm volatile("" : : "r"(dummy));
    
    return 0;
}

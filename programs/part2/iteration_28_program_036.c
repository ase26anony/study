/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
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
    long l1 = 100, l2 = 200;
    double d1 = 10.5, d2 = 20.5;
    
    /* Volatile to prevent optimization */
    volatile int control = selector;
    
    /* Irreducible control flow using goto */
    if (control < 0) goto negative_case;
    
    /* Large switch with many cases */
    switch (control % 20) {
        case 0:
            a = b + c;
            s1 = s2 - s3;
            /* Clobber registers (x86) */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_common;
            
        case 1:
            c = d * e;
            f1 = f2 + f3;
            asm volatile("" : : : "esi", "edi");
            goto case_common;
            
        case 2:
            d = a ^ b;
            ch1 = ch2 + 1;
            asm volatile("" : : : "eax", "ebx");
            goto negative_case;
            
        case 3:
            e = c | d;
            l1 = l2 >> 1;
            asm volatile("" : : : "ecx", "edx");
            goto case_common;
            
        case 4:
            s2 = s1 * s3;
            d1 = d2 * 2.0;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case_common;
            
        case 5:
            s3 = s1 + s2;
            f2 = f1 * f3;
            asm volatile("" : : : "xmm2", "xmm3");
            goto negative_case;
            
        case 6:
            ch2 = ch1 - ch3;
            a = b << 2;
            asm volatile("" : : : "eax", "ebx", "ecx");
            goto case_common;
            
        case 7:
            ch3 = ch1 + ch2;
            b = c >> 1;
            asm volatile("" : : : "edx", "esi");
            goto case_common;
            
        case 8:
            f3 = f1 - f2;
            l2 = l1 * 2;
            asm volatile("" : : : "edi", "ebp");
            goto negative_case;
            
        case 9:
            l1 = l2 + a;
            d2 = d1 / 2.0;
            asm volatile("" : : : "xmm4", "xmm5");
            goto case_common;
            
        case 10:
            a = b * c + d;
            s1 = s2 | s3;
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi");
            goto case_common;
            
        case 11:
            b = a ^ c ^ d;
            ch1 = ch2 & ch3;
            asm volatile("" : : : "edi", "ebp");
            goto negative_case;
            
        case 12:
            c = (a + b) * (d - e);
            f1 = f2 * f3 + 1.0f;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case_common;
            
        case 13:
            d = (b << 3) | (c >> 2);
            s2 = s1 ^ s3;
            asm volatile("" : : : "eax", "ebx", "ecx");
            goto case_common;
            
        case 14:
            e = a * b * c * d;
            l1 = l2 << 2;
            asm volatile("" : : : "edx", "esi", "edi");
            goto negative_case;
            
        case 15:
            s3 = (s1 + s2) * 2;
            d1 = d2 * d2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            goto case_common;
            
        case 16:
            ch2 = ch1 * 2 + ch3;
            f2 = f1 + f3 * 2.0f;
            asm volatile("" : : : "eax", "ebx");
            goto case_common;
            
        case 17:
            ch3 = ch2 - ch1 + 64;
            l2 = l1 | 0xFF;
            asm volatile("" : : : "ecx", "edx");
            goto negative_case;
            
        case 18:
            f3 = (f1 + f2) / 2.0f;
            a = b + c + d + e;
            asm volatile("" : : : "esi", "edi", "ebp");
            goto case_common;
            
        case 19:
            d2 = d1 * 3.14159;
            s1 = s2 & s3;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
            goto case_common;
            
        default:
            a = 0;
            goto case_common;
    }

case_common:
    /* Common code block reachable from multiple cases */
    a = a + b + c;
    s1 = s1 + s2 + s3;
    ch1 = ch1 + ch2 + ch3;
    f1 = f1 + f2 + f3;
    l1 = l1 + l2;
    d1 = d1 + d2;
    
    /* More register clobbering */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp");
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7");
    
    return a + s1 + ch1 + (int)f1 + (int)l1 + (int)d1;

negative_case:
    /* Another reachable block creating irreducible flow */
    a = a - b - c;
    s1 = s1 - s2 - s3;
    ch1 = ch1 - ch2 - ch3;
    f1 = f1 - f2 - f3;
    l1 = l1 - l2;
    d1 = d1 - d2;
    
    /* Jump back to switch or common block based on condition */
    if (control % 3 == 0) {
        asm volatile("" : : : "eax", "ebx");
        goto case_common;
    } else {
        asm volatile("" : : : "ecx", "edx", "esi");
        return a - s1 - ch1 - (int)f1 - (int)l1 - (int)d1;
    }
}

/* Another complex function to increase overall complexity */
NOINLINE static int secondary_pressure_function(int x) {
    volatile int y = x;
    int r = 0;
    
    /* Loop with switch inside */
    for (int i = 0; i < 10; i++) {
        switch ((y + i) % 7) {
            case 0: r += x * i; break;
            case 1: r += x / (i + 1); break;
            case 2: r += x << (i % 4); break;
            case 3: r += x >> (i % 4); break;
            case 4: r += x ^ i; break;
            case 5: r += x | i; break;
            case 6: r += x & i; break;
        }
        asm volatile("" : : : "eax", "ebx");
    }
    
    return r;
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
    
    /* Initialize with time-based seed */
    srand(time(NULL));
    selector = rand() % 100;
    
    printf("Running MCF test with %d iterations...\n", iterations);
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different control paths */
        volatile int sel = (selector + i) % 50;
        
        /* Call main pressure function */
        total += register_pressure_function(sel);
        
        /* Call secondary function occasionally */
        if (i % 17 == 0) {
            total += secondary_pressure_function(sel);
        }
        
        /* Modify selector to create varying patterns */
        if (i % 1000 == 0) {
            selector = (selector * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

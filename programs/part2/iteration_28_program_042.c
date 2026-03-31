/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with extreme register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many variables of different types to stress register allocation */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector; /* Prevent optimization */
    
    /* Complex switch with many cases creating multiple basic blocks */
    switch (control % 12) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto label1;
        
        case 1:
            g = h << 2;
            s1 = s2 + s3;
            asm volatile("" : : : "esi", "edi");
            /* Jump to another case block */
            if (ch1 > 'A') goto case3;
            break;
        
        case 2:
        label1:
            fl1 = fl2 * 2.0f;
            ch2 = ch3 + 1;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 3:
        case3:
            a = d - e;
            f = g / 2;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            /* Create irreducible loop with goto */
            if (s4 > 0) goto label2;
            break;
        
        case 4:
            b = c * d;
            e = f + g;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto label3;
        
        case 5:
        label2:
            fl3 = fl1 + fl2;
            s4 = s1 - s2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            /* Another goto creating complex flow */
            if (ch3 < 'Z') goto label4;
            break;
        
        case 6:
            h = a | b;
            c = d ^ e;
            asm volatile("" : : : "xmm6", "xmm7");
            break;
        
        case 7:
        label3:
            ch1 = ch2 * 2;
            fl2 = fl3 / 1.5f;
            asm volatile("" : : : "mm0", "mm1");
            goto label5;
        
        case 8:
            s3 = s4 << 1;
            a = b & c;
            asm volatile("" : : : "st", "st(1)", "st(2)");
            break;
        
        case 9:
        label4:
            d = e + f + g;
            ch3 = ch1 - ch2;
            asm volatile("" : : : "st(3)", "st(4)");
            /* Nested conditional goto */
            if (fl1 > 0.0f) {
                if (s1 < 100) goto label6;
            }
            break;
        
        case 10:
        label5:
            f = g * h;
            s2 = s3 >> 1;
            asm volatile("" : : : "st(5)", "st(6)");
            break;
        
        case 11:
        default:
        label6:
            fl1 = fl2 + fl3;
            a = b - c + d - e;
            asm volatile("" : : : "st(7)");
            /* Final irreducible jump */
            if (ch2 != 0) goto label1;
            break;
    }
    
    /* Use all variables to prevent elimination */
    return a + b + c + d + e + f + g + h + 
           s1 + s2 + s3 + s4 + 
           ch1 + ch2 + ch3 + 
           (int)fl1 + (int)fl2 + (int)fl3;
}

/* Another function with different control flow pattern */
NOINLINE static int secondary_pressure_function(volatile int x) {
    int v1 = x, v2 = x*2, v3 = x*3, v4 = x*4;
    int v5 = x*5, v6 = x*6, v7 = x*7, v8 = x*8;
    volatile int branch = x % 7;
    
    /* Complex if-else chain with gotos */
    if (branch == 0) {
        v1 = v2 + v3;
        asm volatile("" : : : "eax", "ebx");
        goto block_a;
    } else if (branch == 1) {
        v4 = v5 - v6;
        asm volatile("" : : : "ecx", "edx");
        goto block_c;
    } else if (branch == 2) {
    block_a:
        v7 = v8 * v1;
        asm volatile("" : : : "esi", "edi");
        goto block_b;
    } else if (branch == 3) {
        v2 = v3 / 2;
        asm volatile("" : : : "r8", "r9");
        goto block_d;
    } else if (branch == 4) {
    block_b:
        v5 = v6 | v7;
        asm volatile("" : : : "r10", "r11");
        goto block_e;
    } else if (branch == 5) {
    block_c:
        v8 = v1 ^ v2;
        asm volatile("" : : : "r12", "r13");
        goto block_a;
    } else {
    block_d:
        v3 = v4 & v5;
        asm volatile("" : : : "r14", "r15");
    block_e:
        v6 = v7 << 1;
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
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
    
    srand(time(NULL));
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        selector = rand() % 100;
        
        /* Alternate between different pressure functions */
        if (i % 3 == 0) {
            total += register_pressure_function(selector);
        } else if (i % 3 == 1) {
            total += secondary_pressure_function(selector);
        } else {
            /* Mix in some direct computation */
            int temp = selector;
            asm volatile("" : "+r" (temp) : : "eax", "ebx", "ecx", "edx");
            total += temp;
        }
        
        /* Occasionally change control flow pattern */
        if (i % 1000 == 0) {
            selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

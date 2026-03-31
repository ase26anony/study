/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Force no inlining to preserve complex control flow */
__attribute__((noinline, noipa))
static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    short s1 = 100, s2 = 200, s3 = 300;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    volatile int control = selector; /* Prevent optimization */
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto case_negative;
    
    /* Large switch with many cases creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5; /* Create cross-block jumps */
        
        case 1:
            g = h - i;
            j = k / 2;
            f1 = f2 + f3;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 2:
            m = n ^ o;
            p = a | b;
            s1 = s2 + s3;
            goto case_8;
        
        case 3:
            ch1 = ch2 + 1;
            ch3 = ch1 - 2;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 4:
            a = b * c * d;
            e = f + g + h;
            goto case_12;
        
        case 5:
        case_5:
            i = j << 2;
            k = l >> 1;
            asm volatile("" : : : "r11", "r12", "r13");
            break;
        
        case 6:
            f2 = f1 * 2.0f;
            f3 = f2 / 1.5f;
            goto case_14;
        
        case 7:
            m = n + o + p;
            a = b - c;
            asm volatile("" : : : "xmm0", "xmm1");
            break;
        
        case 8:
        case_8:
            s2 = s3 * 2;
            s1 = s2 - 100;
            ch2 = ch1 + ch3;
            goto case_18;
        
        case 9:
            d = e ^ f ^ g;
            h = i & j;
            asm volatile("" : : : "xmm2", "xmm3", "xmm4");
            break;
        
        case 10:
            k = l * m;
            n = o / 2;
            f1 = f2 - f3;
            goto case_1;
        
        case 11:
            p = a + b + c + d;
            asm volatile("" : : : "r14", "r15");
            break;
        
        case 12:
        case_12:
            e = f * g * h;
            i = j - k;
            s3 = s1 + s2;
            goto case_6;
        
        case 13:
            ch3 = ch1 * 2;
            m = n % 7;
            asm volatile("" : : : "xmm5", "xmm6");
            break;
        
        case 14:
        case_14:
            o = p << 3;
            a = b >> 2;
            f3 = f1 + f2;
            goto case_10;
        
        case 15:
            c = d ^ e;
            f = g | h;
            asm volatile("" : : : "xmm7", "xmm8", "xmm9");
            break;
        
        case 16:
            i = j * k * l;
            m = n + o;
            goto case_3;
        
        case 17:
            s1 = s2 / 2;
            ch1 = ch2 - 1;
            asm volatile("" : : : "xmm10", "xmm11");
            break;
        
        case 18:
        case_18:
            p = a & b & c;
            d = e + f + g;
            f2 = f1 * 3.0f;
            goto case_16;
        
        case 19:
            h = i ^ j ^ k;
            l = m | n;
            asm volatile("" : : : "xmm12", "xmm13", "xmm14");
            break;
        
        default:
            goto case_0;
    }
    
    /* Additional irreducible region */
    if (control % 3 == 0) {
        goto extra_computation;
    }
    
    /* Return a value using all variables to prevent elimination */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p
           + s1 + s2 + s3 + ch1 + ch2 + ch3 + (int)f1 + (int)f2 + (int)f3;

extra_computation:
    /* More operations creating additional basic blocks */
    a = b * c;
    d = e / 2;
    goto finish;

case_negative:
    /* Alternative entry point creating complex CFG */
    a = -b;
    c = -d;
    if (control % 2 == 0) {
        goto case_5;
    } else {
        goto case_12;
    }

case_0:
    a = 0;
    b = 0;
    goto finish;

finish:
    /* Final computation using all variables */
    return (a * b) + (c * d) + (e * f) + (g * h) + (i * j) + 
           (k * l) + (m * n) + (o * p) + s1 + s2 + s3;
}

/* Another complex function to increase overall pressure */
__attribute__((noinline, noipa))
static int secondary_pressure(volatile int x) {
    int v1 = x, v2 = x*2, v3 = x*3, v4 = x*4;
    int v5 = x*5, v6 = x*6, v7 = x*7, v8 = x*8;
    
    /* Complex loop with conditionals */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            v1 += v2;
            v3 -= v4;
            asm volatile("" : : : "rax", "rbx");
        } else {
            v5 *= v6;
            v7 /= v8;
            asm volatile("" : : : "rcx", "rdx");
        }
        
        /* Nested switch */
        switch (i % 4) {
            case 0: v1++; break;
            case 1: v2--; break;
            case 2: v3 ^= v4; break;
            case 3: v5 |= v6; break;
        }
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int total = 0;
    
    /* Use command line or default iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling pressure functions */
    for (int count = 0; count < iterations; count++) {
        volatile int selector = rand() % 100;
        
        /* Call both pressure functions to increase overall register pressure */
        int result1 = register_pressure_function(selector);
        int result2 = secondary_pressure(selector % 50);
        
        total += result1 + result2;
        
        /* Occasionally flush to prevent optimization */
        if (count % 10000 == 0) {
            asm volatile("" : : "r"(total) : "memory");
        }
    }
    
    printf("Total result: %d\n", total);
    
    return 0;
}

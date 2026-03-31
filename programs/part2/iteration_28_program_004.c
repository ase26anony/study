/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short f = 6, g = 7, h = 8, i = 9, j = 10;
    char k = 11, l = 12, m = 13, n = 14, o = 15;
    float p = 16.0f, q = 17.0f, r = 18.0f;
    double s = 19.0, t = 20.0;
    unsigned int u = 21, v = 22, w = 23;
    
    /* Volatile to prevent optimization */
    volatile int control = selector;
    
    /* Complex switch with many cases */
    switch (control % 12) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case1_label;
        
        case1_label:
        case 1:
            g = h - i;
            j = k * l;
            p = q + r;
            asm volatile("" : : : "esi", "edi");
            if (a > 10) goto case3_label;
            break;
        
        case 2:
            m = n | o;
            s = t / 2.0;
            asm volatile("" : : : "r8", "r9", "r10");
            /* Fall through */
        
        case3_label:
        case 3:
            u = v ^ w;
            a = b << 2;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            goto case5_label;
        
        case 4:
            c = d >> 1;
            e = f & g;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case5_label:
        case 5:
            h = i + j;
            k = l - m;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            if (u < 50) goto case7_label;
            break;
        
        case 6:
            n = o * 2;
            p = q - r;
            asm volatile("" : : : "xmm6", "xmm7");
            /* Fall through */
        
        case7_label:
        case 7:
            s = t * 3.0;
            u = v / 2;
            asm volatile("" : : : "rax", "rbx", "rcx");
            goto case9_label;
        
        case 8:
            w = a + b;
            c = d - e;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            break;
        
        case9_label:
        case 9:
            f = g * h;
            i = j | k;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            if (p > 20.0f) goto case11_label;
            break;
        
        case 10:
            l = m ^ n;
            o = a << 1;
            asm volatile("" : : : "r12", "r13", "r14");
            /* Fall through */
        
        case11_label:
        case 11:
            b = c >> 2;
            d = e & f;
            asm volatile("" : : : "r15", "xmm8", "xmm9");
            goto end_label;
        
        default:
            a = b + c + d;
            asm volatile("" : : : "xmm10", "xmm11", "xmm12");
            break;
    }
    
    /* Additional irreducible control flow with gotos */
    if (a < 100) {
        goto loop1;
    } else {
        goto loop2;
    }
    
loop1:
    for (int x = 0; x < 5; x++) {
        a += x;
        if (x == 3) goto loop2;
    }
    goto end_label;
    
loop2:
    for (int y = 0; y < 5; y++) {
        b += y;
        if (y == 2) goto loop1;
    }
    
end_label:
    /* Use all variables to prevent elimination */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + (int)p + (int)q + (int)r + 
           (int)s + (int)t + u + v + w;
}

/* Another function to create more register pressure */
NOINLINE static int secondary_pressure(volatile int x) {
    int a1 = x, a2 = x*2, a3 = x*3, a4 = x*4, a5 = x*5;
    int b1 = x+1, b2 = x+2, b3 = x+3, b4 = x+4, b5 = x+5;
    
    /* Complex conditional jumps */
    if (x % 3 == 0) {
        asm volatile("" : : : "eax", "ebx");
        goto block_a;
    } else if (x % 3 == 1) {
        asm volatile("" : : : "ecx", "edx");
        goto block_b;
    } else {
        asm volatile("" : : : "esi", "edi");
        goto block_c;
    }
    
block_a:
    a1 = a2 * a3;
    if (a1 > 100) goto block_d;
    return a1 + b1;
    
block_b:
    a2 = a3 + a4;
    if (a2 < 50) goto block_a;
    return a2 + b2;
    
block_c:
    a3 = a4 - a5;
    if (a3 == 0) goto block_b;
    return a3 + b3;
    
block_d:
    a4 = a5 / 2;
    return a4 + b4;
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
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = rand() % 100;
        
        /* Call both pressure functions */
        int result1 = register_pressure_function(selector);
        int result2 = secondary_pressure(selector % 10);
        
        total += result1 + result2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            selector = i;
        }
    }
    
    printf("Total: %lld\n", total);
    return 0;
}

/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile int control = selector; /* Prevent optimization */
    
    /* Complex irreducible control flow with goto */
    if (control < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5; /* Create irreducible flow */
        
        case 1:
            d = e * f;
            f1 = f2 + f3;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 2:
            g = h / (a + 1);
            ch1 = ch2 + 1;
            goto case_10;
        
        case 3:
            i = j - k;
            s1 = s2 * 2;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
        
        case 4:
            l = m ^ n;
            f4 = f1 * 2.0f;
            goto case_15;
        
        case_5: /* Label for goto target */
        case 5:
            o = p << 2;
            s3 = s4 / 2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case 6:
            a = c * d + e;
            ch3 = ch4 - 1;
            goto case_1;
        
        case 7:
            f = g ^ h;
            f2 = f3 * 1.5f;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            break;
        
        case 8:
            i = k + l;
            s2 = s1 * 3;
            goto case_3;
        
        case 9:
            m = n | o;
            ch2 = ch1 * 2;
            break;
        
        case_10: /* Another goto target */
        case 10:
            p = a & b;
            f3 = f4 / 2.0f;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            goto case_7;
        
        case 11:
            c = d - e;
            s4 = s3 + 100;
            break;
        
        case 12:
            g = h * i;
            ch4 = ch3 + 5;
            goto case_negative;
        
        case 13:
            j = k / (l + 1);
            f1 = f2 - f3;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
            break;
        
        case 14:
            n = o ^ p;
            s1 = s2 * s3;
            goto case_8;
        
        case_15: /* Goto target */
        case 15:
            a = b + c + d;
            f4 = f1 + f2 + f3;
            asm volatile("" : : : "xmm6", "xmm7", "xmm8", "xmm9");
            break;
        
        case 16:
            e = f * g;
            ch1 = ch2 + ch3;
            goto case_12;
        
        case 17:
            h = i & j;
            s2 = s3 | s4;
            break;
        
        case 18:
            k = l ^ m;
            f2 = f3 * f4;
            asm volatile("" : : : "rsi", "rdi", "rbp");
            goto case_13;
        
        case 19:
            n = o + p;
            ch3 = ch4 * 2;
            break;
        
        case_negative: /* Goto target from multiple places */
        default:
            a = b - c + d - e;
            f1 = f2 * 3.14f;
            asm volatile("" : : : "xmm10", "xmm11", "xmm12");
            break;
    }
    
    /* More complex control flow with nested loops */
    for (int x = 0; x < 5; x++) {
        if (x % 2 == 0) {
            for (int y = 0; y < 3; y++) {
                a += b * y;
                if (y == 1) goto loop_exit;
            }
        } else {
            b += c * x;
        }
    }
    loop_exit:
    
    /* Use all variables in computation to prevent elimination */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p
                 + s1 + s2 + s3 + s4 + ch1 + ch2 + ch3 + ch4
                 + (int)f1 + (int)f2 + (int)f3 + (int)f4;
    
    return result;
}

/* Another high-pressure function with different pattern */
__attribute__((noinline, noipa))
static int secondary_pressure_function(volatile int mode) {
    int vars[20];
    for (int i = 0; i < 20; i++) vars[i] = i + mode;
    
    /* Complex arithmetic with many intermediate values */
    int sum = 0;
    volatile int counter = mode % 10;
    
    while (counter-- > 0) {
        for (int i = 0; i < 19; i++) {
            vars[i] = vars[i] * vars[i+1] + (i % 3);
            asm volatile("" : : : "memory"); /* Memory clobber */
        }
        sum += vars[counter % 20];
    }
    
    /* More register pressure */
    float fsum = 0.0f;
    for (int i = 0; i < 20; i++) {
        fsum += vars[i] * 0.5f;
    }
    
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
    return sum + (int)fsum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int seed = rand(); /* Prevent optimization */
    long long total = 0;
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        volatile int selector = seed + i;
        
        /* Call first high-pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call second high-pressure function with different pattern */
        int result2 = secondary_pressure_function(selector ^ 0x55AA);
        
        /* Mix results to create data dependencies */
        total += result1 + result2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

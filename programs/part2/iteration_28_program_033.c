/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            g = h - i;
            j = a * b;
            fl1 = fl2 + fl3;
            goto case_10;
        
        case 2:
            s1 = s2 + s3;
            ch1 = ch2 + 1;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 3:
            a = (b << 2) | (c & 0xF);
            d = e ^ f;
            goto case_7;
        
        case 4:
            fl2 = fl1 * 2.0f;
            s2 = s1 * 3;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 5:
        case_5:
            h = i * j + a;
            ch2 = ch1 - 'A' + 'D';
            asm volatile("" : : : "r11", "r12");
            if (d > 10) goto case_15;
            break;
        
        case 6:
            b = c * d - e;
            f = g / 2;
            fl3 = fl1 + fl2;
            goto case_2;
        
        case 7:
        case_7:
            i = j << 1;
            s3 = s1 + s2;
            asm volatile("" : : : "xmm0", "xmm1");
            break;
        
        case 8:
            c = d ^ e ^ f;
            ch3 = ch1 + ch2;
            if (g < 5) goto case_12;
            break;
        
        case 9:
            e = f * g / h;
            fl1 = fl2 - fl3;
            asm volatile("" : : : "xmm2", "xmm3", "xmm4");
            goto case_18;
        
        case 10:
        case_10:
            j = a + b + c + d;
            s1 = s2 * s3;
            break;
        
        case 11:
            a = b | c;
            d = e & f;
            goto case_1;
        
        case 12:
        case_12:
            g = h % i;
            ch1 = ch3 - 1;
            asm volatile("" : : : "r13", "r14", "r15");
            break;
        
        case 13:
            b = c * 3 + d;
            fl2 = fl1 / 2.0f;
            if (j > 20) goto case_5;
            break;
        
        case 14:
            f = g ^ h;
            s2 = s3 << 1;
            asm volatile("" : : : "rax", "rbx", "rcx");
            goto case_8;
        
        case 15:
        case_15:
            h = i + j * 2;
            ch2 = ch1 + 5;
            break;
        
        case 16:
            c = d - e + f;
            fl3 = fl1 * fl2;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            goto case_11;
        
        case 17:
            e = f / g * h;
            s3 = s1 | s2;
            break;
        
        case 18:
        case_18:
            i = j - a - b;
            ch3 = ch2 - 'B' + 'E';
            asm volatile("" : : : "xmm5", "xmm6");
            if (c > 15) goto case_12;
            break;
        
        case 19:
            a = b * c - d;
            fl1 = fl2 + fl3 * 2.0f;
            goto case_0;
        
        default:
        case_negative:
            /* More register pressure */
            a = b + c + d + e + f;
            g = h * i / j;
            asm volatile("" : : : "xmm7", "xmm8", "xmm9", "xmm10");
            break;
    }
    
    /* Additional irreducible loop */
    for (int k = 0; k < 3; k++) {
        if (k == 0) goto loop_part1;
        if (k == 1) goto loop_part2;
        
        loop_part1:
        a += b;
        b += c;
        continue;
        
        loop_part2:
        c += d;
        d += e;
        asm volatile("" : : : "rsp");
    }
    
    /* Use all variables to prevent elimination */
    result = a + b + c + d + e + f + g + h + i + j +
             s1 + s2 + s3 + ch1 + ch2 + ch3 +
             (int)fl1 + (int)fl2 + (int)fl3;
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector;
    long long total = 0;
    
    /* Use command line or random seed */
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    if (iterations <= 0) iterations = 100000;
    
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling the high-pressure function */
    for (int i = 0; i < iterations; i++) {
        selector = rand() % 40 - 10;  /* Range: -10 to 29 */
        int result = register_pressure_function(selector);
        total += result;
        
        /* Occasionally change control flow pattern */
        if (i % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional test with different optimization patterns */
    volatile int alt_selector = 999;
    int alt_result = register_pressure_function(alt_selector);
    printf("Alt result: %d\n", alt_result);
    
    return 0;
}

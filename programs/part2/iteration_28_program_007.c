/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    
    /* Use goto to create irreducible control flow */
    if (selector < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (selector % 11) {
        case 0:
            a = b + c;
            d = e * 2;
            /* Clobber registers on x86 */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_common;
            
        case 1:
            s1 = s2 - s3;
            ch1 = ch2 + 1;
            asm volatile("" : : : "esi", "edi");
            f1 = f2 * f3;
            goto case_common;
            
        case 2:
            l1 = l2 / 2;
            d1 = d2 * 3.14;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case_5;
            
        case 3:
            a = c * d - e;
            s1 = s3 << 2;
            asm volatile("" : : : "rax", "rbx", "rcx");
            goto case_common;
            
        case 4:
            f1 = f2 + f3;
            ch3 = ch1 * 2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case_negative;
            
        case 5:
        case_5:
            l3 = l1 + l2;
            d2 = d1 / 2.0;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            /* Fall through */
            
        case 6:
            b = a ^ c;
            s2 = s1 | s3;
            asm volatile("" : : : "eax", "ebx");
            goto case_common;
            
        case 7:
            f3 = f1 * f2;
            ch2 = ch3 - 1;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            goto case_8;
            
        case 8:
        case_8:
            e = d % 3;
            l2 = l3 << 1;
            asm volatile("" : : : "rdi", "rsi");
            /* Intentional fallthrough */
            
        case 9:
            c = a + b + d;
            s3 = s1 + s2;
            asm volatile("" : : : "r8", "r9");
            goto case_common;
            
        case 10:
            d1 = d2 * d2;
            f2 = f1 + f3;
            asm volatile("" : : : "xmm6", "xmm7", "xmm8", "xmm9");
            goto case_common;
            
        default:
            a = 0;
            goto case_common;
    }
    
case_negative:
    if (selector < -10) {
        a = -a;
        b = -b;
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
        goto case_common;
    } else {
        c = -c;
        d = -d;
        asm volatile("" : : : "r10", "r11", "r12", "r13");
        /* Loop to create more basic blocks */
        for (int i = 0; i < 3; i++) {
            e += i;
            asm volatile("" : : : "rax");
        }
    }
    
case_common:
    /* Complex computation using all variables to prevent elimination */
    int result = a + b + c + d + e;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)l1 + (int)l2 + (int)l3;
    result += (int)d1 + (int)d2;
    
    /* More register pressure */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another function with different control flow pattern */
__attribute__((noinline, noipa))
int secondary_pressure_function(volatile int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int y1 = 6, y2 = 7, y3 = 8, y4 = 9, y5 = 10;
    
    /* Nested switches */
    switch (mode % 7) {
        case 0:
            if (mode & 1) {
                x1 = x2 + x3;
                asm volatile("" : : : "eax", "ebx");
                goto label_a;
            } else {
                x4 = x5 * 2;
                asm volatile("" : : : "ecx", "edx");
                goto label_b;
            }
            
        case 1:
        label_a:
            y1 = y2 - y3;
            asm volatile("" : : : "esi", "edi");
            switch (mode % 3) {
                case 0: x1++; break;
                case 1: x2++; break;
                case 2: x3++; break;
            }
            break;
            
        case 2:
        label_b:
            y4 = y5 / 2;
            asm volatile("" : : : "r8", "r9");
            for (int i = 0; i < 4; i++) {
                x5 += i;
                asm volatile("" : : : "rax");
                if (i == 2) goto early_exit;
            }
            break;
            
        case 3:
            x1 = x2 * x3;
            asm volatile("" : : : "xmm0", "xmm1");
            goto label_c;
            
        case 4:
        label_c:
            y2 = y3 + y4;
            asm volatile("" : : : "xmm2", "xmm3");
            if (mode > 100) {
                x4 = x5;
                asm volatile("" : : : "r10", "r11");
            }
            break;
            
        case 5:
            x3 = x4 ^ x5;
            asm volatile("" : : : "r12", "r13");
            /* Do-while loop */
            int j = 0;
            do {
                y5 += j;
                asm volatile("" : : : "r14");
                j++;
            } while (j < 3);
            break;
            
        case 6:
        early_exit:
            x2 = x1 * 3;
            asm volatile("" : : : "r15", "rbp");
            break;
    }
    
    return x1 + x2 + x3 + x4 + x5 + y1 + y2 + y3 + y4 + y5;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int selector = (argc > 2) ? atoi(argv[2]) : rand();
    
    long long total = 0;
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to exercise different paths */
        volatile int sel1 = selector + i;
        volatile int sel2 = selector - i;
        
        total += register_pressure_function(sel1);
        
        if (i % 3 == 0) {
            total += secondary_pressure_function(sel2);
        }
        
        /* Occasionally change selector pattern */
        if (i % 1000 == 0) {
            selector ^= i;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

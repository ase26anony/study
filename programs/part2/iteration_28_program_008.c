/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double dbl1 = 1.11, dbl2 = 2.22;
    long long ll1 = 100, ll2 = 200;
    
    /* Volatile to prevent optimization */
    volatile int v = selector;
    int result = 0;
    
    /* Complex switch with many cases */
    switch (v & 0xF) {  /* 16 cases */
        case 0:
            a = b + c;
            s1 = s2 - s3;
            f1 = f2 * 2.0f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case1_label;
        
        case1_label:
        case 1:
            c = d * e;
            ch1 = ch2 + 1;
            dbl1 = dbl2 / 2.0;
            asm volatile("" : : : "esi", "edi");
            if (a > 10) goto case3_label;
            else goto case2_label;
        
        case2_label:
        case 2:
            ll1 = ll2 << 2;
            f3 = f1 + f2;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case4_label;
        
        case 3:
        case3_label:
            b = a | c;
            s4 = s1 ^ s2;
            asm volatile("" : : : "r11", "r12");
            if (ch3 == 'C') goto case5_label;
            break;
        
        case 4:
        case4_label:
            d = e ^ a;
            ch4 = ch1 * 2;
            f2 = f3 - f1;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case6_label;
        
        case 5:
        case5_label:
            e = d & b;
            s3 = s4 | s1;
            dbl2 = dbl1 * 3.0;
            asm volatile("" : : : "xmm2", "xmm3");
            goto case7_label;
        
        case 6:
        case6_label:
            a = c + d + e;
            ll2 = ll1 >> 1;
            asm volatile("" : : : "xmm4", "xmm5");
            goto case8_label;
        
        case 7:
        case7_label:
            c = a * b * d;
            f1 = f2 / f3;
            asm volatile("" : : : "xmm6", "xmm7");
            goto case9_label;
        
        case 8:
        case8_label:
            b = (a << 2) | (c >> 1);
            ch2 = ch3 + ch4;
            asm volatile("" : : : "rax", "rbx");
            goto case10_label;
        
        case 9:
        case9_label:
            d = e * 3 - a;
            s2 = s1 + s3 - s4;
            asm volatile("" : : : "rcx", "rdx");
            goto case11_label;
        
        case 10:
        case10_label:
            e = (d ^ b) & a;
            f3 = f1 * f2 * 2.0f;
            asm volatile("" : : : "rsi", "rdi");
            goto case12_label;
        
        case 11:
        case11_label:
            a = b + c + d + e;
            dbl1 = dbl2 + 1.0;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case13_label;
        
        case 12:
        case12_label:
            c = (a & 0xFF) | (b << 8);
            ll1 = ll2 * 2;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            goto case14_label;
        
        case 13:
        case13_label:
            b = c * d - e;
            ch3 = ch1 + ch2;
            asm volatile("" : : : "xmm8", "xmm9", "xmm10");
            goto case15_label;
        
        case 14:
        case14_label:
            d = a / (b + 1);
            s4 = s2 * s3;
            asm volatile("" : : : "xmm11", "xmm12", "xmm13");
            break;
        
        case 15:
        case15_label:
            e = (a + b + c + d) & 0xFFFF;
            f2 = f1 + f3;
            dbl2 = dbl1 * dbl2;
            asm volatile("" : : : "xmm14", "xmm15");
            /* Fall through to default */
        
        default:
            /* Mix all variables */
            result = a + b + c + d + e;
            result += s1 + s2 + s3 + s4;
            result += ch1 + ch2 + ch3 + ch4;
            result += (int)f1 + (int)f2 + (int)f3;
            result += (int)dbl1 + (int)dbl2;
            result += (int)(ll1 & 0xFFFFFFFF) + (int)(ll2 & 0xFFFFFFFF);
            
            /* More register clobbering */
            asm volatile("" : : : 
                "eax", "ebx", "ecx", "edx",
                "esi", "edi",
                "r8", "r9", "r10", "r11",
                "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3",
                "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11",
                "xmm12", "xmm13", "xmm14", "xmm15");
    }
    
    /* Additional irreducible control flow with goto */
    if (result > 1000) {
        goto extra_label1;
    } else if (result > 500) {
        goto extra_label2;
    } else {
        goto extra_label3;
    }
    
extra_label1:
    result = result * 2 - 100;
    goto finish;
    
extra_label2:
    result = result / 2 + 50;
    goto finish;
    
extra_label3:
    result = result ^ 0x55AA;
    /* fall through */
    
finish:
    /* Final computation using all variables */
    result = (result * a + b * c - d * e) & 0xFFF;
    result ^= (s1 << 8) | s2;
    result += (ch1 << 16) | (ch2 << 8) | ch3;
    
    return result;
}

/* Another function to create more control flow complexity */
NOINLINE static int nested_control_flow(volatile int x) {
    int i, j, k = 0;
    
    /* Nested loops with conditionals */
    for (i = 0; i < (x & 0x7); i++) {
        volatile int limit = (x >> 3) & 0x7;
        for (j = 0; j < limit; j++) {
            if ((i + j) & 1) {
                k += i * j;
                goto loop_inner;
            } else {
                k -= i + j;
                if (k < 0) goto loop_outer;
            }
            
            k += 2;
            
loop_inner:
            asm volatile("" : : : "eax", "ebx");
            continue;
        }
        
loop_outer:
        asm volatile("" : : : "ecx", "edx");
    }
    
    /* Switch inside loop */
    while (k > 0) {
        volatile int sw = k & 0x3;
        switch (sw) {
            case 0: k = k >> 1; goto case_0;
            case 1: k = k * 3 + 1; goto case_1;
            case_0:
            case 2: k = k - 5; break;
            case_1:
            case 3: k = k + 2; goto end_switch;
            default: k = k ^ 0xFF;
        }
        
        asm volatile("" : : : "esi", "edi");
        
end_switch:
        if (k < 0) break;
    }
    
    return k;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total = 0;
    int i;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (i = 0; i < iterations; i++) {
        /* Vary selector to exercise different control paths */
        selector = rand() & 0xFF;
        
        /* Call the high register pressure function */
        int res1 = register_pressure_function(selector);
        
        /* Call nested control flow function */
        int res2 = nested_control_flow(selector);
        
        /* Mix results to prevent optimization */
        total += res1 * 3 + res2 * 7;
        
        /* Occasionally change selector pattern */
        if ((i & 0xFFF) == 0) {
            selector = (selector << 1) | (total & 1);
        }
    }
    
    printf("Total result: %lld\n", total);
    
    return (int)(total & 0x7FFFFFFF);
}

/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no inlining to preserve complex control flow */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile int control = selector; /* Prevent optimization */
    
    /* Complex irreducible control flow using goto and switch */
    switch (control % 12) {
        case 0:
            a = b + c;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case2_label;
        
        case 1:
            d = e * f;
            f1 = f2 + f3;
            goto case5_label;
        
        case2_label:
        case 2:
            g = h - i;
            s1 = s2 + s3;
            /* More register clobbering */
            asm volatile("" : : : "esi", "edi");
            goto case7_label;
        
        case 3:
            j = k / 2;
            ch1 = ch2 + 1;
            goto case9_label;
        
        case 4:
            l = m ^ n;
            f4 = f1 * 2.0f;
            goto case0_label;
        
        case5_label:
        case 5:
            o = p << 2;
            s4 = s1 - s2;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case11_label;
        
        case 6:
            a = b * c * d;
            f2 = f3 / f4;
            goto case3_label;
        
        case7_label:
        case 7:
            e = f | g;
            ch3 = ch4 - 1;
            asm volatile("" : : : "r11", "r12", "r13");
            goto case6_label;
        
        case 8:
            h = i & j;
            s3 = s4 * 2;
            goto case10_label;
        
        case 9:
        case9_label:
            k = l + m + n;
            f3 = f4 - f1;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case8_label;
        
        case 10:
        case10_label:
            m = n ^ o ^ p;
            ch4 = ch1 * 2;
            goto case4_label;
        
        case 11:
        case11_label:
            p = a + b + c + d;
            s2 = s3 / 2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            /* Fall through */
        
        case0_label:
        case3_label:
        default:
            /* Mix all variables to ensure they're used */
            a = b + c - d + e - f + g - h + i - j + k - l + m - n + o - p;
            f1 = f2 + f3 - f4;
            s1 = s2 + s3 - s4;
            ch1 = ch2 + ch3 - ch4;
            break;
    }
    
    /* More irreducible control flow with nested loops */
    for (int x = 0; x < 3; x++) {
        if (x % 2 == 0) {
            for (int y = 0; y < 2; y++) {
                a += b;
                if (y == 1) goto outer_loop;
            }
        } else {
            b += c;
            if (x == 1) goto skip_point;
        }
        c += d;
        continue;
        
    skip_point:
        d += e;
        if (a > 10) break;
        
    outer_loop:
        e += f;
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
    }
    
    /* Final computation using all variables */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    result += s1 + s2 + s3 + s4;
    result += ch1 + ch2 + ch3 + ch4;
    result += (int)(f1 + f2 + f3 + f4);
    
    return result;
}

/* Another complex function to create more flow graph edges */
__attribute__((noinline, noipa))
int secondary_pressure_function(volatile int seed) {
    int x1 = seed, x2 = seed * 2, x3 = seed * 3;
    int x4 = seed * 4, x5 = seed * 5, x6 = seed * 6;
    
    /* Unstructured control flow */
    if (seed % 3 == 0) {
        goto label_a;
    } else if (seed % 3 == 1) {
        goto label_b;
    } else {
        goto label_c;
    }
    
label_a:
    x1 = x2 + x3;
    asm volatile("" : : : "r14", "r15");
    if (x1 > 100) goto label_d;
    else goto label_c;
    
label_b:
    x4 = x5 - x6;
    asm volatile("" : : : "xmm6", "xmm7");
    goto label_e;
    
label_c:
    x2 = x3 * x4;
    goto label_f;
    
label_d:
    x5 = x6 / 2;
    goto label_b;
    
label_e:
    x3 = x4 ^ x5;
    goto label_c;
    
label_f:
    x6 = x1 & x2;
    return x1 + x2 + x3 + x4 + x5 + x6;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int base_selector = rand();
    long long total = 0;
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    for (int count = 0; count < iterations; count++) {
        /* Vary the selector to exercise different control flow paths */
        volatile int selector = base_selector + count;
        
        /* Call both pressure functions to create more register allocation complexity */
        int result1 = register_pressure_function(selector);
        int result2 = secondary_pressure_function(selector ^ 0x55AA);
        
        total += result1 + result2;
        
        /* Occasionally change control flow pattern */
        if (count % 1000 == 0) {
            base_selector = rand();
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

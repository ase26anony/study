/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and irreducible control flow */
NOINLINE int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'a', ch2 = 'b', ch3 = 'c';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases creating multiple basic blocks */
    switch (control % 12) {
        case 0:
            a = b + c;
            b = d * e;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case1_label;
        
        case 1:
        case1_label:
            c = f - g;
            d = h / (i ? i : 1);
            fl1 = fl2 + fl3;
            asm volatile("" : : : "esi", "edi");
            if (a > b) goto case3_label;
            else goto case5_label;
        
        case 2:
            e = j << 2;
            f = s1 * s2;
            ch1 = ch2 + 1;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case 3:
        case3_label:
            g = a | b;
            h = c & d;
            fl2 = fl1 * 2.0f;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            goto case7_label;
        
        case 4:
            i = e ^ f;
            j = g % (h ? h : 1);
            s3 = s1 + s2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case 5:
        case5_label:
            a = ch1 * ch2;
            b = s3 - s1;
            fl3 = fl2 / fl1;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            if (c < d) goto case9_label;
            break;
        
        case 6:
            c = a + b + c + d;
            d = e * f * g;
            asm volatile("" : : : "xmm6", "xmm7");
            goto case2_label;
        
        case 7:
        case7_label:
            e = (h << 3) | (i >> 1);
            f = j & 0xFF;
            ch3 = ch1 + ch2;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
            break;
        
        case 8:
            g = s1 | s2 | s3;
            h = ch1 * ch2 * ch3;
            fl1 = fl2 - fl3;
            asm volatile("" : : : "rsi", "rdi");
            goto case11_label;
        
        case 9:
        case9_label:
            i = a ^ b ^ c;
            j = d | e | f;
            s1 = s2 + s3;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
        
        case 10:
            a = g + h + i + j;
            b = s1 * s2 * s3;
            fl2 = fl3 * 3.14f;
            asm volatile("" : : : "xmm8", "xmm9", "xmm10");
            goto case0_label;
        
        case 11:
        case11_label:
            c = ch1 << 4;
            d = ch2 >> 2;
            fl3 = fl1 + fl2;
            asm volatile("" : : : "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
            break;
        
        default:
            a = b = c = 0;
            asm volatile("" : : : "memory");
    }
    
    /* Additional irreducible control flow with gotos */
    if (control % 3 == 0) {
        goto extra_label1;
    }
    
    extra_label2:
    result = a + b + c + d + e + f + g + h + i + j + 
             s1 + s2 + s3 + ch1 + ch2 + ch3 + (int)fl1 + (int)fl2 + (int)fl3;
    
    /* Prevent tail call optimization */
    asm volatile("" : : "r"(result) : "memory");
    return result;
    
    extra_label1:
    a = a * 2;
    b = b / 2;
    goto extra_label2;
}

/* Another function to create more complex call graph */
NOINLINE int secondary_pressure_function(int iter) {
    volatile int local_selector = iter;
    int sum = 0;
    
    for (int k = 0; k < 5; k++) {
        /* Nested loop with register pressure */
        int x = k * 10;
        int y = k * 20;
        int z = k * 30;
        
        asm volatile("" : : : "eax", "ebx");
        
        switch (local_selector % 7) {
            case 0: x = y + z; break;
            case 1: y = z - x; break;
            case 2: z = x * y; break;
            case 3: x = y | z; break;
            case 4: y = z & x; break;
            case 5: z = x ^ y; break;
            case 6: x = y % (z ? z : 1); break;
        }
        
        sum += x + y + z;
        local_selector++;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    long long total = 0;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int iter = 0; iter < iterations; iter++) {
        volatile int selector = rand();
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_pressure_function(iter);
        
        /* Mix results to prevent optimization */
        total += result1 + result2;
        
        /* Occasionally change control flow */
        if (iter % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Ensure result is used */
    volatile long long final_check = total;
    return (final_check > 0) ? 0 : 1;
}

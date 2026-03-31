/* test_mcf.c - Program to trigger MCF algorithm special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no inlining and no inter-procedural analysis */
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
    int result = 0;
    
    /* Start with goto to create irreducible region */
    if (control & 1) goto case_block_2;
    
switch_start:
    switch (control % 10) {
        case 0:
            /* Case 0 operations */
            a = b + c;
            b = c * d;
            c = d - e;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            if (a > 10) goto case_block_5;
            break;
            
        case 1:
            /* Different operations */
            d = e + f;
            e = f * g;
            f = g - h;
            asm volatile("" : : : "esi", "edi");
            result += d + e + f;
            goto case_block_3;
            
        case 2:
case_block_2:
            g = h + i;
            h = i * j;
            i = j - k;
            asm volatile("" : : : "r8", "r9", "r10");
            if (g < 50) goto switch_start;
            break;
            
        case 3:
case_block_3:
            j = k + l;
            k = l * m;
            l = m - n;
            asm volatile("" : : : "r11", "r12", "r13");
            result += j * k;
            goto case_block_7;
            
        case 4:
            m = n + o;
            n = o * p;
            o = p - a;
            asm volatile("" : : : "xmm0", "xmm1");
            f1 = f2 + f3;
            break;
            
        case 5:
case_block_5:
            p = a + b;
            s1 = s2 + s3;
            s2 = s3 - s4;
            asm volatile("" : : : "xmm2", "xmm3");
            ch1 = ch2 + 1;
            ch2 = ch3 - 1;
            if (p > 100) goto case_block_9;
            break;
            
        case 6:
            f2 = f3 * f4;
            f3 = f4 / 2.0f;
            f4 = f1 + 1.0f;
            asm volatile("" : : : "xmm4", "xmm5");
            result += (int)f2;
            goto case_block_8;
            
        case 7:
case_block_7:
            s3 = s4 * 2;
            s4 = s1 / 2;
            ch3 = ch4 + 2;
            ch4 = ch1 - 2;
            asm volatile("" : : : "r14", "r15");
            break;
            
        case 8:
case_block_8:
            a = s1 + s2;
            b = s3 - s4;
            c = ch1 * ch2;
            d = ch3 + ch4;
            asm volatile("" : : : "rax", "rbx");
            if (a + b > c + d) goto case_block_2;
            break;
            
        case 9:
case_block_9:
            e = f1 * 10;
            f = f2 * 20;
            g = f3 * 30;
            h = f4 * 40;
            asm volatile("" : : : "rcx", "rdx", "rsi", "rdi");
            result += e + f + g + h;
            /* Loop back to create complex CFG */
            if (selector % 3 == 0) goto case_block_3;
            break;
    }
    
    /* More operations mixing all variables */
    i = a + b + c + d;
    j = e + f + g + h;
    k = s1 + s2 + s3 + s4;
    l = ch1 + ch2 + ch3 + ch4;
    m = (int)(f1 + f2 + f3 + f4);
    
    /* Final computation using all variables */
    result += i + j + k + l + m + n + o + p;
    
    /* More register clobbering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another complex function to increase overall CFG complexity */
__attribute__((noinline, noipa))
int secondary_pressure_function(int base) {
    volatile int x = base;
    int arr[20];
    
    /* Unrolled loop with register pressure */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
        asm volatile("" : : : "eax", "ebx");
    }
    
    /* Complex conditional jumps */
    if (x % 2 == 0) {
        goto label_a;
    } else if (x % 3 == 0) {
        goto label_b;
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += arr[i];
        if (sum > 1000) goto label_c;
    }
    
    return sum;

label_a:
    asm volatile("" : : : "ecx", "edx");
    return arr[0] + arr[1];
    
label_b:
    asm volatile("" : : : "esi", "edi");
    return arr[10] * arr[11];
    
label_c:
    asm volatile("" : : : "r8", "r9", "r10");
    return sum / 2;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int seed = rand();
    long long total = 0;
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    for (int count = 0; count < iterations; count++) {
        /* Vary the selector to exercise different control flow paths */
        volatile int selector = seed + count;
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_pressure_function(selector % 100);
        
        /* Mix results to prevent optimization */
        total += result1 + result2;
        
        /* Modify seed to change control flow */
        if (count % 1000 == 0) {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

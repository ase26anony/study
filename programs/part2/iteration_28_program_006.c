/* test_mcf.c - Program to trigger MCF algorithm special block printing */
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
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    int result = 0;
    
    /* Complex switch with many cases */
    switch (selector % 12) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto label1;
        
        case 1:
            g = h - i;
            fl1 = fl2 + fl3;
            asm volatile("" : : : "esi", "edi");
            /* Jump to another case */
            if (ch1 == 'A') goto case3;
            break;
        
        case 2:
        label1:
            s1 = s2 * s3;
            ch2 = ch1 + 1;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case5;
        
        case 3:
        case3:
            j = a * b * c;
            fl2 = fl1 / 2.0f;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            break;
        
        case 4:
            d = e + f + g;
            ch3 = ch2 - 1;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto label2;
        
        case 5:
        case5:
            h = i * j;
            s3 = s1 + s2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            if (fl3 > 2.0f) goto case7;
            break;
        
        case 6:
            a = b * c * d;
            fl3 = fl1 * fl2;
            asm volatile("" : : : "xmm6", "xmm7");
            goto label3;
        
        case 7:
        case7:
            e = f - g - h;
            s2 = s1 * 2;
            asm volatile("" : : : "rax", "rbx", "rcx");
            break;
        
        case 8:
        label2:
            i = j + a + b;
            ch1 = ch3;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            goto case10;
        
        case 9:
            c = d * e;
            fl1 = fl3 - fl2;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
        
        case 10:
        case10:
            f = g + h + i;
            s1 = s3;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            goto label3;
        
        case 11:
        label3:
            j = a - b - c;
            fl2 = fl1 * 3.0f;
            asm volatile("" : : : "xmm8", "xmm9", "xmm10", "xmm11");
            break;
    }
    
    /* More irreducible control flow with gotos */
    if (selector & 1) {
        goto extra_label1;
    } else {
        goto extra_label2;
    }
    
extra_label1:
    a += b;
    goto merge_point;
    
extra_label2:
    c += d;
    goto merge_point;
    
merge_point:
    /* Use all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)fl1 + (int)fl2 + (int)fl3;
    
    /* Final register clobber */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another function to create more complex call graph */
NOINLINE int secondary_function(volatile int x) {
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) goto loop_label1;
        if (i % 3 == 1) goto loop_label2;
        
        sum += arr[i];
        continue;
        
    loop_label1:
        sum += arr[i] * 2;
        continue;
        
    loop_label2:
        sum += arr[i] * 3;
        continue;
    }
    
    asm volatile("" : : : "eax", "ebx", "ecx", "edx");
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int selector = rand();
    
    long long total = 0;
    
    /* Hot loop calling the high-pressure function */
    for (int i = 0; i < iterations; i++) {
        selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call both functions to increase complexity */
        total += register_pressure_function(selector);
        if (i % 7 == 0) {
            total += secondary_function(selector % 100);
        }
        
        /* Vary control flow */
        if (i % 13 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

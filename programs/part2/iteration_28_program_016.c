/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function to create high register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'a', ch2 = 'b', ch3 = 'c';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;  /* Create cross-block jumps */
        
        case 1:
            d = e * f;
            fl1 = fl2 + fl3;
            asm volatile("" : : : "esi", "edi");
            goto case_10;
        
        case 2:
            g = h - i;
            s1 = s2 * s3;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
        
        case 3:
            ch1 = ch2 + 1;
            j = a * b;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case_15;
        
        case 4:
            fl2 = fl3 * 2.0f;
            c = d / e;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            break;
        
        case 5:
        case_5:
            f = g ^ h;
            s2 = s1 + s3;
            asm volatile("" : : : "rax", "rbx");
            if (ch1 > 'm') goto case_negative;
            break;
        
        case 6:
            i = j << 2;
            fl3 = fl1 - fl2;
            asm volatile("" : : : "rcx", "rdx");
            goto case_0;
        
        case 7:
            a = b | c;
            ch2 = ch3 - 1;
            asm volatile("" : : : "xmm2", "xmm3", "xmm4");
            break;
        
        case 8:
            d = e & f;
            s3 = s1 ^ s2;
            asm volatile("" : : : "rsi", "rdi");
            goto case_12;
        
        case 9:
            g = h % i;
            fl1 = fl2 / fl3;
            asm volatile("" : : : "xmm5", "xmm6");
            break;
        
        case 10:
        case_10:
            j = a + b + c;
            ch3 = ch1 * 2;
            asm volatile("" : : : "r8", "r9");
            if (fl1 > 5.0f) goto case_5;
            break;
        
        case 11:
            b = c * d;
            s1 = s2 - s3;
            asm volatile("" : : : "xmm7", "xmm8", "xmm9");
            goto case_18;
        
        case 12:
        case_12:
            e = f / g;
            fl2 = fl3 * 3.0f;
            asm volatile("" : : : "r10", "r11", "r12");
            break;
        
        case 13:
            h = i ^ j;
            ch1 = ch2 + ch3;
            asm volatile("" : : : "xmm10", "xmm11");
            goto case_2;
        
        case 14:
            c = d << 1;
            s2 = s3 >> 1;
            asm volatile("" : : : "r13", "r14", "r15");
            break;
        
        case 15:
        case_15:
            f = g | h;
            fl3 = fl1 + fl2;
            asm volatile("" : : : "xmm12", "xmm13", "xmm14");
            if (s1 < 0) goto case_negative;
            break;
        
        case 16:
            i = j % a;
            ch2 = ch1 - '0';
            asm volatile("" : : : "rax", "rbx", "rcx");
            goto case_8;
        
        case 17:
            b = c & d;
            s3 = s1 | s2;
            asm volatile("" : : : "xmm15");
            break;
        
        case 18:
        case_18:
            e = f * g;
            fl1 = fl2 - fl3;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            if (ch3 < 'z') goto case_10;
            break;
        
        case 19:
            h = i + j;
            ch3 = ch1 / 2;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case_5;
        
        default:
            goto case_negative;
    }
    
    /* Compute result using all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)(fl1 + fl2 + fl3);
    
    return result;

case_negative:
    /* Alternative path with different operations */
    a = b - c;
    d = e / f;
    g = h * i;
    j = a ^ b;
    s1 = s2 + s3;
    ch1 = ch2 * ch3;
    fl1 = fl2 / fl3;
    
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", 
                               "esi", "edi", "r8", "r9");
    
    result = a + d + g + j + s1 + ch1 + (int)fl1;
    return result;
}

/* Another complex function to increase overall pressure */
NOINLINE static int secondary_pressure_function(int base) {
    volatile int x = base;
    int arr[20];
    
    /* Unrolled loop with register pressure */
    for (int k = 0; k < 20; k++) {
        arr[k] = k * x;
        asm volatile("" : : : "memory");
    }
    
    /* Complex conditional chain */
    if (x % 3 == 0) {
        for (int k = 0; k < 10; k++) {
            arr[k] += arr[19 - k];
            asm volatile("" : : : "xmm0", "xmm1");
        }
        goto merge_point;
    } else if (x % 3 == 1) {
        for (int k = 0; k < 15; k++) {
            arr[k] *= 2;
            asm volatile("" : : : "xmm2", "xmm3", "xmm4");
        }
        goto merge_point;
    } else {
        for (int k = 5; k < 20; k++) {
            arr[k] -= k;
            asm volatile("" : : : "xmm5", "xmm6", "xmm7");
        }
    }

merge_point:
    int sum = 0;
    for (int k = 0; k < 20; k++) {
        sum += arr[k];
        if (k % 4 == 0) {
            asm volatile("" : : : "rax", "rbx");
        }
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total = 0;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int count = 0; count < iterations; count++) {
        /* Vary selector to exercise different control paths */
        selector = rand() % 100;
        
        /* Call main pressure function */
        int res1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int res2 = secondary_pressure_function(selector + count);
        
        /* Accumulate results to prevent optimization */
        total += res1 + res2;
        
        /* Occasionally flush to prevent loop optimization */
        if (count % 10000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

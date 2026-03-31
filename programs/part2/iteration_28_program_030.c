/* test_mcf.c - Program to trigger MCF special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    
    /* Result variable that depends on all locals */
    int result = 0;
    
    /* Complex switch with irreducible control flow */
    switch (selector % 12) {
        case 0:
            a = b + c;
            s1 = s2 - s3;
            f1 = f2 * f3;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case1_label;
            
        case 1:
        case1_label:
            c = d * e;
            ch1 = ch2 + 1;
            d1 = d2 / 2.0;
            asm volatile("" : : : "esi", "edi");
            if (a > 10) goto case5_label;
            break;
            
        case 2:
            b = a ^ c;
            s3 = s1 | s2;
            l1 = l2 << 2;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case3_label;
            
        case 3:
        case3_label:
            d = e << 1;
            ch3 = ch1 ^ ch2;
            f3 = f1 + f2;
            asm volatile("" : : : "r11", "r12", "r13");
            break;
            
        case 4:
            e = a & b;
            s2 = s3 * 2;
            l2 = l3 >> 1;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case6_label;
            
        case 5:
        case5_label:
            a = c | d;
            ch2 = ch3 - 1;
            d2 = d1 * 3.0;
            asm volatile("" : : : "xmm3", "xmm4");
            if (b < 5) goto case8_label;
            break;
            
        case 6:
        case6_label:
            c = b ^ a;
            s1 = s2 + s3;
            f2 = f3 - f1;
            asm volatile("" : : : "xmm5", "xmm6", "xmm7");
            goto case7_label;
            
        case 7:
        case7_label:
            d = e >> 2;
            ch1 = ch2 * 2;
            l3 = l1 + l2;
            asm volatile("" : : : "rax", "rbx");
            break;
            
        case 8:
        case8_label:
            b = d & e;
            s3 = s1 - s2;
            f1 = f2 / f3;
            asm volatile("" : : : "rcx", "rdx");
            goto case9_label;
            
        case 9:
        case9_label:
            e = a + c;
            ch3 = ch1 | ch2;
            d1 = d2 + 1.0;
            asm volatile("" : : : "rsi", "rdi");
            if (c > 20) goto case11_label;
            break;
            
        case 10:
            a = b * c;
            s2 = s3 / 2;
            l1 = l2 ^ l3;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case0_label;
            
        case 11:
        case11_label:
            c = d - e;
            ch2 = ch3 & 0x7F;
            f3 = f1 * f2;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            break;
            
        default:
            a = b + d;
            s1 = s3 * 3;
            d2 = d1 - 0.5;
            asm volatile("" : : : "memory");
            break;
    }
    
    /* More irreducible control flow with gotos */
    if (selector % 3 == 0) {
        goto extra_calc1;
    } else if (selector % 3 == 1) {
        goto extra_calc2;
    } else {
        goto extra_calc3;
    }
    
extra_calc1:
    result = a + b + c;
    goto combine;
    
extra_calc2:
    result = d + e + s1;
    goto combine;
    
extra_calc3:
    result = ch1 + ch2 + ch3;
    /* Fall through */
    
combine:
    /* Combine all variables to prevent optimization */
    result += s2 + s3 + (int)f1 + (int)f2 + (int)f3;
    result += l1 + l2 + l3;
    result += (int)d1 + (int)d2;
    
    /* More register pressure */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", 
                               "esi", "edi", "memory");
    
    return result;
}

/* Another complex function to increase overall complexity */
NOINLINE static int secondary_pressure_function(int base) {
    volatile int x = base;
    int arr[20];
    
    /* Unrolled loop with register pressure */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
        asm volatile("" : "+r"(arr[i]) : : "memory");
    }
    
    /* Complex conditional chain */
    if (x % 2 == 0) {
        for (int i = 0; i < 10; i++) {
            arr[i] += arr[19 - i];
        }
    } else if (x % 3 == 0) {
        for (int i = 0; i < 10; i++) {
            arr[i] -= arr[19 - i];
        }
    } else {
        for (int i = 0; i < 10; i++) {
            arr[i] *= arr[19 - i];
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += arr[i];
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    int total = 0;
    
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
        /* Vary selector to hit different control paths */
        selector = rand() % 100;
        
        /* Call main pressure function */
        int res1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int res2 = secondary_pressure_function(selector + i);
        
        /* Combine results to create observable side effect */
        total += res1 + res2;
        
        /* Occasionally flush to prevent optimization */
        if (i % 10000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %d\n", total);
    
    return 0;
}

/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no inlining and no inter-procedural analysis */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 100, u2 = 200, u3 = 300;
    
    /* Use volatile to prevent optimization */
    volatile int v = selector;
    volatile int control = 0;
    
    /* Complex switch with many cases */
    switch (v % 12) {
        case 0:
            a = b + c;
            s1 = s2 - s3;
            f1 = f2 * 2.0f;
            /* Clobber registers (x86 version) */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case1_jump;
        
        case1_jump:
        case 1:
            d = e * a;
            ch1 = ch2 + 1;
            d1 = d2 / 2.0;
            control = 1;
            if (control) goto case3_jump;
            break;
            
        case 2:
            u1 = u2 ^ u3;
            s4 = s1 | s2;
            f3 = f1 + f2;
            asm volatile("" : : : "esi", "edi");
            break;
            
        case3_jump:
        case 3:
            b = c << 2;
            ch3 = ch4 - 1;
            f2 = f3 * 3.14f;
            /* Another register clobber */
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case5_jump;
            
        case 4:
            e = d >> 1;
            s2 = s3 * 2;
            d2 = d1 + 1.0;
            if (e > 10) goto case0_jump;
            break;
            
        case5_jump:
        case 5:
            c = a & b;
            u2 = u1 | u3;
            f1 = f2 - f3;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
            
        case0_jump:
        case 6:
            a = b | c;
            ch4 = ch1 ^ ch2;
            d1 = d2 * 1.5;
            goto case8_jump;
            
        case 7:
            d = e ^ a;
            s3 = s4 + s1;
            f3 = f1 / 2.0f;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            break;
            
        case8_jump:
        case 8:
            b = c + d;
            u3 = u1 & u2;
            f2 = f3 + 4.0f;
            if (b < 50) goto case10_jump;
            break;
            
        case 9:
            e = a - d;
            ch2 = ch3 * 2;
            d2 = d1 - 0.5;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            break;
            
        case10_jump:
        case 10:
            c = d * e;
            s1 = s2 / 2;
            f1 = f2 * f3;
            goto case11_jump;
            
        case11_jump:
        case 11:
            a = b ^ d;
            u1 = u2 + u3;
            d1 = d2 / 3.0;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
            break;
    }
    
    /* Irreducible control flow with gotos */
    if (control) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    a += b;
    goto label3;
    
label2:
    c -= d;
    goto label4;
    
label3:
    e *= 2;
    goto label5;
    
label4:
    s1 = s2 + s3;
    goto label6;
    
label5:
    ch1 = ch2 - 1;
    goto label7;
    
label6:
    f1 = f2 + 1.0f;
    goto label8;
    
label7:
    d1 = d2 * 2.0;
    goto label9;
    
label8:
    u1 = u2 ^ 0xFF;
    goto label10;
    
label9:
    s4 = s1 | 0x0F;
    /* Final register clobber */
    asm volatile("" : : : "memory");
    
label10:
    /* Combine all variables to prevent elimination */
    int result = a + b + c + d + e;
    result += s1 + s2 + s3 + s4;
    result += ch1 + ch2 + ch3 + ch4;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)d1 + (int)d2;
    result += u1 + u2 + u3;
    
    return result;
}

/* Another complex function to increase graph complexity */
__attribute__((noinline, noipa))
int secondary_pressure_function(int base) {
    volatile int x = base;
    int arr[20];
    
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
    }
    
    /* Nested loops with register pressure */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 5; k++) {
                sum += arr[i] + arr[j] - arr[k];
                asm volatile("" : : : "eax", "ebx");
            }
        }
    }
    
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
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call main pressure function */
        int res1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int res2 = secondary_pressure_function(selector % 100);
        
        /* Mix results to create data dependencies */
        total += res1 + res2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            selector = rand();
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

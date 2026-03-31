/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function with high register pressure and irreducible control flow */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 1000, u2 = 2000;
    
    /* Use goto to create irreducible control flow */
    if (selector < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (selector % 11) {
        case 0:
            a = b + c;
            d = e * 2;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            f1 = f2 + f3;
            goto case_common;
            
        case 1:
            b = a - c;
            s1 = s2 + s3;
            asm volatile("" : : : "esi", "edi");
            ch1 = ch2 ^ ch3;
            goto case_common;
            
        case 2:
            c = d * e;
            l1 = l2 - l3;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            d1 = d2 * 2.0;
            goto case_common;
            
        case 3:
            d = a / (b ? b : 1);
            u1 = u2 << 2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            f2 = f3 * f1;
            goto case_alternate;
            
        case 4:
            e = c ^ d;
            s2 = s1 | s3;
            asm volatile("" : : : "rax", "rbx", "rcx");
            ch2 = ch1 + 1;
            goto case_alternate;
            
        case 5:
            a = b * c * d;
            l2 = l1 + l3;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            d2 = d1 / 2.0;
            goto case_alternate;
            
        case 6:
            b = a | c;
            u2 = u1 >> 1;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            f3 = f1 - f2;
            goto case_negative;
            
        case 7:
            c = d & e;
            s3 = s1 ^ s2;
            asm volatile("" : : : "ymm0", "ymm1");
            ch3 = ch2 - 1;
            goto case_negative;
            
        case 8:
            d = a + b + c + e;
            l3 = l1 * l2;
            asm volatile("" : : : "zmm0", "zmm1");
            f1 = f2 * f3 + 1.0f;
            goto case_negative;
            
        case 9:
            e = (a << 2) | (b >> 3);
            u1 = u2 * 3;
            asm volatile("" : : : "mm0", "mm1", "mm2");
            d1 = d2 + 1.0;
            goto case_common;
            
        case 10:
            a = b ^ c ^ d ^ e;
            s1 = s2 * s3;
            asm volatile("" : : : "st", "st(1)", "st(2)");
            ch1 = ch3;
            goto case_common;
            
        default:
            a = 0;
            goto case_common;
    }

case_common:
    /* More arithmetic to increase register pressure */
    a = a + b - c * d / (e ? e : 1);
    s1 = s1 + s2 - s3;
    ch1 = ch1 + ch2 - ch3;
    f1 = f1 + f2 - f3;
    l1 = l1 + l2 - l3;
    d1 = d1 + d2;
    u1 = u1 + u2;
    
    /* Another goto to create loop in control flow */
    if (a > 100) goto case_alternate;
    
    return a + s1 + ch1 + (int)f1 + (int)l1 + (int)d1 + u1;

case_alternate:
    /* Alternate computation path */
    b = b * 2 + c * 3 - d * 4;
    s2 = s2 / 2 + s3 * 3;
    ch2 = ch2 * 2 - ch1;
    f2 = f2 * 2.0f + f1;
    l2 = l2 + l1 * 2 - l3;
    d2 = d2 * 1.5;
    u2 = u2 - u1;
    
    /* Jump back to create irreducible region */
    if (b < 50) goto case_common;
    
    return b + s2 + ch2 + (int)f2 + (int)l2 + (int)d2 + u2;

case_negative:
    /* Third computation path */
    c = a * b - d * e;
    s3 = s1 * s2 / (s3 ? s3 : 1);
    ch3 = ch1 ^ ch2;
    f3 = f1 * f2 - f3;
    l3 = l1 ^ l2;
    d1 = d1 - d2;
    u1 = u1 ^ u2;
    
    /* Complex conditional jumps */
    if (c > 0) {
        if (s3 < 100) goto case_common;
        else goto case_alternate;
    } else {
        if (ch3 > 64) goto case_alternate;
    }
    
    return c + s3 + ch3 + (int)f3 + (int)l3 + (int)d1 + u1;
}

/* Another complex function to increase overall pressure */
__attribute__((noinline))
int secondary_pressure(volatile int x) {
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += arr[i];
        /* More register clobbering */
        asm volatile("" : : : "memory");
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
    
    /* Hot loop calling the pressure function */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call main pressure function */
        int result = register_pressure_function(selector % 20 - 10);
        total += result;
        
        /* Call secondary function periodically */
        if (i % 7 == 0) {
            total += secondary_pressure(selector % 100);
        }
        
        /* More variation in control flow */
        if (i % 13 == 0) {
            selector = -selector;
        }
    }
    
    printf("Total: %lld\n", total);
    return 0;
}

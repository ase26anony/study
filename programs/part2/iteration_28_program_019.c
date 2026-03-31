/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.11, d2 = 2.22;
    unsigned int u1 = 100, u2 = 200, u3 = 300;
    
    /* Create irreducible control flow with goto */
    if (selector < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (selector & 0xF) {
        case 0:
            a = b + c;
            s1 = s2 - s3;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_mix;
            
        case 1:
            b = c * d;
            f1 = f2 + f3;
            asm volatile("" : : : "esi", "edi");
            break;
            
        case 2:
            c = d / (e ? e : 1);
            d1 = d2 * 2.0;
            goto case_4;
            
        case 3:
            d = e - a;
            u1 = u2 ^ u3;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
            
        case 4:
        case_4:
            e = a | b;
            ch1 = ch2 + 1;
            asm volatile("" : : : "r11", "r12");
            goto case_6;
            
        case 5:
            s2 = s3 * s4;
            f2 = f1 - f3;
            break;
            
        case 6:
        case_6:
            s3 = s4 / 2;
            d2 = d1 + 1.0;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case_8;
            
        case 7:
            s4 = s1 + s2;
            ch2 = ch3 - 1;
            break;
            
        case 8:
        case_8:
            ch3 = ch4 * 2;
            u2 = u3 & u1;
            asm volatile("" : : : "xmm3", "xmm4");
            goto case_10;
            
        case 9:
            ch4 = ch1 / 2;
            f3 = f1 * f2;
            break;
            
        case 10:
        case_10:
            f1 = f2 / f3;
            a = b ^ c;
            asm volatile("" : : : "xmm5", "xmm6", "xmm7");
            goto case_12;
            
        case 11:
            f2 = f3 + f1;
            d = e << 2;
            break;
            
        case 12:
        case_12:
            f3 = f1 - f2;
            u3 = u1 | u2;
            asm volatile("" : : : "r13", "r14", "r15");
            goto case_end;
            
        case 13:
            d1 = d2 * 3.14;
            c = d & e;
            break;
            
        case 14:
            d2 = d1 / 2.0;
            b = c ^ d;
            asm volatile("" : : : "rax", "rbx", "rcx");
            goto final_mix;
            
        case 15:
            u1 = u2 + u3;
            a = b * c;
            break;
            
        default:
            goto case_negative;
    }
    
    goto after_switch;
    
case_negative:
    a = -b;
    s1 = -s2;
    asm volatile("" : : : "rdx", "rsi", "rdi");
    
case_mix:
    ch1 = ~ch2;
    f1 = -f2;
    asm volatile("" : : : "r8", "r9", "r10", "r11");
    
final_mix:
    u1 = ~u2;
    d1 = -d2;
    
after_switch:
    /* More irreducible control flow with nested loops */
    for (int i = 0; i < 3; i++) {
        if (i == 1) goto loop_middle;
        for (int j = 0; j < 2; j++) {
            if (j == 0) goto inner_label;
            a += j;
        inner_label:
            b += i;
        }
        if (i == 2) goto loop_end;
    loop_middle:
        c += i;
    }
loop_end:

    /* Use all variables to prevent optimization */
    int result = a + b + c + d + e;
    result += s1 + s2 + s3 + s4;
    result += ch1 + ch2 + ch3 + ch4;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)d1 + (int)d2;
    result += u1 + u2 + u3;
    
    /* Final register clobber */
    asm volatile("" : : : "memory", "cc", 
#ifdef __x86_64__
                 "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
                 "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
                 "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
#elif defined(__arm__)
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12"
#elif defined(__aarch64__)
                 "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                 "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                 "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
                 "x24", "x25", "x26", "x27", "x28", "x29", "x30"
#endif
                );
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector;
    long long total = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Use random or time-based selector */
    if (argc > 2) {
        selector = atoi(argv[2]);
    } else {
        srand(time(NULL));
        selector = rand();
    }
    
    printf("Running MCF stress test with %d iterations, selector=%d\n", 
           iterations, selector);
    
    /* Hot loop calling the high-pressure function */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector slightly each iteration */
        volatile int local_selector = selector + (i & 0xF);
        int result = register_pressure_function(local_selector);
        total += result;
        
        /* Occasionally change control flow */
        if ((i & 0xFF) == 0) {
            selector ^= 0x5555;
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Prevent optimization of total */
    asm volatile("" : "+r"(total));
    
    return (int)(total & 0x7FFFFFFF);
}

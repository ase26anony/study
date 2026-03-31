/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Complex function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long l1 = 100, l2 = 200, l3 = 300;
    double d1 = 1.11, d2 = 2.22;
    
    /* Use volatile to prevent optimization */
    volatile int v = selector;
    int result = 0;
    
    /* Complex switch with many cases - creates many basic blocks */
    switch (v & 0xF) {
        case 0:
            a = b + c;
            s1 = s2 - s3;
            f1 = f2 * f3;
            /* Irreducible control flow with goto */
            goto case_5_jump;
            
        case 1:
            d = e * a;
            ch1 = ch2 + 1;
            d1 = d2 / 2.0;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            break;
            
        case 2:
            b = c - d;
            s2 = s3 + s1;
            f2 = f3 - f1;
            goto case_8_jump;
            
        case 3:
            c = d / (a ? a : 1);
            ch2 = ch3 - 1;
            l1 = l2 * l3;
            asm volatile("" : : : "esi", "edi");
            break;
            
        case 4:
            e = a * b * c;
            s3 = s1 | s2;
            f3 = f1 + f2;
            /* Another goto creating complex flow */
            goto case_1_jump;
            
        case_5_jump:
        case 5:
            a = b ^ c;
            ch3 = ch1 & ch2;
            d2 = d1 * 3.0;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
            
        case 6:
            d = e << 2;
            s1 = s2 >> 1;
            l2 = l1 + l3;
            goto case_3_jump;
            
        case 7:
            b = c | d;
            f1 = f2 / f3;
            ch1 = ch2 ^ ch3;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
            
        case_8_jump:
        case 8:
            c = a & b;
            s2 = s3 * 2;
            d1 = d2 - 1.0;
            break;
            
        case_9:
            e = d % (c ? c : 1);
            f3 = f1 * f2;
            l3 = l1 - l2;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            goto case_7_jump;
            
        case_1_jump:
        case 10:
            a = b + d;
            ch2 = ch1 * 2;
            f2 = f3 / f1;
            break;
            
        case_3_jump:
        case 11:
            d = c - e;
            s3 = s1 + s2;
            l1 = l2 | l3;
            asm volatile("" : : : "mm0", "mm1");
            break;
            
        case_7_jump:
        case 12:
            b = a * c;
            f1 = f3 - f2;
            ch3 = ch1 + ch2;
            break;
            
        case 13:
            c = d ^ e;
            s1 = s3 & s2;
            d2 = d1 / 2.0;
            goto case_0_jump;
            
        case 14:
            e = b << c;
            f3 = f1 + f2;
            l2 = l3 * l1;
            asm volatile("" : : : "st", "st(1)", "st(2)");
            break;
            
        case_0_jump:
        case 15:
            a = c | d;
            s2 = s1 >> 2;
            f2 = f3 * f1;
            break;
            
        default:
            /* More arithmetic to use all variables */
            a = b + c + d + e;
            s1 = s2 + s3;
            ch1 = ch2 + ch3;
            f1 = f2 + f3;
            l1 = l2 + l3;
            d1 = d2;
            break;
    }
    
    /* Use all variables in final computation to prevent elimination */
    result = a + b + c + d + e;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)l1 + (int)l2 + (int)l3;
    result += (int)d1 + (int)d2;
    
    /* Final register clobber */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another complex function to increase overall pressure */
NOINLINE static int secondary_pressure_function(int base) {
    int x1 = base, x2 = base + 1, x3 = base + 2;
    int y1, y2, y3;
    volatile int dir = base % 4;
    
    /* Complex loop with goto */
    int i = 0;
loop_start:
    if (i >= 10) goto loop_end;
    
    switch (dir) {
        case 0:
            x1 = x2 * x3;
            y1 = x1 >> 2;
            asm volatile("" : : : "ebx", "ecx");
            goto update;
        case 1:
            x2 = x1 - x3;
            y2 = x2 & 0xFF;
            goto update;
        case 2:
            x3 = x1 | x2;
            y3 = x3 ^ 0x55;
            asm volatile("" : : : "edx");
            goto update;
        case 3:
            x1 = x2 + x3;
            y1 = y2 + y3;
            break;
    }
    
update:
    dir = (dir + 1) % 4;
    i++;
    
    /* Unconditional jump back - creates irreducible region */
    if (i % 2 == 0) {
        goto loop_start;
    } else {
        i++;
        goto loop_start;
    }
    
loop_end:
    return x1 + x2 + x3 + y1 + y2 + y3;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = rand() % 32;
        
        /* Call main pressure function */
        int res1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int res2 = secondary_pressure_function(res1 % 100);
        
        /* Accumulate results to prevent elimination */
        total += res1 + res2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total: %lld\n", total);
    
    /* One more call with extreme values */
    selector = 0xFFFFFFFF;
    total += register_pressure_function(selector);
    total += secondary_pressure_function(selector % 100);
    
    printf("Final total: %lld\n", total);
    
    return 0;
}

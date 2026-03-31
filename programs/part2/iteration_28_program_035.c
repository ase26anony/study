/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Complex function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    double db1 = 4.4, db2 = 5.5;
    long long ll1 = 100, ll2 = 200;
    
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases - creates many basic blocks */
    switch (control % 12) {
        case 0:
            a = b + c;
            d = e * f;
            /* Irreducible control flow with goto */
            goto case2_label;
            
        case 1:
            g = h - i;
            j = a * b;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            break;
            
        case 2:
        case2_label:
            s1 = s2 + s3;
            ch1 = ch2 + 1;
            fl1 = fl2 * 2.0f;
            /* Another goto creating cross-block jumps */
            goto case4_label;
            
        case 3:
            db1 = db2 / 2.0;
            ll1 = ll2 << 1;
            asm volatile("" : : : "esi", "edi");
            break;
            
        case 4:
        case4_label:
            a = (b * c) / d;
            e = f ^ g;
            /* Jump to different case block */
            goto case7_label;
            
        case 5:
            h = i | j;
            s2 = s3 - s1;
            fl2 = fl3 + fl1;
            break;
            
        case 6:
            ch2 = ch3 * 2;
            db2 = db1 * 3.0;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case9_label;
            
        case 7:
        case7_label:
            ll2 = ll1 >> 1;
            a = b + c + d;
            /* Complex expression chain */
            result = a * b - c + d / e;
            goto case11_label;
            
        case 8:
            f = g * h + i;
            j = a & b;
            ch3 = ch1 + ch2;
            break;
            
        case 9:
        case9_label:
            s3 = s1 * s2;
            fl3 = fl1 / fl2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case0_fallback;
            
        case 10:
            db1 = db2 + db1;
            ll1 = ll2 - ll1;
            /* Nested computation */
            for (int k = 0; k < 3; k++) {
                a += k;
                b *= (k + 1);
            }
            break;
            
        case 11:
        case11_label:
            result = a + b + c + d + e + f + g + h + i + j;
            result += s1 + s2 + s3;
            result += ch1 + ch2 + ch3;
            result += (int)(fl1 + fl2 + fl3);
            result += (int)(db1 + db2);
            result += (int)(ll1 + ll2);
            break;
            
        default:
        case0_fallback:
            /* Default case with more operations */
            a = b * c * d;
            e = f / g + h;
            asm volatile("" : : : "memory");
            result = -1;
    }
    
    /* More irreducible control flow with labels and gotos */
    if (result > 1000) {
        goto large_result;
    } else if (result < 0) {
        goto negative_result;
    } else {
        goto normal_result;
    }
    
large_result:
    result >>= 2;
    goto final_compute;
    
negative_result:
    result = -result;
    goto final_compute;
    
normal_result:
    result *= 2;
    /* fall through */
    
final_compute:
    /* Final computation using all variables */
    result += a + b + c + d + e;
    result += f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)(fl1 * 10);
    result += (int)(db1 * 5);
    result += (int)ll1;
    
    /* More register clobbering */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                                 "rsi", "rdi", "r8", "r9");
    
    return result;
}

/* Another complex function to increase overall complexity */
NOINLINE static int secondary_pressure_function(int base) {
    int x1 = base, x2 = base + 1, x3 = base + 2;
    int x4 = base + 3, x5 = base + 4;
    volatile int mod = base % 7;
    
    /* Small switch creating additional control flow */
    switch (mod) {
        case 0: x1 = x2 * x3; break;
        case 1: x2 = x3 + x4; break;
        case 2: x3 = x4 - x5; break;
        case 3: x4 = x5 / 2; break;
        case 4: x5 = x1 | x2; break;
        case 5: x1 = x2 & x3; break;
        case 6: x2 = x3 ^ x4; break;
    }
    
    /* Loop with variable bounds */
    int sum = 0;
    for (int i = 0; i < (mod + 2); i++) {
        sum += x1 + x2 - x3 * x4 + x5;
        asm volatile("" : : : "r10", "r11", "r12", "r13");
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int total = 0;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int count = 0; count < iterations; count++) {
        volatile int selector = rand();
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_pressure_function(selector % 100);
        
        /* Mix results to prevent optimization */
        total += result1 - result2;
        
        /* Occasionally call with special values */
        if (count % 1000 == 0) {
            total += register_pressure_function(0);
            total += register_pressure_function(11);
        }
    }
    
    printf("Final total: %d\n", total);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(total) : "memory");
    
    return total != 0 ? 0 : 1;
}

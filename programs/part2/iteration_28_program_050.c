/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with extreme register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto negative_case;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            /* Clobber registers with inline asm */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            d = e * f;
            fl1 = fl2 + fl3;
            goto case_10;
        
        case 2:
            g = h - i;
            ch1 = ch2 + 1;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 3:
            s1 = s2 * s3;
            result = a + d;
            goto negative_case;
        
        case 4:
            fl2 = fl1 * 2.0f;
            j = a + b + c;
            asm volatile("" : : : "r8", "r9", "r10");
            break;
        
        case_5:
        case 5:
            c = d * e;
            s2 = s1 + s3;
            goto case_15;
        
        case 6:
            h = i / 2;
            fl3 = fl1 - fl2;
            asm volatile("" : : : "r11", "r12", "r13");
            break;
        
        case 7:
            a = b * c * d;
            ch2 = ch3 - 1;
            goto case_0;
        
        case 8:
            e = f + g + h;
            s3 = s1 * s2;
            break;
        
        case 9:
            fl1 = fl2 / fl3;
            i = j - a;
            asm volatile("" : : : "r14", "r15");
            goto case_20;
        
        case_10:
        case 10:
            b = c + d + e;
            ch3 = ch1 + ch2;
            break;
        
        case 11:
            f = g * h;
            fl2 = fl3 * 2.0f;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case_5;
        
        case 12:
            s1 = s2 - s3;
            result = e + f + g;
            break;
        
        case 13:
            a = b / 2;
            fl3 = fl1 + fl2;
            goto negative_case;
        
        case 14:
            c = d * e * f;
            ch1 = ch2 * 2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            break;
        
        case_15:
        case 15:
            g = h + i + j;
            s2 = s3 * 2;
            goto case_10;
        
        case 16:
            fl1 = a + b + c;
            d = e - f;
            break;
        
        case 17:
            h = i * j;
            ch2 = ch3 / 2;
            asm volatile("" : : : "xmm6", "xmm7");
            goto case_0;
        
        case 18:
            s3 = s1 + s2;
            result = h + i + j;
            break;
        
        case 19:
            fl2 = fl3 * 3.0f;
            j = a * b * c;
            goto case_20;
        
        case_0:
        default:
            a = b + c + d;
            fl3 = fl1 * fl2;
            break;
    }
    
    /* More irreducible control flow */
    if (result > 100) {
        goto final_calc;
    }
    
negative_case:
    /* Alternate path with different computations */
    a = b - c;
    d = e * f;
    g = h / 2;
    fl1 = fl2 - fl3;
    s1 = s2 + s3;
    ch1 = ch2 - ch3;
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
    
    if (control % 3 == 0) {
        goto case_15;
    }
    
case_20:
    /* Another basic block */
    i = j * 2;
    fl2 = fl1 * 3.14f;
    s2 = s3 - s1;
    
final_calc:
    /* Combine all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)(fl1 + fl2 + fl3);
    
    /* Final register clobbering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another complex function to increase overall CFG complexity */
NOINLINE static int secondary_pressure_function(int base) {
    volatile int x = base;
    int arr[20];
    
    /* Unrolled loop creating many basic blocks */
    arr[0] = x; if (x > 0) goto lbl1;
    arr[1] = x * 2; goto lbl2;
lbl1:
    arr[2] = x / 2; if (x % 2 == 0) goto lbl3;
    arr[3] = x + 1; goto lbl4;
lbl2:
    arr[4] = x - 1; if (x < 100) goto lbl5;
    arr[5] = x * x; goto lbl6;
lbl3:
    arr[6] = x % 3; asm volatile("" : : : "r8", "r9");
    goto lbl7;
lbl4:
    arr[7] = x << 2; if (x & 1) goto lbl8;
    arr[8] = x >> 1; goto lbl9;
lbl5:
    arr[9] = x | 0xFF; asm volatile("" : : : "r10", "r11");
    goto lbl10;
lbl6:
    arr[10] = x & 0x0F; if (x == 0) goto lbl1;
    arr[11] = ~x; goto lbl3;
lbl7:
    arr[12] = x ^ 0xAA; asm volatile("" : : : "r12", "r13");
    goto lbl4;
lbl8:
    arr[13] = x + 100; if (x > 50) goto lbl5;
    arr[14] = x - 50; goto lbl6;
lbl9:
    arr[15] = x * 3; asm volatile("" : : : "r14", "r15");
    goto lbl7;
lbl10:
    arr[16] = x / 3; if (x % 5 == 0) goto lbl8;
    arr[17] = x % 7; goto lbl9;
    
    /* Never reached, but creates more edges */
    arr[18] = 0;
    arr[19] = 1;
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int iterations = 100000;
    volatile int total = 0;
    
    /* Use command line argument for iteration count */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high register pressure functions */
    for (int i = 0; i < iterations; i++) {
        volatile int selector = rand() % 100;
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_pressure_function(selector);
        
        /* Mix results to create data dependencies */
        total += result1 + result2;
        
        /* Occasionally call with special values */
        if (i % 1000 == 0) {
            total += register_pressure_function(-1);
            total += register_pressure_function(0);
            total += register_pressure_function(100);
        }
    }
    
    printf("Total result: %d\n", total);
    
    /* Additional test with different optimization barriers */
    {
        volatile int special_test = 999;
        asm volatile("" : "+r" (special_test));
        int final = register_pressure_function(special_test);
        printf("Final test result: %d\n", final);
    }
    
    return total > 0 ? 0 : 1;
}

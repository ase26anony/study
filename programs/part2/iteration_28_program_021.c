/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function with extreme register pressure and irreducible control flow */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    long long ll1 = 100, ll2 = 200;
    double dbl1 = 1.23, dbl2 = 4.56;
    
    /* Volatile to prevent optimization */
    volatile int control = selector;
    
    /* Complex irreducible control flow with goto */
    if (control < 0) goto negative_case;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 20) {
        case 0:
            a = b + c;
            s1 = s2 - s3;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            b = c * d;
            f1 = f2 + f3;
            ch1 = ch2 + 1;
            goto case_10;
        
        case 2:
            c = d / (e ? e : 1);
            ll1 = ll2 >> 1;
            asm volatile("" : : : "esi", "edi");
            break;
        
        case 3:
            d = e ^ a;
            dbl1 = dbl2 * 2.0;
            goto case_15;
        
        case 4:
            e = a | b;
            s4 = s1 + s2;
            break;
        
        case_5:
        case 5:
            s1 = s3 * s2;
            ch3 = ch4 - 1;
            asm volatile("" : : : "r8", "r9", "r10");
            if (a > b) goto case_10;
            break;
        
        case 6:
            s2 = s4 / (s1 ? s1 : 1);
            f2 = f3 - f1;
            goto negative_case;
        
        case 7:
            s3 = s1 & s2;
            ll2 = ll1 << 2;
            break;
        
        case 8:
            s4 = s2 | s3;
            ch2 = ch1 * 2;
            asm volatile("" : : : "r11", "r12", "r13");
            goto case_0;
        
        case_10:
        case 9:
            ch1 = ch3 + ch4;
            dbl2 = dbl1 / 2.0;
            if (c < d) goto case_5;
            break;
        
        case 10:
            ch2 = ch4 - ch1;
            f3 = f1 * f2;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case 11:
            ch3 = ch1 ^ ch2;
            a = b * c + d;
            goto case_20;
        
        case 12:
            ch4 = ch2 | ch3;
            b = c - d * e;
            break;
        
        case 13:
            f1 = f2 / (f3 ? f3 : 1.0f);
            s1 = s2 + s3 - s4;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            goto case_1;
        
        case 14:
            f2 = f3 * f1;
            ll1 = ll2 + 100;
            break;
        
        case_15:
        case 15:
            f3 = f1 + f2;
            ch1 = ch2 * ch3;
            if (ll1 > ll2) goto case_10;
            break;
        
        case 16:
            ll1 = ll2 * 2;
            dbl1 = dbl2 + 1.0;
            asm volatile("" : : : "r14", "r15");
            goto negative_case;
        
        case 17:
            ll2 = ll1 / 2;
            a = b ^ c ^ d;
            break;
        
        case 18:
            dbl1 = dbl2 * 3.14;
            s1 = s2 & s3 & s4;
            goto case_5;
        
        case_20:
        case 19:
            dbl2 = dbl1 - 0.5;
            ch4 = ch1 + ch2 + ch3;
            asm volatile("" : : : "xmm6", "xmm7", "xmm8", "xmm9");
            break;
        
        default:
            goto negative_case;
    }
    
    /* Another irreducible region */
    if (a > 100) {
        goto final_calc;
    } else if (b > 50) {
        goto mid_calc;
    }
    
mid_calc:
    c = d * e;
    if (s1 < 0) goto final_calc;
    
negative_case:
    d = e + a;
    s2 = s3 - s4;
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
    
final_calc:
    /* Use all variables to prevent elimination */
    int result = a + b + c + d + e;
    result += s1 + s2 + s3 + s4;
    result += ch1 + ch2 + ch3 + ch4;
    result += (int)f1 + (int)f2 + (int)f3;
    result += (int)ll1 + (int)ll2;
    result += (int)dbl1 + (int)dbl2;
    
    return result;
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
        sum += arr[i % 10] * arr[(i + 5) % 10];
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
    volatile int base_selector = (argc > 2) ? atoi(argv[2]) : rand();
    
    long long total = 0;
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        volatile int selector = base_selector + i;
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_pressure(selector % 100);
        
        /* Mix results to create data dependencies */
        total += result1 - result2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            base_selector = (base_selector * 1103515245 + 12345) & 0x7fffffff;
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional loop with different pattern */
    total = 0;
    for (int i = 0; i < iterations / 10; i++) {
        for (int j = 0; j < 10; j++) {
            volatile int sel = (i * j + base_selector) % 100;
            total += register_pressure_function(sel);
            
            /* Nested control flow */
            if (j % 3 == 0) {
                total += secondary_pressure(sel * 2);
            } else if (j % 3 == 1) {
                total -= secondary_pressure(sel / 2);
            }
        }
    }
    
    printf("Final total: %lld\n", total);
    return 0;
}

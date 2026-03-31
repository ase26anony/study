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
    double db1 = 4.4, db2 = 5.5;
    volatile int control = selector;
    int result = 0;
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto case_negative;
    
    /* Large switch with many cases */
    switch (control % 20) {
        case 0:
            a = b + c;
            b = c * d;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            c = d - e;
            d = e / (a ? a : 1);
            fl1 = fl2 + fl3;
            goto case_10;
        
        case 2:
            e = f ^ g;
            f = g | h;
            s1 = s2 + s3;
            goto case_15;
        
        case 3:
            g = h & i;
            h = i << 2;
            ch1 = ch2 + 1;
            break;
        
        case 4:
            i = j * a;
            j = b % 7;
            db1 = db2 * 2.0;
            goto case_0;
        
        case_5:
        case 5:
            a = a + b + c;
            b = b - c + d;
            asm volatile("" : : : "esi", "edi");
            if (a > 100) goto case_10;
            break;
        
        case 6:
            c = c * d * e;
            d = d / (e ? e : 1);
            fl2 = fl1 * fl3;
            goto case_1;
        
        case 7:
            e = e ^ f ^ g;
            f = f | g | h;
            s2 = s1 - s3;
            break;
        
        case 8:
            g = g & h & i;
            h = h << 3;
            ch2 = ch1 - 1;
            if (h < 50) goto case_5;
            break;
        
        case 9:
            i = i * j * a;
            j = j % 11;
            db2 = db1 / 2.0;
            goto case_2;
        
        case_10:
        case 10:
            a = a * 2 + b;
            b = b / 2 + c;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
        
        case 11:
            c = c - d - e;
            d = d * e * f;
            fl3 = fl1 + fl2;
            goto case_15;
        
        case 12:
            e = e | f | g;
            f = f ^ g ^ h;
            s3 = s1 + s2;
            break;
        
        case 13:
            g = g << 1;
            h = h >> 1;
            ch3 = ch1 + ch2;
            if (g > h) goto case_10;
            break;
        
        case 14:
            i = i + j + a;
            j = j - a + b;
            db1 = db2 + 1.0;
            goto case_5;
        
        case_15:
        case 15:
            a = a ^ b ^ c;
            b = b & c & d;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
        
        case 16:
            c = c << 2;
            d = d >> 2;
            fl1 = fl2 - fl3;
            goto case_0;
        
        case 17:
            e = e % 13;
            f = f * 3;
            s1 = s2 * s3;
            break;
        
        case 18:
            g = g + h + i;
            h = h - i + j;
            ch1 = ch3 - ch2;
            if (g < 0) goto case_15;
            break;
        
        case 19:
            i = i | j | a;
            j = j & a & b;
            db2 = db1 * 3.0;
            goto case_10;
        
        case_negative:
        default:
            a = -a;
            b = -b;
            c = -c;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            break;
    }
    
    /* Use all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)fl1 + (int)fl2 + (int)fl3;
    result += (int)db1 + (int)db2;
    
    return result;
}

/* Another function to create additional control flow complexity */
NOINLINE static int nested_control_flow(volatile int iter) {
    int total = 0;
    volatile int counter = iter;
    
    while (counter > 0) {
        if (counter % 3 == 0) {
            total += register_pressure_function(counter);
            asm volatile("" : : : "rax", "rbx", "rcx");
        } else if (counter % 3 == 1) {
            for (int i = 0; i < 5; i++) {
                total += register_pressure_function(counter + i);
            }
        } else {
            do {
                total += register_pressure_function(counter * 2);
                counter--;
            } while (counter % 7 != 0 && counter > 0);
        }
        counter--;
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile int selector = 0;
    long long total_result = 0;
    
    /* Use command line argument or default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop to ensure function is compiled with optimizations */
    for (int i = 0; i < iterations; i++) {
        /* Volatile to prevent optimization */
        selector = (rand() % 100) - 25;
        
        /* Call the high-pressure function */
        total_result += register_pressure_function(selector);
        
        /* Periodically call nested function for more complexity */
        if (i % 1000 == 0) {
            total_result += nested_control_flow(i % 50);
        }
        
        /* More register clobbering */
        asm volatile("" : : : "memory");
    }
    
    printf("Total result: %lld\n", total_result);
    
    return 0;
}

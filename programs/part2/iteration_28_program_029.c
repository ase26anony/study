/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and irreducible control flow */
NOINLINE int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    double db1 = 4.4, db2 = 5.5;
    
    /* Use goto to create irreducible control flow */
    if (selector < 0) goto label_irreducible;
    
    /* Large switch statement creating many basic blocks */
    switch (selector % 20) {
        case 0:
            a = b + c;
            b = c * d;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            c = d - e;
            d = e / (a ? a : 1);
            fl1 = fl2 * fl3;
            goto case_10;
        
        case 2:
            e = f ^ g;
            f = g | h;
            ch1 = ch2 + 1;
            goto label_irreducible;
        
        case 3:
            g = h & i;
            h = i << 2;
            db1 = db2 * 2.0;
            break;
        
        case 4:
            i = j >> 1;
            j = a + b;
            s1 = s2 - s3;
            goto case_15;
        
        case_5:
        case 5:
            a = c * d + e;
            fl2 = fl3 / 2.0f;
            asm volatile("" : : : "esi", "edi");
            goto case_8;
        
        case 6:
            b = d - e * f;
            ch2 = ch3 - 1;
            break;
        
        case 7:
            c = e / (f ? f : 1);
            db2 = db1 + 1.0;
            goto case_12;
        
        case_8:
        case 8:
            d = f ^ g ^ h;
            s2 = s3 * 2;
            break;
        
        case 9:
            e = g | h | i;
            fl3 = fl1 - fl2;
            goto case_14;
        
        case_10:
        case 10:
            f = h & i & j;
            ch3 = ch1 * 2;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            break;
        
        case 11:
            g = i << (j % 4);
            s3 = s1 + s2;
            goto label_irreducible;
        
        case_12:
        case 12:
            h = j >> (a % 4);
            db1 = db2 / 2.0;
            break;
        
        case 13:
            i = a + b + c + d;
            fl1 = fl2 + fl3;
            goto case_18;
        
        case 14:
            j = b - c - d - e;
            ch1 = ch2 + ch3;
            break;
        
        case_15:
        case 15:
            a = c * d * e * f;
            s1 = s2 / (s3 ? s3 : 1);
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            break;
        
        case 16:
            b = d ^ e ^ f ^ g;
            db2 = db1 * 3.0;
            goto case_19;
        
        case 17:
            c = e | f | g | h;
            fl2 = fl3 * 1.5f;
            break;
        
        case_18:
        case 18:
            d = f & g & h & i;
            ch2 = ch3 - ch1;
            goto case_0;
        
        case_19:
        case 19:
            e = g << (h % 3);
            s2 = s3 << 1;
            break;
        
        case_0:
        default:
            f = h >> (i % 3);
            fl3 = fl1 / 1.1f;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            break;
    }
    
    /* Irreducible region created with goto */
    label_irreducible:
    if (selector % 3 == 0) {
        a = b + c;
        goto case_5;
    } else if (selector % 3 == 1) {
        b = c + d;
        goto case_10;
    } else {
        c = d + e;
        goto case_15;
    }
    
    /* More complex operations mixing all variables */
    a = a + b - c * d / (e ? e : 1);
    b = (f ^ g) | (h & i) << (j % 4);
    c = s1 + s2 - s3;
    d = ch1 * ch2 + ch3;
    e = (int)(fl1 + fl2 + fl3);
    f = (int)(db1 * db2);
    
    /* Use all variables in return value to prevent elimination */
    return a + b + c + d + e + f + g + h + i + j + s1 + s2 + s3 + ch1 + ch2 + ch3;
}

/* Another function with different control flow pattern */
NOINLINE int secondary_pressure_function(volatile int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    float y1 = 1.5f, y2 = 2.5f, y3 = 3.5f;
    
    /* Nested loops with conditions */
    for (int i = 0; i < (mode % 10); i++) {
        x1 += i;
        for (int j = 0; j < (mode % 5); j++) {
            x2 += j;
            if (j % 2) {
                x3 *= 2;
                asm volatile("" : : : "rax", "rbx", "rcx");
                goto inner_label;
            } else {
                x4 /= 2;
                goto outer_label;
            }
            
            inner_label:
            x5 = x6 + x7;
        }
        outer_label:
        x6 = x7 - x8;
    }
    
    /* Switch inside loop */
    while (x9 < 100) {
        switch (x9 % 7) {
            case 0: x1 += x2; break;
            case 1: x2 -= x3; break;
            case 2: x3 *= x4; break;
            case 3: x4 /= (x5 ? x5 : 1); break;
            case 4: x5 = x6 ^ x7; break;
            case 5: x6 = x7 | x8; break;
            case 6: x7 = x8 & x9; break;
        }
        x9++;
        asm volatile("" : : : "rdx", "rsi", "rdi");
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + (int)(y1 + y2 + y3);
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
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to exercise different control paths */
        selector = rand() % 100;
        
        /* Call both pressure functions */
        total += register_pressure_function(selector);
        
        if (i % 3 == 0) {
            total += secondary_pressure_function(selector % 20);
        }
        
        /* Occasionally change selector mid-loop */
        if (i % 7 == 0) {
            selector = (selector * 13 + 7) % 50;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

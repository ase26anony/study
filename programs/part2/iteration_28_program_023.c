/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short f = 6, g = 7, h = 8, i = 9, j = 10;
    char k = 11, l = 12, m = 13, n = 14, o = 15;
    float p = 16.0f, q = 17.0f, r = 18.0f;
    double s = 19.0, t = 20.0;
    unsigned int u = 21, v = 22, w = 23;
    
    /* Labels for goto statements to create irreducible CFG */
    label1:
    label2:
    label3:
    
    /* Complex switch with many cases */
    switch (selector) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            goto label2;
            
        case 1:
            g = h - i;
            j = k * l;
            p = q + r;
            asm volatile("" : : : "eax", "ebx", "ecx");
            if (a > 10) goto label1;
            break;
            
        case 2:
            m = n | o;
            s = t / 2.0;
            asm volatile("" : : : "eax", "ebx");
            goto label3;
            
        case 3:
            u = v ^ w;
            a = b << 2;
            asm volatile("" : : : "eax", "ecx", "edx");
            if (c < 5) goto label1;
            break;
            
        case 4:
            d = e + f + g;
            h = i - j;
            asm volatile("" : : : "ebx", "ecx", "edx");
            goto label2;
            
        case 5:
            k = l * m;
            n = o / 2;
            p = q * r;
            asm volatile("" : : : "eax", "ebx", "esi", "edi");
            break;
            
        case 6:
            s = t + 1.0;
            u = v & w;
            asm volatile("" : : : "eax", "ecx");
            if (d > 20) goto label3;
            break;
            
        case 7:
            a = b * c * d;
            e = f | g;
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto label1;
            
        case 8:
            h = i ^ j;
            k = l << 1;
            m = n >> 1;
            asm volatile("" : : : "ebx", "ecx", "esi");
            break;
            
        case 9:
            o = p + q;
            r = s * t;
            u = v - w;
            asm volatile("" : : : "eax", "edx", "edi");
            if (e < 15) goto label2;
            break;
            
        default:
            a = b + c + d + e;
            f = g * h;
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            goto label3;
    }
    
    /* More irreducible control flow with nested loops */
    int x, y;
    for (x = 0; x < 3; x++) {
        for (y = 0; y < 2; y++) {
            if (x == y) {
                a += b;
                goto label1;
            } else {
                c -= d;
                goto label2;
            }
        }
    }
    
    label3:
    /* Final computation using all variables */
    int result = a + b + c + d + e + f + g + h + i + j + 
                 k + l + m + n + o + (int)p + (int)q + (int)r + 
                 (int)s + (int)t + u + v + w;
    
    return result;
}

/* Another complex function to increase overall CFG complexity */
NOINLINE static int secondary_pressure_function(volatile int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    switch (mode % 5) {
        case 0:
            x1 = x2 * x3;
            x4 = x5 + x6;
            asm volatile("" : : : "eax", "ebx");
            break;
        case 1:
            x7 = x8 - x9;
            x10 = x1 * x2;
            asm volatile("" : : : "ecx", "edx");
            break;
        case 2:
            x3 = x4 | x5;
            x6 = x7 ^ x8;
            asm volatile("" : : : "esi", "edi");
            break;
        case 3:
            x9 = x10 << 1;
            x1 = x2 >> 1;
            asm volatile("" : : : "eax", "ebx", "ecx");
            break;
        case 4:
            x3 = x4 * x5 * x6;
            x7 = x8 + x9 + x10;
            asm volatile("" : : : "edx", "esi", "edi");
            break;
    }
    
    /* Unstructured control flow */
    if (mode & 1) {
        goto jump_point1;
    } else {
        goto jump_point2;
    }
    
    jump_point1:
    x1 = x2 + x3;
    goto end_point;
    
    jump_point2:
    x4 = x5 - x6;
    
    end_point:
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
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
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to exercise different control paths */
        selector = rand() % 15;
        
        /* Call both pressure functions */
        int result1 = register_pressure_function(selector);
        int result2 = secondary_pressure_function(selector + i);
        
        total += result1 + result2;
        
        /* Occasionally change control flow pattern */
        if (i % 1000 == 0) {
            selector = (selector * 13 + 7) % 20;
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional test with different optimization patterns */
    volatile int test_var = 0;
    for (int i = 0; i < 1000; i++) {
        test_var += register_pressure_function(i % 12);
        test_var -= secondary_pressure_function(i % 8);
    }
    
    printf("Final test_var: %d\n", test_var);
    
    return 0;
}

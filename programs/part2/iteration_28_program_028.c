/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with extreme register pressure and complex control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15;
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases - creates many basic blocks */
    switch (control & 0xF) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case1_label;
            
        case 1:
        case1_label:
            g = h - i;
            j = k / 2;
            fl1 = fl2 + fl3;
            asm volatile("" : : : "esi", "edi");
            if (a > 10) goto case3_label;
            break;
            
        case 2:
            m = n ^ o;
            s1 = s2 + s3;
            ch1 = ch2 + 1;
            asm volatile("" : : : "r8", "r9", "r10");
            for (int x = 0; x < 3; x++) {
                a += x;
                b *= (x + 1);
            }
            break;
            
        case 3:
        case3_label:
            c = d << 2;
            e = f >> 1;
            fl2 = fl1 * 2.0f;
            asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
            /* Nested loop */
            for (int y = 0; y < 2; y++) {
                for (int z = 0; z < 2; z++) {
                    g += y * z;
                }
            }
            goto case5_label;
            
        case 4:
            h = i | j;
            k = l & m;
            s3 = s4 - s1;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            if (ch3 < 'M') {
                ch4 = ch3 + 5;
                goto case7_label;
            }
            break;
            
        case 5:
        case5_label:
            n = o ^ a;
            b = c * d;
            fl3 = fl1 / fl2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            do {
                e++;
                f--;
            } while (e < 20);
            break;
            
        case 6:
            d = e + f + g;
            h = i - j - k;
            s4 = s1 * s2;
            asm volatile("" : : : "xmm6", "xmm7");
            switch (ch2) {
                case 'A': a += 10; break;
                case 'B': b += 20; break;
                default: c += 30; break;
            }
            break;
            
        case 7:
        case7_label:
            l = m + n + o;
            ch2 = ch1 + ch3;
            fl1 = fl2 - fl3;
            asm volatile("" : : : "rax", "rbx", "rcx");
            while (s2 < 500) {
                s2 += 50;
                s3 -= 25;
            }
            goto case9_label;
            
        case 8:
            f = g * h * i;
            j = k / l;
            s1 = s2 % s3;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            if (fl1 > 0.0f) {
                fl2 = fl1 * 3.14f;
                goto case2_label;
            }
            break;
            
        case 9:
        case9_label:
            m = n & o & a;
            b = c | d;
            ch3 = ch4 - ch1;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            for (int w = 0; w < 4; w++) {
                if (w % 2 == 0) {
                    e += w;
                    goto inner_label;
                }
                inner_label:
                f -= w;
            }
            break;
            
        case 10:
            o = a ^ b ^ c;
            d = e << 1;
            s2 = s3 >> 2;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            /* Irreducible control flow with goto */
            if (ch4 > 'Z') goto case4_label;
            else goto case6_label;
            
        case 11:
        case2_label:
            a = b + c + d + e;
            f = g - h - i;
            fl2 = fl3 * 4.0f;
            asm volatile("" : : : "xmm8", "xmm9", "xmm10");
            /* Another nested control structure */
            for (int p = 0; p < 3; p++) {
                switch (p) {
                    case 0: j += p; break;
                    case 1: k += p; goto inner_loop;
                    case 2: l += p; break;
                }
                inner_loop:
                m += p * 2;
            }
            break;
            
        case 12:
            g = h * i * j * k;
            l = m / n;
            ch4 = ch1 + ch2 + ch3;
            asm volatile("" : : : "xmm11", "xmm12", "xmm13", "xmm14");
            if (s4 > 1000) {
                s1 = s4 / 2;
                goto case8_label;
            }
            break;
            
        case 13:
        case4_label:
            c = d ^ e ^ f;
            h = i << 3;
            s3 = s4 >> 1;
            asm volatile("" : : : "xmm15");
            /* Complex conditional jumps */
            if (fl3 < 10.0f) {
                if (a > b) goto case10_label;
                else goto case12_label;
            }
            break;
            
        case 14:
        case6_label:
            e = f + g + h + i + j;
            k = l - m;
            fl1 = fl2 / fl3;
            asm volatile("" : : : "st", "st(1)", "st(2)");
            /* Multiple goto targets */
            if (ch2 == 'B') goto case13_label;
            if (ch3 == 'C') goto case14_label;
            break;
            
        case 15:
        case8_label:
        case10_label:
        case12_label:
        case13_label:
        case14_label:
            n = o + a + b + c + d + e;
            f = g * h * i * j * k;
            s4 = s1 + s2 + s3;
            ch1 = ch2 + ch3 + ch4;
            fl3 = fl1 + fl2;
            asm volatile("" : : : "mm0", "mm1", "mm2", "mm3");
            /* Final computation using all variables */
            result = a + b + c + d + e + f + g + h + i + j + 
                    k + l + m + n + o + s1 + s2 + s3 + s4 + 
                    ch1 + ch2 + ch3 + ch4 + (int)fl1 + 
                    (int)fl2 + (int)fl3;
            break;
    }
    
    /* Use all variables to prevent optimization */
    result += a - b + c * d - e / (f + 1) + g % (h + 1) +
             i ^ j ^ k | l & m | n ^ o +
             s1 * s2 - s3 + s4 +
             ch1 * ch2 + ch3 - ch4 +
             (int)(fl1 * 100.0f) + (int)(fl2 * 50.0f) + (int)(fl3 * 25.0f);
    
    return result;
}

/* Another function to create additional control flow complexity */
NOINLINE static int secondary_pressure_function(int base) {
    volatile int x = base;
    int arr[20];
    int sum = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * base;
    }
    
    /* Complex loop with conditionals */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            for (int j = 0; j < i; j++) {
                if (j % 2 == 0) {
                    arr[i] += arr[j];
                    goto inner_point;
                } else {
                    arr[i] -= arr[j];
                }
                inner_point:
                asm volatile("" : : : "eax", "ebx");
            }
        } else if (i % 3 == 1) {
            int k = 0;
            while (k < i) {
                arr[i] *= (k + 1);
                k++;
                if (k > 10) break;
            }
        } else {
            do {
                arr[i] /= 2;
                asm volatile("" : : : "ecx", "edx");
            } while (arr[i] > 100);
        }
        
        sum += arr[i];
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    long long total = 0;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Seed random number generator */
    srand(time(NULL));
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Main hot loop */
    for (int count = 0; count < iterations; count++) {
        volatile int selector = rand() % 256;
        
        /* Call the register pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function to add more complexity */
        int result2 = secondary_pressure_function(selector & 0xF);
        
        /* Mix results to create data dependencies */
        total += result1 + result2;
        
        /* Occasionally change control flow */
        if (count % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Additional test with different optimization patterns */
    if (iterations > 1000) {
        printf("Running additional MCF pattern tests...\n");
        
        /* Test with different selector patterns */
        for (int pattern = 0; pattern < 16; pattern++) {
            volatile int patterned_selector = pattern * 17;
            int r = register_pressure_function(patterned_selector);
            total += r;
            
            /* Force different execution paths */
            switch (pattern % 4) {
                case 0:
                    asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
                    break;
                case 1:
                    asm volatile("" : : : "rsi", "rdi", "r8", "r9");
                    break;
                case 2:
                    asm volatile("" : : : "r10", "r11", "r12", "r13");
                    break;
                case 3:
                    asm volatile("" : : : "r14", "r15", "xmm0", "xmm1");
                    break;
            }
        }
    }
    
    printf("Final total: %lld\n", total);
    return (int)(total % 1000);
}

/* test_mcf.c - Program to trigger MCF algorithm's special block printing logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NOINLINE __attribute__((noinline, noipa))

/* Function with high register pressure and irreducible control flow */
NOINLINE static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'a', ch2 = 'b', ch3 = 'c';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases */
    switch (control & 0xF) {
        case 0:
            a = b + c;
            b = c * d;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto label1;
        
        case 1:
            c = d - e;
            d = e / (a ? a : 1);
            asm volatile("" : : : "esi", "edi");
            goto label3;
        
        case 2:
        label1:
            e = f ^ g;
            f = g | h;
            asm volatile("" : : : "r8", "r9", "r10");
            if (a > b) goto label2;
            else goto label4;
        
        case 3:
            g = h & i;
            h = i << 2;
            asm volatile("" : : : "r11", "r12", "r13");
            goto label5;
        
        case 4:
        label2:
            i = j >> 1;
            j = a + b + c;
            asm volatile("" : : : "xmm0", "xmm1");
            fl1 = fl2 * fl3;
            goto label6;
        
        case 5:
            s1 = s2 + s3;
            s2 = s3 - s1;
            asm volatile("" : : : "xmm2", "xmm3");
            goto label7;
        
        case 6:
        label3:
            s3 = ch1 * ch2;
            ch1 = ch2 + 1;
            asm volatile("" : : : "xmm4", "xmm5");
            if (ch3 == 'c') goto label8;
            break;
        
        case 7:
            ch2 = ch3 - 'a';
            ch3 = ch1 * 2;
            asm volatile("" : : : "xmm6", "xmm7");
            goto label9;
        
        case 8:
        label4:
            fl2 = fl3 / 2.0f;
            fl3 = fl1 + 1.0f;
            asm volatile("" : : : "mm0", "mm1");
            goto label10;
        
        case 9:
            a = b * c + d;
            b = c - d * e;
            asm volatile("" : : : "mm2", "mm3");
            goto label2;
        
        case 10:
        label5:
            c = d | e & f;
            d = e ^ f ^ g;
            asm volatile("" : : : "mm4", "mm5");
            goto label3;
        
        case 11:
            e = f << (g & 3);
            f = g >> (h % 4);
            asm volatile("" : : : "mm6", "mm7");
            goto label1;
        
        case 12:
        label6:
            g = h + i - j;
            h = i * j / (a ? a : 1);
            asm volatile("" : : : "st", "st(1)");
            goto label5;
        
        case 13:
            i = j % (b ? b : 1);
            j = a * b * c;
            asm volatile("" : : : "st(2)", "st(3)");
            goto label4;
        
        case 14:
        label7:
            s1 = s2 * s3;
            s2 = s3 / (s1 ? s1 : 1);
            asm volatile("" : : : "st(4)", "st(5)");
            goto label6;
        
        case 15:
        label8:
            s3 = ch1 + ch2 + ch3;
            ch1 = ch2 - ch3;
            asm volatile("" : : : "st(6)", "st(7)");
            goto label7;
        
        default:
        label9:
            fl1 = fl2 + fl3;
            fl2 = fl3 - fl1;
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            break;
    }
    
    label10:
    /* Use all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)fl1 + (int)fl2 + (int)fl3;
    
    /* More irreducible control flow with gotos */
    if (result & 1) {
        goto label11;
    } else {
        goto label12;
    }
    
    label11:
    result ^= 0xAAAA;
    goto label13;
    
    label12:
    result ^= 0x5555;
    goto label13;
    
    label13:
    /* Final register clobbering */
    asm volatile("" : : : "memory");
    return result;
}

/* Another complex function to increase overall pressure */
NOINLINE static int secondary_pressure(volatile int x) {
    int arr[20];
    int sum = 0;
    
    for (int i = 0; i < 20; i++) {
        arr[i] = x + i;
        asm volatile("" : : : "eax", "ebx");
    }
    
    /* Complex loop with conditionals */
    for (int i = 0; i < 20; i++) {
        if (i & 1) {
            arr[i] *= 2;
            goto loop_mid;
        } else {
            arr[i] /= 2;
            goto loop_end;
        }
        
        loop_mid:
        arr[i] += i;
        goto loop_cont;
        
        loop_end:
        arr[i] -= i;
        goto loop_cont;
        
        loop_cont:
        sum += arr[i];
    }
    
    return sum;
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
    
    for (int count = 0; count < iterations; count++) {
        /* Vary selector to hit different switch cases */
        selector = (selector * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Call high-pressure functions */
        int res1 = register_pressure_function(selector);
        int res2 = secondary_pressure(selector & 0xFF);
        
        total += res1 + res2;
        
        /* Occasionally change control flow pattern */
        if ((count & 0xFFF) == 0) {
            selector ^= 0xAAAAAAAA;
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

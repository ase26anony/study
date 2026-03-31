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
    
    /* Complex irreducible control flow using goto */
    if (selector < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (selector % 10) {
        case 0:
            a = b + c;
            b = c * d;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_common;
            
        case 1:
            c = d - e;
            d = e / (a ? a : 1);
            asm volatile("" : : : "esi", "edi");
            goto case_common;
            
        case 2:
            e = f * g;
            f = g ^ h;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case_common;
            
        case 3:
            g = h | i;
            h = i & j;
            asm volatile("" : : : "r11", "r12", "r13");
            goto case_common;
            
        case 4:
            i = j << 2;
            j = a >> 1;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case_common;
            
        case 5:
            s1 = s2 + s3;
            s2 = s3 - s1;
            asm volatile("" : : : "xmm2", "xmm3");
            goto case_common;
            
        case 6:
            ch1 = ch2 + 1;
            ch2 = ch3 - 1;
            asm volatile("" : : : "xmm4", "xmm5");
            goto case_common;
            
        case 7:
            fl1 = fl2 * 2.0f;
            fl2 = fl3 / 2.0f;
            asm volatile("" : : : "xmm6", "xmm7");
            goto case_common;
            
        case 8:
            db1 = db2 * 3.0;
            db2 = db1 / 3.0;
            asm volatile("" : : : "xmm8", "xmm9");
            goto case_common;
            
        case 9:
            a = b + c + d + e + f;
            asm volatile("" : : : "xmm10", "xmm11", "xmm12");
            goto case_common;
            
        case_negative:
            /* Another path into the control flow */
            a = -b;
            b = -c;
            /* Fall through */
            
        default:
            /* Complex arithmetic mixing all types */
            a = (int)(b + c + (d * e) / (f ? f : 1));
            s1 = (short)(ch1 + ch2 + ch3);
            fl1 = (float)(db1 + db2) / 2.0f;
            asm volatile("" : : : "xmm13", "xmm14", "xmm15");
    }
    
case_common:
    /* More irreducible control flow with nested loops */
    for (int x = 0; x < 3; x++) {
        if (x == 1) goto loop_inner;
        for (int y = 0; y < 2; y++) {
            if (y == 0 && x == 0) goto skip_point;
loop_inner:
            a += x * y;
skip_point:
            b -= y;
        }
    }
    
    /* Final computation using all variables */
    int result = a + b + c + d + e + f + g + h + i + j +
                 s1 + s2 + s3 + ch1 + ch2 + ch3 +
                 (int)fl1 + (int)fl2 + (int)fl3 +
                 (int)db1 + (int)db2;
    
    /* More register clobbering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another complex function to increase overall pressure */
NOINLINE int secondary_pressure(volatile int x) {
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
        asm volatile("" : : : "rax", "rbx", "rcx");
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) goto add_special;
        sum += arr[i];
        continue;
add_special:
        sum += arr[i] * 2;
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
    volatile int selector = rand();
    
    long long total = 0;
    
    /* Hot loop calling pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call main pressure function */
        total += register_pressure_function(selector % 20 - 5);  /* Range: -5 to 14 */
        
        /* Call secondary function occasionally */
        if (i % 7 == 0) {
            total += secondary_pressure(i % 13);
        }
        
        /* More control flow variation */
        if (i % 100 == 0) {
            selector = rand();
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

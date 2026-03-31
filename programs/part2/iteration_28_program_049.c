/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no inlining to preserve complex control flow */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    short s1 = 11, s2 = 12, s3 = 13;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    double db1 = 4.4, db2 = 5.5;
    volatile int control = selector;
    
    /* Complex irreducible control flow using goto */
    if (control < 0) goto case_negative;
    
    /* Large switch with many cases */
    switch (control % 20) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_5;
        
        case 1:
            g = h - i;
            j = a * b;
            fl1 = fl2 + fl3;
            goto case_10;
        
        case 2:
            s1 = s2 * s3;
            ch1 = ch2 + 1;
            db1 = db2 * 2.0;
            break;
        
        case 3:
            a = c * d;
            e = f + g;
            asm volatile("" : : : "esi", "edi");
            goto case_15;
        
        case 4:
            h = i / j;
            s2 = s1 - s3;
            fl2 = fl1 * 3.0f;
            break;
        
        case_5:
        case 5:
            b = a << 2;
            c = d >> 1;
            ch2 = ch3 - 1;
            asm volatile("" : : : "r8", "r9", "r10", "r11");
            goto case_1;
        
        case 6:
            f = g ^ h;
            i = j | a;
            db2 = db1 / 2.0;
            break;
        
        case 7:
            s3 = s1 + s2;
            fl3 = fl2 - fl1;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2");
            goto case_12;
        
        case 8:
            a = b * c * d;
            e = f % g;
            ch3 = ch1 * 2;
            break;
        
        case 9:
            h = i + j + a;
            s1 = s2 * s3 / 2;
            asm volatile("" : : : "xmm3", "xmm4", "xmm5");
            goto case_18;
        
        case_10:
        case 10:
            b = c - d;
            fl1 = fl2 * fl3;
            db1 = a + b;
            break;
        
        case 11:
            g = h * i;
            j = f - e;
            ch1 = ch2 + ch3;
            asm volatile("" : : : "r12", "r13", "r14", "r15");
            goto case_3;
        
        case 12:
        case_12:
            a = b / (c + 1);
            d = e * f - g;
            s2 = s3 << 1;
            break;
        
        case 13:
            h = i ^ j;
            fl2 = fl3 + 1.0f;
            db2 = db1 * 3.0;
            asm volatile("" : : : "mm0", "mm1");
            goto case_7;
        
        case 14:
            a = b * c + d * e;
            f = g - h;
            ch2 = ch1 | 0x20;
            break;
        
        case_15:
        case 15:
            i = j % a;
            s3 = s1 + s2 * 2;
            fl3 = fl1 / fl2;
            asm volatile("" : : : "st", "st(1)", "st(2)");
            goto case_11;
        
        case 16:
            b = c << d;
            e = f >> g;
            db1 = db2 + a;
            break;
        
        case 17:
            h = i * j;
            a = b + c + d;
            ch3 = ch2 & 0x7F;
            asm volatile("" : : : "ymm0", "ymm1");
            goto case_13;
        
        case_18:
        case 18:
            f = g / h;
            i = j * a;
            s1 = s2 - s3;
            break;
        
        case 19:
            b = c ^ d ^ e;
            fl1 = fl2 * 2.0f;
            db2 = db1 / 4.0;
            asm volatile("" : : : "zmm0", "zmm1");
            goto case_0;
        
        default:
            a = b = c = 0;
            break;
    }
    
    /* Post-switch computations */
    if (control > 100) {
        a = a * 2 + b;
        c = d - e + f;
        g = h / (i + 1);
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
        goto final_compute;
    }
    
case_negative:
    a = -a;
    b = -b;
    c = -c;
    
final_compute:
    /* Force use of all variables to prevent elimination */
    int result = a + b + c + d + e + f + g + h + i + j;
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)fl1 + (int)fl2 + (int)fl3;
    result += (int)db1 + (int)db2;
    
    /* More register clobbering */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Another complex function to increase overall pressure */
__attribute__((noinline))
int secondary_pressure(volatile int x) {
    int arr[20];
    for (int k = 0; k < 20; k++) {
        arr[k] = x * k;
        asm volatile("" : : : "rsi", "rdi");
    }
    
    int sum = 0;
    for (int k = 0; k < 20; k++) {
        sum += arr[k];
        if (k % 3 == 0) {
            asm volatile("" : : : "r8", "r9");
            sum -= x;
        } else if (k % 3 == 1) {
            asm volatile("" : : : "r10", "r11");
            sum *= 2;
        } else {
            asm volatile("" : : : "r12", "r13");
            sum /= 3;
        }
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
    volatile int seed = rand();
    
    long long total = 0;
    
    /* Hot loop calling pressure functions */
    for (int count = 0; count < iterations; count++) {
        volatile int selector = seed + count;
        
        /* Call main pressure function */
        int res1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int res2 = secondary_pressure(selector % 100);
        
        /* Mix results to create data dependencies */
        total += res1 * 3 + res2 * 2;
        
        /* Modify seed to change control flow */
        if (count % 7 == 0) {
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
            asm volatile("" : : : "rax", "rbx");
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

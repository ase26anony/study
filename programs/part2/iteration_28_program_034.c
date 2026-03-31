/* test_mcf.c - Program to trigger MCF algorithm special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force complex control flow and prevent inlining */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    volatile int control = selector;
    int result = 0;
    
    /* Complex irreducible control flow with goto */
    if (control < 0) goto negative_case;
    
    /* Large switch statement creating many basic blocks */
    switch (control % 10) {
        case 0:
            a = b + c;
            b = c * d;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_common;
        
        case 1:
            c = d - e;
            d = e / (f ? f : 1);
            asm volatile("" : : : "esi", "edi");
            goto case_common;
        
        case 2:
            e = f ^ g;
            f = g | h;
            asm volatile("" : : : "r8", "r9", "r10");
            goto negative_case;
        
        case 3:
            g = h << 2;
            h = a >> 1;
            asm volatile("" : : : "r11", "r12", "r13");
            goto case_common;
        
        case 4:
            s1 = s2 + s3;
            s2 = s3 - s4;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case_common;
        
        case 5:
            fl1 = fl2 * fl3;
            fl2 = fl3 + 1.0f;
            asm volatile("" : : : "xmm2", "xmm3", "xmm4");
            goto negative_case;
        
        case 6:
            ch1 = ch2 + 1;
            ch2 = ch3 - 1;
            asm volatile("" : : : "rax", "rbx");
            goto case_common;
        
        case 7:
            a = b * c * d;
            asm volatile("" : : : "rcx", "rdx", "rsi", "rdi");
            goto negative_case;
        
        case 8:
            /* Nested loop inside case */
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    a += i * j;
                    asm volatile("" : : : "r8", "r9", "r10", "r11");
                }
            }
            goto case_common;
        
        case 9:
            /* Another irreducible region */
            if (a > b) goto label1;
            if (c > d) goto label2;
            label1:
                e = f + g;
                asm volatile("" : : : "r12", "r13", "r14", "r15");
            label2:
                h = a - b;
                goto case_common;
        
        default:
            goto negative_case;
    }
    
    case_common:
        /* Common processing with more operations */
        a = a * 2 + b;
        b = b / 2 + c;
        c = c - 1 + d;
        d = d % 10 + e;
        asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
        goto finish;
    
    negative_case:
        /* Alternative path */
        a = -a;
        b = -b;
        c = -c;
        d = -d;
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
    
    finish:
        /* Combine all variables to prevent elimination */
        result = a + b + c + d + e + f + g + h + s1 + s2 + s3 + s4 
                 + ch1 + ch2 + ch3 + ch4 + (int)fl1 + (int)fl2 + (int)fl3;
    
    /* Final register clobber */
    asm volatile("" : : : "memory");
    return result;
}

/* Another complex function to increase overall pressure */
__attribute__((noinline))
int secondary_pressure(volatile int x) {
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
        asm volatile("" : : : "eax", "ebx");
    }
    
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += arr[i % 10] * arr[(i + 5) % 10];
    }
    
    /* Complex conditional */
    if (sum > 1000) {
        asm volatile("" : : : "ecx", "edx");
        return sum / 2;
    } else if (sum > 500) {
        asm volatile("" : : : "esi", "edi");
        return sum * 2;
    } else {
        asm volatile("" : : : "r8", "r9");
        return sum + 100;
    }
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
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different control paths */
        selector = (selector * 1103515245 + 12345) & 0x7fffffff;
        
        /* Call pressure functions */
        int r1 = register_pressure_function(selector);
        int r2 = secondary_pressure(selector % 100);
        
        total += r1 + r2;
        
        /* Occasionally change control flow */
        if (i % 1000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    return 0;
}

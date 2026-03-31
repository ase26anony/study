/* test_mcf.c - Program to trigger MCF algorithm special block printing */
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
    char ch1 = 'A', ch2 = 'B', ch3 = 'C';
    float fl1 = 1.1f, fl2 = 2.2f, fl3 = 3.3f;
    double db1 = 4.4, db2 = 5.5;
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with many cases creating multiple basic blocks */
    switch (control % 12) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto label1;
        
        case 1:
            g = h - i;
            j = a * b;
            asm volatile("" : : : "esi", "edi");
            /* Jump to another case block */
            if (ch1 == 'A') goto case3;
            break;
            
        case 2:
        label1:
            s1 = s2 + s3;
            ch1 = ch2 + 1;
            fl1 = fl2 * fl3;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case4;
            
        case 3:
        case3:
            db1 = db2 / 2.0;
            result = a + b + c;
            asm volatile("" : : : "r8", "r9", "r10");
            if (result > 10) goto case5;
            break;
            
        case 4:
        case4:
            a = (b << 2) | c;
            d = e ^ f;
            asm volatile("" : : : "r11", "r12", "r13");
            goto case6;
            
        case 5:
        case5:
            s2 = s1 * s3;
            ch2 = ch3 - 1;
            asm volatile("" : : : "xmm2", "xmm3", "xmm4");
            break;
            
        case 6:
        case6:
            fl2 = fl1 + fl3;
            db2 = db1 * 3.0;
            asm volatile("" : : : "r14", "r15");
            goto case8;
            
        case 7:
            g = h / i;
            j = a % b;
            asm volatile("" : : : "rax", "rbx", "rcx");
            if (j == 0) goto case9;
            break;
            
        case 8:
        case8:
            a = b & c;
            d = e | f;
            asm volatile("" : : : "rdx", "rsi", "rdi");
            break;
            
        case 9:
        case9:
            s3 = s1 + s2;
            ch3 = ch1 + ch2;
            asm volatile("" : : : "xmm5", "xmm6", "xmm7");
            goto case11;
            
        case 10:
            fl3 = fl1 - fl2;
            result = d + e + f;
            asm volatile("" : : : "xmm8", "xmm9", "xmm10");
            break;
            
        case 11:
        case11:
            a = ~b;
            d = c ^ 0xFF;
            asm volatile("" : : : "xmm11", "xmm12", "xmm13", "xmm14");
            /* Create loop within switch for more complexity */
            for (int k = 0; k < 3; k++) {
                result += k * a;
                asm volatile("" : : : "r8", "r9");
            }
            break;
    }
    
    /* More irreducible control flow with gotos */
    if (result > 50) {
        goto compute_more;
    } else if (result < 20) {
        goto adjust_values;
    }
    
    /* Normal path */
    result = a + b + c + d + e + f + g + h + i + j;
    goto finalize;
    
compute_more:
    result = result * 2 - s1 - s2 - s3;
    asm volatile("" : : : "rax", "rbx");
    goto finalize;
    
adjust_values:
    result = result + ch1 + ch2 + ch3 + (int)fl1 + (int)fl2 + (int)fl3;
    asm volatile("" : : : "rcx", "rdx");
    
finalize:
    /* Use all variables to prevent elimination */
    result += s1 + s2 + s3;
    result += ch1 + ch2 + ch3;
    result += (int)fl1 + (int)fl2 + (int)fl3;
    result += (int)db1 + (int)db2;
    
    /* Final register clobber */
    asm volatile("" : : : "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                                 "xmm4", "xmm5", "xmm6", "xmm7");
    
    return result;
}

/* Another complex function to create more flow graph edges */
NOINLINE static int secondary_function(volatile int x) {
    int arr[20];
    int sum = 0;
    
    /* Uninitialized use creates more register pressure */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
        asm volatile("" : : : "eax", "ebx");
    }
    
    /* Complex loop with conditionals */
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) {
            sum += arr[i];
            goto skip;
        } else if (i % 3 == 1) {
            sum -= arr[i];
            goto adjust;
        } else {
            sum *= arr[i] + 1;
        }
        
        if (sum > 1000) {
            goto overflow;
        }
        
    skip:
        asm volatile("" : : : "ecx", "edx");
        continue;
        
    adjust:
        sum = sum / 2;
        asm volatile("" : : : "esi", "edi");
        continue;
        
    overflow:
        sum = sum % 1000;
        asm volatile("" : : : "r8", "r9");
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
    
    /* Hot loop calling high-pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to hit different switch cases */
        selector = rand() % 100;
        
        /* Call both functions to create more register pressure */
        int res1 = register_pressure_function(selector);
        int res2 = secondary_function(selector);
        
        total += res1 + res2;
        
        /* Occasionally call with different patterns */
        if (i % 1000 == 0) {
            selector = i;
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total result: %lld\n", total);
    
    /* Final call with extreme values */
    selector = 0x7FFFFFFF;
    int final_res = register_pressure_function(selector);
    printf("Final call result: %d\n", final_res);
    
    return 0;
}

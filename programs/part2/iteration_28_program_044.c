/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and inter-procedural analysis */
__attribute__((noinline, noipa))
int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    char ch1 = 'A', ch2 = 'B', ch3 = 'C', ch4 = 'D';
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile int control = selector; /* Prevent optimization */
    
    /* Complex switch with many cases creating multiple basic blocks */
    switch (control & 0xF) { /* 16 cases */
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            goto case1_label; /* Irreducible control flow */
        
        case1_label:
        case 1:
            g = h - i;
            j = k / 2;
            f1 = f2 + f3;
            asm volatile("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            if (ch1 == 'A') goto case3_label;
            break;
        
        case 2:
            m = n ^ o;
            p = a | b;
            s1 = s2 + s3;
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3");
            break;
        
        case3_label:
        case 3:
            ch2 = ch3 + 1;
            ch4 = ch1 - 1;
            f4 = f1 * f2;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx");
            goto case5_label;
        
        case 4:
            a = c * d;
            e = f + g;
            asm volatile("" : : : "rsi", "rdi", "rbp", "rsp");
            break;
        
        case5_label:
        case 5:
            h = i - j;
            k = l * 2;
            s4 = s1 - s2;
            asm volatile("" : : : "xmm4", "xmm5", "xmm6", "xmm7");
            if (f3 > 0.0f) goto case7_label;
            break;
        
        case 6:
            m = n & o;
            p = a ^ b;
            f3 = f4 / 2.0f;
            asm volatile("" : : : "ymm0", "ymm1", "ymm2", "ymm3");
            break;
        
        case7_label:
        case 7:
            ch3 = ch4 << 1;
            ch1 = ch2 >> 1;
            asm volatile("" : : : "ymm4", "ymm5", "ymm6", "ymm7");
            goto case9_label;
        
        case 8:
            c = d + e;
            f = g - h;
            asm volatile("" : : : "zmm0", "zmm1", "zmm2", "zmm3");
            break;
        
        case9_label:
        case 9:
            i = j * k;
            l = m / 2;
            f2 = f3 + f4;
            asm volatile("" : : : "zmm4", "zmm5", "zmm6", "zmm7");
            if (s3 < 500) goto case11_label;
            break;
        
        case 10:
            n = o | p;
            a = b & c;
            s2 = s3 * 2;
            asm volatile("" : : : "st", "st(1)", "st(2)", "st(3)");
            break;
        
        case11_label:
        case 11:
            ch4 = ch1 + ch2;
            ch3 = ch4 - ch1;
            asm volatile("" : : : "st(4)", "st(5)", "st(6)", "st(7)");
            goto case13_label;
        
        case 12:
            d = e * f;
            g = h + i;
            asm volatile("" : : : "mm0", "mm1", "mm2", "mm3");
            break;
        
        case13_label:
        case 13:
            j = k - l;
            m = n * 2;
            f1 = f2 / 3.0f;
            asm volatile("" : : : "mm4", "mm5", "mm6", "mm7");
            if (ch3 == 'C') goto case15_label;
            break;
        
        case 14:
            o = p ^ a;
            b = c | d;
            s3 = s4 + s1;
            asm volatile("" : : : "tmm0", "tmm1", "tmm2", "tmm3");
            break;
        
        case15_label:
        case 15:
            ch2 = ch3 << 2;
            ch1 = ch4 >> 2;
            f4 = f1 * f3;
            asm volatile("" : : : "tmm4", "tmm5", "tmm6", "tmm7");
            break;
        
        default:
            /* More operations in default case */
            a = b + c + d + e + f + g + h;
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                         "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            break;
    }
    
    /* Additional irreducible control flow with gotos */
    if (a > 100) {
        goto extra_block1;
    } else {
        goto extra_block2;
    }
    
extra_block1:
    b = c * d;
    goto merge_point;
    
extra_block2:
    b = c / d;
    /* Fall through */
    
merge_point:
    /* Complex computation using all variables to prevent elimination */
    int result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + p;
    result += s1 + s2 + s3 + s4;
    result += ch1 + ch2 + ch3 + ch4;
    result += (int)(f1 + f2 + f3 + f4);
    
    /* Final register clobbering */
    asm volatile("" : : : "memory", "cc", 
                 "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                 "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
    
    return result;
}

/* Another complex function to create more flow graph edges */
__attribute__((noinline, noipa))
int secondary_pressure_function(volatile int x) {
    int arr[20];
    volatile int sum = 0;
    
    /* Loop with register pressure */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * x;
        sum += arr[i];
        asm volatile("" : : : "eax", "ebx", "ecx");
    }
    
    /* Nested loops */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i] += arr[j] + i * j;
            if (i > j) {
                goto inner_label;
            }
        }
        continue;
        
    inner_label:
        asm volatile("" : : : "edx", "esi", "edi");
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile long long total = 0;
    
    /* Use command line argument for iteration count if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    srand(time(NULL));
    volatile int base_selector = rand();
    
    printf("Starting MCF stress test with %d iterations...\n", iterations);
    
    /* Hot loop calling high register pressure functions */
    for (int iter = 0; iter < iterations; iter++) {
        volatile int selector = base_selector + iter;
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_pressure_function(selector & 0xFF);
        
        /* Mix results to create data dependencies */
        total += result1 - result2;
        
        /* Occasionally change control flow */
        if ((iter & 0xFFF) == 0) {
            asm volatile("" : : : "memory");
            base_selector = rand();
        }
    }
    
    printf("Total: %lld\n", total);
    return 0;
}

/* test_mcf.c - Program to trigger MCF algorithm's special block printing */
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
    volatile int control = selector;
    int result = 0;
    
    /* Complex switch with irreducible control flow via goto */
    switch (control % 12) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_merge_1;
            
        case 1:
            g = h - i;
            j = k * 2;
            asm volatile("" : : : "esi", "edi");
            goto case_merge_2;
            
        case 2:
            l = m + n;
            o = a ^ b;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case_merge_3;
            
        case 3:
            p = q * 2.0f;
            r = p + 1.0f;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case_merge_1;
            
        case 4:
            a = c * d;
            b = e ^ f;
            asm volatile("" : : : "rax", "rbx");
            /* Fall through */
            
        case 5:
            g = h | i;
            j = k & l;
            asm volatile("" : : : "rcx", "rdx");
            goto case_merge_2;
            
        case 6:
            m = n << 1;
            o = a >> 2;
            asm volatile("" : : : "r11", "r12");
            goto loop_back;
            
        case 7:
            p = q / 2.0f;
            r = r * p;
            asm volatile("" : : : "xmm2", "xmm3");
            goto case_merge_3;
            
        case 8:
            a = b * c * d;
            e = f + g + h;
            asm volatile("" : : : "r13", "r14", "r15");
            goto case_merge_1;
            
        case 9:
            i = j * k;
            l = m - n;
            asm volatile("" : : : "mm0", "mm1");
            goto case_merge_2;
            
        case 10:
            o = a * b * c * d;
            asm volatile("" : : : "st", "st(1)");
            goto loop_back;
            
        case 11:
            p = q + r;
            a = (int)p;
            asm volatile("" : : : "xmm4", "xmm5", "xmm6");
            /* Fall through to default */
            
        default:
            b = c + d + e;
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            goto final_merge;
    }

loop_back:
    /* Create loop-like structure with goto */
    if (a < 100) {
        a += b;
        goto loop_back;
    }
    goto final_merge;

case_merge_1:
    f = g * h;
    i = j + k;
    goto intermediate;

case_merge_2:
    l = m * n;
    o = p > q ? 1 : 0;
    goto intermediate;

case_merge_3:
    a = b | c;
    d = e & f;
    /* Continue to intermediate */

intermediate:
    /* More operations creating additional basic blocks */
    if (g > 10) {
        h = i * 2;
        asm volatile("" : : : "r8", "r9");
    } else {
        h = i / 2;
        asm volatile("" : : : "r10", "r11");
    }
    
    /* Nested conditionals */
    for (int x = 0; x < 3; x++) {
        if (x % 2 == 0) {
            j += k;
            asm volatile("" : : : "r12");
        } else {
            j -= k;
            asm volatile("" : : : "r13");
        }
    }

final_merge:
    /* Combine all variables to prevent elimination */
    result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + (int)p + (int)q + (int)r;
    
    /* Final register clobbering */
    asm volatile("" : : : 
        "eax", "ebx", "ecx", "edx",
        "esi", "edi",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
    
    return result;
}

/* Another function to create more control flow complexity */
NOINLINE static int nested_control_flow(volatile int depth, volatile int seed) {
    int total = seed;
    
    if (depth > 0) {
        /* Recursive-like structure with goto instead of recursion */
        depth--;
        if (depth % 3 == 0) {
            total += register_pressure_function(seed + depth);
            goto branch_a;
        } else if (depth % 3 == 1) {
            total += register_pressure_function(seed - depth);
            goto branch_b;
        } else {
            total += register_pressure_function(seed * depth);
            goto branch_c;
        }
    }
    
    goto finish;

branch_a:
    total *= 2;
    if (depth > 1) {
        total += nested_control_flow(depth - 1, seed + 1);
    }
    goto finish;

branch_b:
    total /= 2;
    if (depth > 1) {
        total += nested_control_flow(depth - 1, seed - 1);
    }
    goto finish;

branch_c:
    total ^= 0xAAAA;
    if (depth > 1) {
        total += nested_control_flow(depth - 1, seed ^ 0x5555);
    }
    /* Fall through */

finish:
    return total;
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
        /* Vary selector to hit different switch cases */
        selector = rand() % 100;
        
        /* Call functions with complex control flow */
        int result1 = register_pressure_function(selector);
        int result2 = nested_control_flow((selector % 5) + 1, selector);
        
        total += result1 + result2;
        
        /* Occasionally add more pressure */
        if (i % 1000 == 0) {
            /* Extra complex call */
            for (int j = 0; j < 10; j++) {
                total += register_pressure_function(selector + j);
            }
        }
    }
    
    printf("Total result: %lld\n", total);
    printf("Test completed.\n");
    
    return 0;
}

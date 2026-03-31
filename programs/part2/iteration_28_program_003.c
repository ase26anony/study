/* test_mcf.c - Program to trigger MCF algorithm special block printing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force no inlining and no interprocedural optimizations */
__attribute__((noinline, noipa))
static int register_pressure_function(volatile int selector) {
    /* Declare many local variables of different types to increase register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    short f = 6, g = 7, h = 8, i = 9, j = 10;
    char k = 11, l = 12, m = 13, n = 14, o = 15;
    float p = 16.0f, q = 17.0f, r = 18.0f;
    volatile int result = 0;
    
    /* Complex irreducible control flow using goto */
    if (selector < 0) goto case_negative;
    
    /* Large switch statement creating many basic blocks */
    switch (selector % 10) {
        case 0:
            a = b + c;
            d = e * f;
            /* Clobber registers to increase pressure */
            asm volatile("" : : : "eax", "ebx", "ecx", "edx");
            goto case_merge;
            
        case 1:
            g = h - i;
            j = k * l;
            asm volatile("" : : : "esi", "edi");
            if (m > n) goto case_0_again;
            break;
            
        case 2:
            p = q + r;
            a = (int)p;
            asm volatile("" : : : "xmm0", "xmm1");
            goto case_3;
            
        case 3:
            b = c << 2;
            d = e >> 1;
            asm volatile("" : : : "eax", "ebx");
            /* Fall through */
            
        case 4:
            f = g & h;
            i = j | k;
            asm volatile("" : : : "ecx", "edx");
            break;
            
        case 5:
            l = m ^ n;
            o = ~o;
            asm volatile("" : : : "r8", "r9", "r10");
            goto case_6;
            
        case 6:
            a = b * c * d;
            asm volatile("" : : : "rax", "rbx");
            if (e > 100) goto case_7;
            break;
            
        case 7:
            p = q * r;
            asm volatile("" : : : "xmm2", "xmm3");
            /* Create loop within case */
            for (int x = 0; x < 3; x++) {
                a += x;
                asm volatile("" : : : "eax");
            }
            break;
            
        case 8:
            /* Nested conditionals */
            if (f > g) {
                if (h < i) {
                    j = k + l;
                } else {
                    j = k - l;
                }
                asm volatile("" : : : "r11", "r12");
            }
            goto case_9;
            
        case 9:
            m = n * o;
            asm volatile("" : : : "r13", "r14", "r15");
            break;
            
        default:
            a = b + c + d + e;
            asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
            break;
    }
    
    goto finish;
    
case_0_again:
    a = b + c;
    d++;
    goto case_merge;
    
case_negative:
    a = -b;
    c = -d;
    asm volatile("" : : : "eax", "ebx");
    goto finish;
    
case_merge:
    e = f + g;
    asm volatile("" : : : "ecx");
    /* Another goto to create irreducible flow */
    if (h > i) goto case_3;
    
case_3:
    j = k + 1;
    asm volatile("" : : : "edx");
    
finish:
    /* Combine all variables to prevent optimization */
    result = a + b + c + d + e + f + g + h + i + j + k + l + m + n + o + (int)p + (int)q + (int)r;
    
    /* More register clobbering */
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                 "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
    
    return result;
}

/* Another complex function to create more flow graph edges */
__attribute__((noinline, noipa))
static int secondary_pressure_function(int base) {
    int x1 = base, x2 = base + 1, x3 = base + 2;
    volatile int y = 0;
    
    /* Complex control flow with goto */
    if (base % 3 == 0) goto block_a;
    if (base % 3 == 1) goto block_b;
    
    x1 = x2 * x3;
    asm volatile("" : : : "eax", "ebx");
    goto block_c;
    
block_a:
    x2 = x1 - x3;
    asm volatile("" : : : "ecx", "edx");
    if (x2 > 0) goto block_c;
    
block_b:
    x3 = x1 + x2;
    asm volatile("" : : : "esi", "edi");
    
block_c:
    y = x1 + x2 + x3;
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    
    return y;
}

int main(int argc, char *argv[]) {
    int iterations = 100000;
    volatile long long total = 0;
    volatile int selector = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100000;
    }
    
    /* Initialize random seed */
    srand(time(NULL));
    
    printf("Starting MCF trigger test with %d iterations...\n", iterations);
    
    /* Hot loop calling high register pressure functions */
    for (int i = 0; i < iterations; i++) {
        /* Vary selector to exercise different control flow paths */
        selector = rand() % 20 - 5;  /* Range -5 to 14 */
        
        /* Call main pressure function */
        int result1 = register_pressure_function(selector);
        
        /* Call secondary function */
        int result2 = secondary_pressure_function(i);
        
        /* Accumulate results to prevent optimization */
        total += result1 + result2;
        
        /* Occasionally flush to prevent loop optimization */
        if (i % 10000 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Total: %lld\n", total);
    
    /* One more call with extreme values to hit edge cases */
    selector = -100;
    total += register_pressure_function(selector);
    
    selector = 1000;
    total += register_pressure_function(selector);
    
    printf("Final total: %lld\n", total);
    
    return 0;
}

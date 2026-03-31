/* Target: reorg.cc lines 2135-2149 - Delay slot filling logic */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for specific variables to avoid conflicts */
register int a asm("$2");
register int b asm("$3");
register int c asm("$4");
register int d asm("$5");
register int e asm("$6");
register int f asm("$7");
register int g asm("$8");
register int h asm("$9");

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

int main() {
    /* Initialize registers with values that will create varying branch outcomes */
    a = 0;
    b = 100;
    c = 0;
    d = 1;
    e = 2;
    f = 3;
    g = 4;
    h = 5;
    
    int result = 0;
    
    /* Main loop to force multiple delay slot filling attempts */
    for (int i = 0; i < iterations; i++) {
        /* Varying number of nops to create different filling scenarios */
        switch (i % 4) {
            case 0:
                /* Branch pattern 1: Single nop before label */
                if (__builtin_expect(a < b, 1)) {
                    asm volatile("nop" ::: "memory");
                    goto target_label1;
                }
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                target_label1:
                /* Candidate for delay slot: simple register operation */
                c = d + 1;  /* Independent of branch condition registers */
                break;
                
            case 1:
                /* Branch pattern 2: Two nops before label */
                if (__builtin_expect(e > f, 0)) {
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
                    goto target_label2;
                }
                asm volatile("nop" ::: "memory");
                target_label2:
                /* Another candidate: different registers */
                g = h + 2;  /* Uses registers $8 and $9, independent */
                break;
                
            case 2:
                /* Branch pattern 3: Three nops, different condition */
                if (__builtin_expect(a == i, 0)) {
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
                    asm volatile("nop" ::: "memory");
                    goto target_label3;
                }
                target_label3:
                /* Simple arithmetic with immediate - trap-free */
                d = e * 2;  /* Multiplication by 2 is safe shift */
                break;
                
            case 3:
                /* Branch pattern 4: No nops, predictable branch */
                if (__builtin_expect(b > 0, 1)) {
                    goto target_label4;
                }
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                target_label4:
                /* Register move operation */
                f = g;  /* Simple move, no resource conflicts */
                break;
        }
        
        /* Update variables to change branch outcomes */
        a++;
        b--;
        e = (e + 1) % 10;
        f = (f + 2) % 10;
        
        /* Use results to keep computations live */
        result += c + g + d + f;
    }
    
    /* Additional test cases with nested branches */
    {
        register int x asm("$10") = 0;
        register int y asm("$11") = 10;
        register int z asm("$12") = 0;
        
        for (int j = 0; j < 50; j++) {
            /* Conditional branch with immediate label following */
            if (__builtin_expect(x < y, 1)) {
                asm volatile("nop" ::: "memory");
                goto nested_target;
            }
            asm volatile("nop" ::: "memory");
            nested_target:
            /* Simple, safe operation after label */
            z = x + 5;  /* Immediate addition, no trap */
            
            x++;
            y--;
            result += z;
        }
    }
    
    /* Final computation to use all variables */
    int final_result = a + b + c + d + e + f + g + h + result;
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}

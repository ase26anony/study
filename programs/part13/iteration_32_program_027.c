#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Simple pseudo-random generator to vary conditions */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function to create side effects */
volatile int side_effect_counter = 0;
void create_side_effect(void) {
    side_effect_counter++;
    asm volatile("" ::: "memory");
}

int main(void) {
    /* Volatile variables to prevent constant folding */
    volatile double v1, v2, v3, v4, v5, v6, v7, v8;
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    
    /* Initialize with function calls to prevent optimization */
    v1 = nan_val;
    v2 = normal_val;
    v3 = inf_val;
    v4 = zero_val;
    v5 = -normal_val;
    v6 = -inf_val;
    v7 = __builtin_nan("0xdead");
    v8 = 2.71828;
    
    /* Mixed integer-FP variable */
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED comparisons (NaN involved) */
    asm volatile("" ::: "cc");  /* Clobber condition codes */
    if (__builtin_isunordered(v1, v2)) {
        sum += 1;
        create_side_effect();
    }
    
    /* Block 2: ORDERED comparisons (no NaN) */
    asm volatile("" ::: "cc");
    if (__builtin_isgreater(v2, v4)) {  /* 3.14159 > 0.0 */
        sum += 2;
        create_side_effect();
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    asm volatile("" ::: "cc");
    double uneq_test = (v1 == v1) ? 0.0 : 1.0;  /* Force UNEQ condition */
    if (!(v2 > v2) && !(v2 < v2)) {  /* UNEQ: not greater and not less */
        sum += 3;
        create_side_effect();
    }
    
    /* Block 4: UNGE (unordered or greater or equal) */
    asm volatile("" ::: "cc");
    if (!(v2 < v4)) {  /* nlt: not less than */
        sum += 4;
        create_side_effect();
    }
    
    /* Block 5: UNGT (unordered or greater) */
    asm volatile("" ::: "cc");
    if (!(v2 <= v4)) {  /* nle: not less or equal */
        sum += 5;
        create_side_effect();
    }
    
    /* Block 6: UNLE (unordered or less or equal) */
    asm volatile("" ::: "cc");
    if (v4 <= v2 || __builtin_isunordered(v4, v2)) {
        sum += 6;
        create_side_effect();
    }
    
    /* Block 7: UNLT (unordered or less) */
    asm volatile("" ::: "cc");
    if (v4 < v2 || __builtin_isunordered(v4, v2)) {
        sum += 7;
        create_side_effect();
    }
    
    /* Block 8: LTGT (less or greater, but not equal and not unordered) */
    asm volatile("" ::: "cc");
    if ((v2 < v8) != (v2 > v8) && !__builtin_isunordered(v2, v8)) {
        sum += 8;
        create_side_effect();
    }
    
    /* Ternary operators with different conditions */
    asm volatile("" ::: "cc");
    sum += (v1 != v1) ? 10 : 20;  /* NaN != NaN is true (unordered) */
    
    asm volatile("" ::: "cc");
    sum += (v2 == v2) ? 30 : 40;  /* Ordered equality */
    
    /* Mixed integer-FP comparison */
    asm volatile("" ::: "cc");
    if ((double)(int)volatile_int > v4) {
        sum += 9;
        create_side_effect();
    }
    
    /* Switch statement with floating comparisons */
    volatile int selector = 3;
    switch (selector) {
        case 1:
            if (v1 < v2) sum += 100;  /* Unordered comparison */
            break;
        case 2:
            if (v2 > v4) sum += 200;  /* Ordered comparison */
            break;
        case 3:
            if (v2 != v8) sum += 300;  /* LTGT potentially */
            break;
        default:
            if (__builtin_isunordered(v7, v5)) sum += 400;
            break;
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    volatile double loop_values[8];
    for (int i = 0; i < 8; i++) {
        loop_values[i] = (i & 1) ? normal_val + i : nan_val;
    }
    
    for (int i = 0; i < 100; i++) {
        uint32_t r = lcg_rand();
        volatile double a = loop_values[r % 8];
        volatile double b = loop_values[(r >> 3) % 8];
        
        /* Different comparisons based on hash of index */
        switch (r % 8) {
            case 0:  /* UNORDERED */
                if (__builtin_isunordered(a, b)) sum += 1;
                break;
            case 1:  /* ORDERED */
                if (!__builtin_isunordered(a, b)) sum += 2;
                break;
            case 2:  /* UNEQ */
                if (!(a > b) && !(a < b)) sum += 3;
                break;
            case 3:  /* UNGE */
                if (!(a < b)) sum += 4;
                break;
            case 4:  /* UNGT */
                if (!(a <= b)) sum += 5;
                break;
            case 5:  /* UNLE */
                if (a <= b || __builtin_isunordered(a, b)) sum += 6;
                break;
            case 6:  /* UNLT */
                if (a < b || __builtin_isunordered(a, b)) sum += 7;
                break;
            case 7:  /* LTGT */
                if ((a < b) != (a > b) && !__builtin_isunordered(a, b)) sum += 8;
                break;
        }
        
        /* Inline assembly clobbering FP status register */
        asm volatile("fwait" ::: "cc");
    }
    
    /* Final output to ensure all code executes */
    printf("Final sum: %d\n", sum);
    printf("Side effects: %d\n", side_effect_counter);
    
    return 0;
}

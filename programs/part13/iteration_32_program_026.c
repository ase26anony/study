#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
volatile double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 0.0, -1.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 42;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile double neg_val = -2.71828;
    
    volatile int volatile_int = 7;
    
    /* Block 1: UNORDERED comparisons */
    {
        asm volatile ("" : : : "st");
        if (__builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1;
        }
        if (nan_val != nan_val) { /* NaN != NaN is true */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED comparisons */
    {
        asm volatile ("" : : : "st");
        if (__builtin_isordered(normal_val, zero_val)) {
            global_sum += 4;
        }
        if (!__builtin_isunordered(inf_val, neg_val)) {
            global_sum += 8;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        asm volatile ("" : : : "st");
        double a = get_value(0);
        double b = get_value(0);
        if (a == b || __builtin_isunordered(a, b)) {
            global_sum += 16;
        }
        /* Using ternary operator */
        global_sum += (nan_val == nan_val || __builtin_isunordered(nan_val, nan_val)) ? 32 : 0;
    }
    
    /* Block 4: UNGE (unordered or not less than) */
    {
        asm volatile ("" : : : "st");
        if (!__builtin_isless(normal_val, nan_val)) {
            global_sum += 64;
        }
        /* Equivalent to !(a < b) including unordered */
        double a = get_value(1);
        double b = get_value(2);
        if (a >= b || __builtin_isunordered(a, b)) {
            global_sum += 128;
        }
    }
    
    /* Block 5: UNGT (unordered or greater than) */
    {
        asm volatile ("" : : : "st");
        if (!__builtin_islessequal(normal_val, nan_val)) {
            global_sum += 256;
        }
        if (__builtin_isgreater(nan_val, normal_val) || __builtin_isunordered(nan_val, normal_val)) {
            global_sum += 512;
        }
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        asm volatile ("" : : : "st");
        if (__builtin_islessequal(nan_val, normal_val) || __builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1024;
        }
        double a = get_value(3);
        double b = get_value(4);
        if (a <= b || __builtin_isunordered(a, b)) {
            global_sum += 2048;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        asm volatile ("" : : : "st");
        if (__builtin_isless(nan_val, normal_val) || __builtin_isunordered(nan_val, normal_val)) {
            global_sum += 4096;
        }
        if (!__builtin_isgreaterequal(zero_val, nan_val)) {
            global_sum += 8192;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, but not equal and not unordered) */
    {
        asm volatile ("" : : : "st");
        double a = get_value(0);
        double b = get_value(1);
        if ((a < b || a > b) && !__builtin_isunordered(a, b)) {
            global_sum += 16384;
        }
        /* Using switch statement */
        switch ((a != b && !__builtin_isunordered(a, b)) ? 1 : 0) {
            case 1: global_sum += 32768; break;
            default: break;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        asm volatile ("" : : : "st");
        volatile int vi = volatile_int;
        double converted = (double)(int)vi;
        if (converted > 5.0 || __builtin_isunordered(converted, 5.0)) {
            global_sum += 65536;
        }
        if ((double)(int)volatile_int < 10.0 && !__builtin_isunordered((double)(int)volatile_int, 10.0)) {
            global_sum += 131072;
        }
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        volatile double arr[10];
        for (int i = 0; i < 10; i++) {
            arr[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            double a = arr[i % 10];
            double b = arr[(i + 1) % 10];
            
            /* Select condition based on hash of iteration */
            switch (r % 8) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1: /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2: /* UNEQ */
                    if (a == b || __builtin_isunordered(a, b)) global_sum += 3;
                    break;
                case 3: /* UNGE */
                    if (!__builtin_isless(a, b)) global_sum += 4;
                    break;
                case 4: /* UNGT */
                    if (!__builtin_islessequal(a, b)) global_sum += 5;
                    break;
                case 5: /* UNLE */
                    if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) global_sum += 6;
                    break;
                case 6: /* UNLT */
                    if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) global_sum += 7;
                    break;
                case 7: /* LTGT */
                    if ((a < b || a > b) && !__builtin_isunordered(a, b)) global_sum += 8;
                    break;
            }
            
            /* Insert inline assembly to clobber FP status */
            asm volatile ("" : : : "st");
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
volatile double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 0.0, -1.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 42;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile double neg_val = -2.71828;
    
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED comparisons */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        if (__builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1;
        }
        
        /* Using explicit operators with NaN */
        double temp = nan_val;
        if (temp != temp) {  /* NaN != NaN is true */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED comparisons */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        if (__builtin_isordered(normal_val, zero_val)) {
            global_sum += 4;
        }
        
        /* Ordered comparison using builtin */
        int result = __builtin_isgreater(normal_val, zero_val) && 
                     __builtin_isordered(normal_val, zero_val);
        global_sum += result ? 8 : 0;
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* UNEQ: unordered OR equal */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            global_sum += 16;
        }
        
        /* Another UNEQ pattern */
        if (a == b || __builtin_isunordered(a, b)) {
            global_sum += 32;
        }
    }
    
    /* Block 4: UNGE (unordered or greater-or-equal) */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        volatile double a = get_value(0);
        volatile double b = get_value(1);
        
        /* Using builtins for UNGE */
        if (__builtin_isunordered(a, b) || a >= b) {
            global_sum += 64;
        }
        
        /* Alternative: not less than */
        if (!(a < b)) {
            global_sum += 128;
        }
    }
    
    /* Block 5: UNGT (unordered or greater) */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        volatile double a = get_value(2);
        volatile double b = get_value(3);
        
        if (__builtin_isunordered(a, b) || a > b) {
            global_sum += 256;
        }
        
        /* Alternative: not less-or-equal */
        if (!(a <= b)) {
            global_sum += 512;
        }
    }
    
    /* Block 6: UNLE (unordered or less-or-equal) */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        volatile double a = get_value(3);
        volatile double b = get_value(4);
        
        if (__builtin_isunordered(a, b) || a <= b) {
            global_sum += 1024;
        }
        
        /* Using ternary operator */
        int val = (a <= b || __builtin_isunordered(a, b)) ? 2048 : 0;
        global_sum += val;
    }
    
    /* Block 7: UNLT (unordered or less) */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        volatile double a = get_value(4);
        volatile double b = get_value(0);
        
        if (__builtin_isunordered(a, b) || a < b) {
            global_sum += 4096;
        }
        
        /* Alternative: not greater-or-equal */
        if (!(a >= b)) {
            global_sum += 8192;
        }
    }
    
    /* Block 8: LTGT (less or greater, but not equal and not unordered) */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        volatile double a = normal_val;
        volatile double b = zero_val;
        
        /* LTGT: (a < b) || (a > b) but both ordered */
        if ((a < b && __builtin_isordered(a, b)) || 
            (a > b && __builtin_isordered(a, b))) {
            global_sum += 16384;
        }
        
        /* Alternative: not equal and ordered */
        if (a != b && __builtin_isordered(a, b)) {
            global_sum += 32768;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        volatile int vi = volatile_int;
        double converted = (double)vi;
        
        if (converted > normal_val) {
            global_sum += 65536;
        }
        
        if ((double)(int)vi < zero_val) {
            global_sum += 131072;
        }
    }
    
    /* Loop with varying conditions */
    {
        volatile double arr[10];
        for (int i = 0; i < 10; i++) {
            arr[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = arr[r % 10];
            volatile double b = arr[(r >> 8) % 10];
            
            /* Switch based on hash to generate different condition codes */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2:  /* UNEQ */
                    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) 
                        global_sum += 4;
                    break;
                case 3:  /* UNGE */
                    if (!(a < b)) global_sum += 8;
                    break;
                case 4:  /* UNGT */
                    if (!(a <= b)) global_sum += 16;
                    break;
                case 5:  /* UNLE */
                    if (!(a > b)) global_sum += 32;
                    break;
                case 6:  /* UNLT */
                    if (!(a >= b)) global_sum += 64;
                    break;
                case 7:  /* LTGT */
                    if (a != b && __builtin_isordered(a, b)) global_sum += 128;
                    break;
            }
            
            /* Additional inline assembly to clobber FPU state */
            asm volatile ("" ::: "cc", "st", "st(1)", "st(2)", "st(3)", 
                                     "st(4)", "st(5)", "st(6)", "st(7)");
        }
    }
    
    /* Complex nested comparisons */
    {
        volatile double x = nan_val;
        volatile double y = normal_val;
        volatile double z = zero_val;
        
        /* Complex expression that might generate multiple condition codes */
        int complex_result = (x != y) ? 
                            ((y > z) ? 1 : (__builtin_isunordered(y, z) ? 2 : 3)) :
                            (__builtin_isordered(x, z) ? 4 : 5);
        global_sum += complex_result;
        
        /* Switch statement with FP comparisons */
        switch ((int)(y > z ? 1 : (y < z ? 2 : 3))) {
            case 1:
                global_sum += 1000;
                break;
            case 2:
                global_sum += 2000;
                break;
            case 3:
                if (__builtin_isunordered(y, z)) {
                    global_sum += 3000;
                } else {
                    global_sum += 4000;
                }
                break;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return 0;
}

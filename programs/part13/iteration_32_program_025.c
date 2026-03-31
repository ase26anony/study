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
static uint32_t lcg_state = 1;
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
    
    /* Block 1: UNORDERED - comparison with NaN */
    {
        volatile double a = nan_val;
        volatile double b = get_value(0);
        
        /* Use __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Also use explicit comparison with NaN */
        asm volatile ("" ::: "cc", "memory");  /* Clobber flags */
        if (a != a || b != b) {  /* NaN check */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED - both operands are not NaN */
    {
        volatile double a = normal_val;
        volatile double b = get_value(1);
        
        /* Use __builtin_isordered */
        if (__builtin_isordered(a, b)) {
            global_sum += 4;
        }
        
        /* Ternary operator with ordered check */
        asm volatile ("" ::: "cc", "memory");
        int result = (a == a && b == b) ? 8 : 0;
        global_sum += result;
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* Two NaNs are unordered equal */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            global_sum += 16;
        }
        
        /* Mixed integer-FP comparison */
        asm volatile ("" ::: "cc", "memory");
        double converted = (double)(int)volatile_int;
        if (!(converted > normal_val) && !(converted < normal_val)) {
            global_sum += 32;
        }
    }
    
    /* Block 4: UNGE - not less than (unordered, greater, or equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Use !__builtin_isless */
        if (!__builtin_isless(a, b)) {
            global_sum += 64;
        }
        
        /* Explicit operator version */
        asm volatile ("" ::: "cc", "memory");
        if (!(a < b)) {
            global_sum += 128;
        }
    }
    
    /* Block 5: UNGT - not less than or equal (unordered or greater) */
    {
        volatile double a = inf_val;
        volatile double b = normal_val;
        
        if (!__builtin_islessequal(a, b)) {
            global_sum += 256;
        }
        
        asm volatile ("" ::: "cc", "memory");
        if (!(a <= b)) {
            global_sum += 512;
        }
    }
    
    /* Block 6: UNLE - unordered or less than or equal */
    {
        volatile double a = nan_val;
        volatile double b = inf_val;
        
        if (!__builtin_isgreater(a, b)) {
            global_sum += 1024;
        }
        
        asm volatile ("" ::: "cc", "memory");
        if (!(a > b)) {
            global_sum += 2048;
        }
    }
    
    /* Block 7: UNLT - unordered or less than */
    {
        volatile double a = neg_val;
        volatile double b = nan_val;
        
        if (!__builtin_isgreaterequal(a, b)) {
            global_sum += 4096;
        }
        
        asm volatile ("" ::: "cc", "memory");
        if (!(a >= b)) {
            global_sum += 8192;
        }
    }
    
    /* Block 8: LTGT - less than or greater than (ordered and not equal) */
    {
        volatile double a = normal_val;
        volatile double b = get_value(2);
        
        /* Use both __builtin_isgreater and __builtin_isless */
        if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) {
            global_sum += 16384;
        }
        
        /* Explicit != with ordered values */
        asm volatile ("" ::: "cc", "memory");
        if (a != b && a == a && b == b) {
            global_sum += 32768;
        }
    }
    
    /* Loop with varying conditions */
    volatile double array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = get_value(i);
    }
    
    for (int i = 0; i < 100; i++) {
        uint32_t r = lcg_rand();
        volatile double a = array[r % 8];
        volatile double b = array[(r >> 3) % 8];
        
        /* Switch based on hash of index to generate different condition codes */
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
                if (!__builtin_isless(a, b)) global_sum += 8;
                break;
            case 4:  /* UNGT */
                if (!__builtin_islessequal(a, b)) global_sum += 16;
                break;
            case 5:  /* UNLE */
                if (!__builtin_isgreater(a, b)) global_sum += 32;
                break;
            case 6:  /* UNLT */
                if (!__builtin_isgreaterequal(a, b)) global_sum += 64;
                break;
            case 7:  /* LTGT */
                if (__builtin_isgreater(a, b) || __builtin_isless(a, b)) 
                    global_sum += 128;
                break;
        }
        
        /* Mixed integer-FP comparison in loop */
        double mixed = (double)(int)(r % 100);
        asm volatile ("" ::: "cc", "memory");  /* Clobber FP status */
        if (mixed > a) {
            global_sum += 256;
        }
    }
    
    /* Final output to ensure all code executes */
    printf("Final sum: %d\n", global_sum);
    
    return 0;
}

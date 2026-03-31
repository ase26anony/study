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

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 1;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double zero = 0.0;
    volatile double one = 1.0;
    volatile double two = 2.0;
    volatile double neg_one = -1.0;
    
    volatile int volatile_int = 42;
    
    /* Force compiler to generate condition codes for each case */
    
    /* 1. UNORDERED - comparison with NaN */
    {
        asm volatile ("" ::: "cc");  /* Clobber condition codes */
        if (nan_val != nan_val) {    /* Always true for NaN */
            global_sum += 1;
        }
        if (__builtin_isunordered(nan_val, one)) {
            global_sum += 2;
        }
    }
    
    /* 2. ORDERED - both operands are not NaN */
    {
        asm volatile ("" ::: "cc");
        if (__builtin_isordered(one, two)) {
            global_sum += 4;
        }
        if (!__builtin_isunordered(one, two)) {
            global_sum += 8;
        }
    }
    
    /* 3. UNEQ - unordered or equal */
    {
        asm volatile ("" ::: "cc");
        volatile double a = get_value(0);
        volatile double b = get_value(0);
        if (!(a != b)) {  /* Forces UNEQ condition */
            global_sum += 16;
        }
    }
    
    /* 4. UNGE - unordered or greater-or-equal */
    {
        asm volatile ("" ::: "cc");
        volatile double a = get_value(1);
        volatile double b = get_value(2);
        if (!(a < b)) {  /* nlt = not less than = UNGE */
            global_sum += 32;
        }
    }
    
    /* 5. UNGT - unordered or greater-than */
    {
        asm volatile ("" ::: "cc");
        volatile double a = get_value(2);
        volatile double b = get_value(1);
        if (!(a <= b)) {  /* nle = not less-or-equal = UNGT */
            global_sum += 64;
        }
    }
    
    /* 6. UNLE - unordered or less-or-equal */
    {
        asm volatile ("" ::: "cc");
        volatile double a = get_value(1);
        volatile double b = get_value(2);
        if (a <= b || __builtin_isunordered(a, b)) {
            global_sum += 128;
        }
    }
    
    /* 7. UNLT - unordered or less-than */
    {
        asm volatile ("" ::: "cc");
        volatile double a = get_value(0);
        volatile double b = get_value(1);
        if (a < b || __builtin_isunordered(a, b)) {
            global_sum += 256;
        }
    }
    
    /* 8. LTGT - less-than or greater-than (ordered and not equal) */
    {
        asm volatile ("" ::: "cc");
        volatile double a = get_value(1);
        volatile double b = get_value(2);
        if (a != b && __builtin_isordered(a, b)) {
            global_sum += 512;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        asm volatile ("" ::: "cc");
        double converted = (double)(int)volatile_int;
        if (converted > 10.0) {
            global_sum += 1024;
        }
        if (converted < 100.0) {
            global_sum += 2048;
        }
    }
    
    /* Loop with varying conditions */
    {
        double arr[10];
        for (int i = 0; i < 10; i++) {
            arr[i] = (double)i + 0.5;
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = arr[i % 10];
            volatile double b = arr[(i + 1) % 10];
            
            /* Use switch to force different condition codes */
            switch (r % 8) {
                case 0:
                    if (__builtin_isunordered(a, b)) global_sum += 1;
                    break;
                case 1:
                    if (__builtin_isordered(a, b)) global_sum += 2;
                    break;
                case 2:
                    if (!(a != b)) global_sum += 3;  /* UNEQ */
                    break;
                case 3:
                    if (!(a < b)) global_sum += 4;   /* UNGE */
                    break;
                case 4:
                    if (!(a <= b)) global_sum += 5;  /* UNGT */
                    break;
                case 5:
                    if (a <= b || __builtin_isunordered(a, b)) 
                        global_sum += 6;  /* UNLE */
                    break;
                case 6:
                    if (a < b || __builtin_isunordered(a, b))
                        global_sum += 7;  /* UNLT */
                    break;
                case 7:
                    if (a != b && __builtin_isordered(a, b))
                        global_sum += 8;  /* LTGT */
                    break;
            }
            
            /* Ternary operator with FP comparison */
            global_sum += (a > b) ? 1 : 0;
            global_sum += (a <= b) ? 2 : 0;
            
            /* Complex expression */
            if ((a > b) && (b < a) && !__builtin_isunordered(a, b)) {
                global_sum += 9;
            }
        }
    }
    
    /* Final check with NaN to ensure all paths are taken */
    {
        volatile double a = get_value(0);
        volatile double b = nan_val;
        
        /* This should generate UNORDERED condition */
        if (a == b) {
            global_sum += 4096;
        }
        
        /* This should generate ORDERED condition */
        if (a == a) {
            global_sum += 8192;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}

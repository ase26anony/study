#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Simple pseudo-random generator for varying conditions */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function with side effects to prevent constant folding */
volatile double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    return values[idx % 5];
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double zero_val = 0.0;
    volatile double one_val = 1.0;
    volatile double two_val = 2.0;
    
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED - comparison with NaN */
    {
        double a = nan_val;
        double b = get_value(0);
        
        /* Use __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Also use explicit comparison with NaN */
        asm volatile ("" ::: "cc");
        if (a != a || b != b) {  /* NaN check */
            global_sum += 1;
        }
    }
    
    /* Block 2: ORDERED - both operands are not NaN */
    {
        double a = one_val;
        double b = two_val;
        
        /* Use __builtin_isordered */
        if (__builtin_isordered(a, b)) {
            global_sum += 2;
        }
        
        /* Ternary operator with ordered comparison */
        global_sum += (a == a && b == b) ? 1 : 0;
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        double a = nan_val;
        double b = nan_val;
        
        /* Two NaNs are unordered but also "equal" in UNEQ sense */
        if (!(a < b) && !(a > b)) {  /* UNEQ: !(a < b) && !(a > b) */
            global_sum += 3;
        }
        
        /* Mix with integer conversion */
        double c = (double)(int)volatile_int;
        if (!(c < one_val) && !(c > one_val)) {
            global_sum += 1;
        }
    }
    
    /* Block 4: UNGE - not less than (unordered or greater or equal) */
    {
        double a = nan_val;
        double b = one_val;
        
        if (!(a < b)) {  /* UNGE: !(a < b) */
            global_sum += 4;
        }
        
        /* Using __builtin_isgreaterequal */
        asm volatile ("" ::: "cc");
        if (__builtin_isgreaterequal(a, b)) {
            global_sum += 1;
        }
    }
    
    /* Block 5: UNGT - not less than or equal (unordered or greater) */
    {
        double a = nan_val;
        double b = two_val;
        
        if (!(a <= b)) {  /* UNGT: !(a <= b) */
            global_sum += 5;
        }
        
        /* Using __builtin_isgreater */
        if (__builtin_isgreater(a, b)) {
            global_sum += 1;
        }
    }
    
    /* Block 6: UNLE - unordered or less or equal */
    {
        double a = nan_val;
        double b = one_val;
        
        if (!(a > b)) {  /* UNLE: !(a > b) */
            global_sum += 6;
        }
        
        /* Using __builtin_islessequal */
        asm volatile ("" ::: "cc");
        if (__builtin_islessequal(a, b)) {
            global_sum += 1;
        }
    }
    
    /* Block 7: UNLT - unordered or less than */
    {
        double a = nan_val;
        double b = two_val;
        
        if (!(a >= b)) {  /* UNLT: !(a >= b) */
            global_sum += 7;
        }
        
        /* Using __builtin_isless */
        if (__builtin_isless(a, b)) {
            global_sum += 1;
        }
    }
    
    /* Block 8: LTGT - less than or greater than (ordered and not equal) */
    {
        double a = one_val;
        double b = two_val;
        
        if ((a < b) || (a > b)) {  /* LTGT: (a < b) || (a > b) */
            global_sum += 8;
        }
        
        /* Different values, both ordered */
        if (one_val != two_val) {
            global_sum += 1;
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
            double a = arr[r % 10];
            double b = arr[(r >> 8) % 10];
            
            /* Switch on hash of index to select different comparisons */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(a, b)) global_sum++;
                    break;
                case 2:  /* UNEQ */
                    if (!(a < b) && !(a > b)) global_sum++;
                    break;
                case 3:  /* UNGE */
                    if (!(a < b)) global_sum++;
                    break;
                case 4:  /* UNGT */
                    if (!(a <= b)) global_sum++;
                    break;
                case 5:  /* UNLE */
                    if (!(a > b)) global_sum++;
                    break;
                case 6:  /* UNLT */
                    if (!(a >= b)) global_sum++;
                    break;
                case 7:  /* LTGT */
                    if ((a < b) || (a > b)) global_sum++;
                    break;
            }
            
            /* Inline assembly to clobber FP status register */
            asm volatile ("" ::: "cc");
            
            /* Mixed integer-FP comparison */
            double c = (double)(int)(r % 100);
            if (c != a) {
                global_sum += (c > b) ? 1 : 0;
            }
        }
    }
    
    /* Additional complex expression mixing multiple conditions */
    {
        volatile double x = nan_val;
        volatile double y = one_val;
        volatile double z = two_val;
        
        /* Complex conditional expression */
        int result = (__builtin_isunordered(x, y) ? 1 : 0) +
                    (__builtin_isgreater(y, z) ? 2 : 0) +
                    (!(x <= y) ? 4 : 0) +
                    ((y < z) || (y > z) ? 8 : 0);
        
        global_sum += result;
        
        /* Nested conditionals */
        if (__builtin_isordered(x, y)) {
            if (y != z) {
                global_sum += 16;
            }
        } else {
            if (!(x >= y)) {
                global_sum += 32;
            }
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}

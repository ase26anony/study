#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int sum = 0;

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 123456789;
static uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Function with side effects to prevent constant folding */
volatile double get_value(int idx) {
    asm volatile("" : : : "memory");
    return (idx % 3 == 0) ? 3.14159 : 
           (idx % 3 == 1) ? 2.71828 : 1.41421;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 42.0;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 7;
    
    /* Force FPU status register clobbering */
    asm volatile("fwait" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)");
    
    /* Block 1: UNORDERED comparisons */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        /* Using builtin for unordered check */
        if (__builtin_isunordered(a, b)) {
            sum += 1;
        }
        
        /* Direct comparison with NaN */
        if (a != a) {  /* NaN != NaN is true */
            sum += 2;
        }
        
        /* Ternary operator with unordered */
        sum += __builtin_isunordered(a, b) ? 3 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        volatile double a = normal_val;
        volatile double b = get_value(1);
        
        if (!__builtin_isunordered(a, b)) {
            sum += 4;
        }
        
        /* Ordered check via builtin */
        sum += (a == a && b == b) ? 5 : 0;
    }
    
    /* Block 3: UNEQ (Unordered or Equal) */
    {
        volatile double a = nan_val;
        volatile double b = nan_val;
        
        /* Two NaNs are unordered equal */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            sum += 6;
        }
        
        /* Using explicit operators */
        sum += (a != a || b != b || a == b) ? 7 : 0;
    }
    
    /* Block 4: UNGE (Unordered or Greater or Equal) */
    {
        volatile double a = nan_val;
        volatile double b = normal_val;
        
        if (!__builtin_isless(a, b)) {
            sum += 8;
        }
        
        /* nlt = not less than */
        sum += (a >= b || a != a || b != b) ? 9 : 0;
    }
    
    /* Block 5: UNGT (Unordered or Greater Than) */
    {
        volatile double a = inf_val;
        volatile double b = normal_val;
        
        if (!__builtin_islessequal(a, b)) {
            sum += 10;
        }
        
        /* nle = not less than or equal */
        sum += (a > b || a != a || b != b) ? 11 : 0;
    }
    
    /* Block 6: UNLE (Unordered or Less or Equal) */
    {
        volatile double a = -inf_val;
        volatile double b = normal_val;
        
        if (!__builtin_isgreater(a, b)) {
            sum += 12;
        }
        
        /* ule = unordered or less or equal */
        sum += (a <= b || a != a || b != b) ? 13 : 0;
    }
    
    /* Block 7: UNLT (Unordered or Less Than) */
    {
        volatile double a = -inf_val;
        volatile double b = normal_val;
        
        if (!__builtin_isgreaterequal(a, b)) {
            sum += 14;
        }
        
        /* ult = unordered or less than */
        sum += (a < b || a != a || b != b) ? 15 : 0;
    }
    
    /* Block 8: LTGT (Less Than or Greater Than) */
    {
        volatile double a = normal_val;
        volatile double b = get_value(2);
        
        if (__builtin_islessgreater(a, b)) {
            sum += 16;
        }
        
        /* une = unordered or not equal */
        sum += (a != b) ? 17 : 0;
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vi = volatile_int;
        volatile double a = (double)(int)vi;
        volatile double b = get_value(3);
        
        /* This can trigger unusual condition code requirements */
        if (a < b) sum += 18;
        if (a > b) sum += 19;
        if (a <= b) sum += 20;
        if (a >= b) sum += 21;
        if (a == b) sum += 22;
        if (a != b) sum += 23;
    }
    
    /* Loop with varying conditions */
    {
        volatile double arr[8];
        for (int i = 0; i < 8; i++) {
            arr[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = arr[r % 8];
            volatile double b = arr[(r >> 3) % 8];
            
            /* Switch based on hash of index to diversify condition codes */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) sum++;
                    break;
                case 1:  /* ORDERED */
                    if (!__builtin_isunordered(a, b)) sum++;
                    break;
                case 2:  /* UNEQ */
                    if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) sum++;
                    break;
                case 3:  /* UNGE */
                    if (!__builtin_isless(a, b)) sum++;
                    break;
                case 4:  /* UNGT */
                    if (!__builtin_islessequal(a, b)) sum++;
                    break;
                case 5:  /* UNLE */
                    if (!__builtin_isgreater(a, b)) sum++;
                    break;
                case 6:  /* UNLT */
                    if (!__builtin_isgreaterequal(a, b)) sum++;
                    break;
                case 7:  /* LTGT */
                    if (__builtin_islessgreater(a, b)) sum++;
                    break;
            }
            
            /* Clobber FPU status register periodically */
            if (i % 13 == 0) {
                asm volatile("fwait" : : : "st", "st(1)", "st(2)", "st(3)");
            }
        }
    }
    
    /* Additional complex expression to force condition code usage */
    {
        volatile double x = get_value(0);
        volatile double y = get_value(1);
        volatile double z = get_value(2);
        
        /* Complex conditional expression */
        int result = (x < y) ? 1 : 
                    (__builtin_isunordered(x, z)) ? 2 :
                    (y > z) ? 3 : 
                    (x == y) ? 4 : 5;
        sum += result;
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}

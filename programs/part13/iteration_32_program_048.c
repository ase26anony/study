#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Function to prevent constant folding */
volatile double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 0.0, -1.0, 3.14159};
    return values[idx % 5];
}

/* Function returning NaN */
volatile double get_nan(void) {
    return __builtin_nan("");
}

/* Function returning potentially NaN */
volatile double maybe_nan(int x) {
    return (x & 1) ? __builtin_nan("") : (double)x;
}

/* Simple LCG for pseudo-random sequence */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double a, b, c, d;
    volatile int vi = 7;
    volatile float f1, f2;
    
    /* Initialize with NaN values */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("");
    volatile double inf = __builtin_inf();
    
    /* Block 1: UNORDERED comparisons */
    {
        a = nan1;
        b = 1.0;
        /* Use __builtin_isunordered */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
        
        /* Direct comparison with NaN */
        f1 = (float)nan1;
        f2 = 2.0f;
        if (f1 != f1 || f2 != f2) {  /* Check for NaN */
            global_counter++;
        }
        
        /* Ternary operator with unordered */
        global_sum += __builtin_isunordered(get_nan(), get_value(0)) ? 1 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        a = 3.14;
        b = 2.71;
        /* Use __builtin_isordered */
        if (__builtin_isordered(a, b)) {
            global_sum += 2;
        }
        
        /* Check both are not NaN */
        if (!__builtin_isunordered(a, b)) {
            global_counter += 2;
        }
        
        /* Inline assembly clobbering FP status */
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    }
    
    /* Block 3: UNEQ (Unordered or Equal) */
    {
        a = nan1;
        b = nan2;
        /* Two NaNs are unordered but equal in the UNEQ sense */
        if (!__builtin_isgreater(a, b) && !__builtin_isless(a, b)) {
            global_sum += 3;
        }
        
        /* Using == with NaN (always false for ordered, but UNEQ handles unordered) */
        c = get_value(1);
        d = get_value(1);
        if (__builtin_isunordered(c, d) || c == d) {
            global_counter += 3;
        }
    }
    
    /* Block 4: UNGE (Unordered or Greater or Equal) */
    {
        a = nan1;
        b = 5.0;
        /* !(a < b) covers UNGE: not less than (unordered or greater or equal) */
        if (!__builtin_isless(a, b)) {
            global_sum += 4;
        }
        
        /* Using >= with potential NaN */
        if (!(maybe_nan(vi) < get_value(2))) {
            global_counter += 4;
        }
    }
    
    /* Block 5: UNGT (Unordered or Greater Than) */
    {
        a = nan1;
        b = 10.0;
        /* !(a <= b) covers UNGT: not less than or equal */
        if (!__builtin_islessequal(a, b)) {
            global_sum += 5;
        }
        
        /* Using > with potential NaN */
        if (!(maybe_nan(vi+1) <= get_value(3))) {
            global_counter += 5;
        }
    }
    
    /* Block 6: UNLE (Unordered or Less or Equal) */
    {
        a = 2.0;
        b = nan1;
        /* !(a > b) covers UNLE: not greater than */
        if (!__builtin_isgreater(a, b)) {
            global_sum += 6;
        }
        
        /* Using <= with potential NaN */
        if (!(get_value(4) > maybe_nan(vi+2))) {
            global_counter += 6;
        }
    }
    
    /* Block 7: UNLT (Unordered or Less Than) */
    {
        a = 1.0;
        b = nan1;
        /* !(a >= b) covers UNLT: not greater than or equal */
        if (!__builtin_isgreaterequal(a, b)) {
            global_sum += 7;
        }
        
        /* Using < with potential NaN */
        if (!(get_value(0) >= maybe_nan(vi+3))) {
            global_counter += 7;
        }
    }
    
    /* Block 8: LTGT (Less Than or Greater Than, but not Equal and not Unordered) */
    {
        a = 3.0;
        b = 4.0;
        /* (a < b) || (a > b) but not equal and not unordered */
        if (__builtin_isless(a, b) || __builtin_isgreater(a, b)) {
            global_sum += 8;
        }
        
        /* Using != for ordered comparison */
        if (get_value(1) != get_value(2)) {
            global_counter += 8;
        }
        
        /* Another inline assembly clobber */
        asm volatile ("" : : : "cc", "memory");
    }
    
    /* Mixed integer-FP comparisons */
    {
        volatile int vx = 42;
        volatile double dx = (double)(int)vx;
        
        if (dx > 3.14) {
            global_sum += 9;
        }
        
        if ((double)(int)vi < get_value(3)) {
            global_counter += 9;
        }
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        volatile double arr[20];
        volatile int i;
        
        /* Initialize array with mixed values */
        for (i = 0; i < 20; i++) {
            arr[i] = (i % 3 == 0) ? maybe_nan(i) : get_value(i);
        }
        
        for (i = 0; i < 20; i++) {
            uint32_t r = lcg_rand();
            double x = arr[i];
            double y = arr[(i + 1) % 20];
            
            /* Switch based on hash of index to use different comparisons */
            switch (r % 8) {
                case 0:
                    /* UNORDERED */
                    if (__builtin_isunordered(x, y)) global_sum += 1;
                    break;
                case 1:
                    /* ORDERED */
                    if (__builtin_isordered(x, y)) global_sum += 2;
                    break;
                case 2:
                    /* UNEQ: !(x > y) && !(x < y) */
                    if (!__builtin_isgreater(x, y) && !__builtin_isless(x, y)) global_sum += 3;
                    break;
                case 3:
                    /* UNGE: !(x < y) */
                    if (!__builtin_isless(x, y)) global_sum += 4;
                    break;
                case 4:
                    /* UNGT: !(x <= y) */
                    if (!__builtin_islessequal(x, y)) global_sum += 5;
                    break;
                case 5:
                    /* UNLE: !(x > y) */
                    if (!__builtin_isgreater(x, y)) global_sum += 6;
                    break;
                case 6:
                    /* UNLT: !(x >= y) */
                    if (!__builtin_isgreaterequal(x, y)) global_sum += 7;
                    break;
                case 7:
                    /* LTGT: (x < y) || (x > y) */
                    if (__builtin_isless(x, y) || __builtin_isgreater(x, y)) global_sum += 8;
                    break;
            }
            
            /* Ternary operator forcing condition code generation */
            global_counter += (__builtin_isunordered(x, y) || x == y) ? 1 : 0;
            
            /* Mixed comparison with integer conversion */
            double z = (double)(int)(i * 10);
            global_counter += (z > y) ? 2 : 1;
        }
    }
    
    /* Switch statement with floating point comparisons */
    {
        volatile double test_val = get_value(global_sum % 5);
        
        switch (global_counter % 7) {
            case 0:
                if (__builtin_isunordered(test_val, nan1)) global_sum += 100;
                break;
            case 1:
                if (__builtin_isordered(test_val, 0.0)) global_sum += 200;
                break;
            case 2:
                if (!__builtin_isgreater(test_val, 1.0) && !__builtin_isless(test_val, 1.0)) 
                    global_sum += 300;
                break;
            case 3:
                if (!__builtin_isless(test_val, 2.0)) global_sum += 400;
                break;
            case 4:
                if (!__builtin_islessequal(test_val, 3.0)) global_sum += 500;
                break;
            case 5:
                if (!__builtin_isgreater(test_val, 4.0)) global_sum += 600;
                break;
            case 6:
                if (!__builtin_isgreaterequal(test_val, 5.0)) global_sum += 700;
                break;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    printf("Final counter: %d\n", global_counter);
    
    return 0;
}

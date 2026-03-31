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
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile double neg_val = -2.71828;
    
    volatile int int_val = 42;
    volatile int zero_int = 0;
    
    /* Force compiler to keep all variables alive */
    asm volatile("" : "+m"(nan_val), "+m"(inf_val), "+m"(normal_val), 
                       "+m"(zero_val), "+m"(neg_val), "+m"(int_val), "+m"(zero_int));
    
    /* Block 1: UNORDERED comparisons (NaN involved) */
    {
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(nan_val, normal_val)) {
            sum += 1;
        }
        
        /* Direct comparison with NaN */
        if (nan_val != nan_val) {  /* This is true for NaN */
            sum += 2;
        }
        
        /* Ternary operator with unordered */
        sum += __builtin_isunordered(nan_val, zero_val) ? 3 : 0;
        
        /* Inline assembly clobber to force re-evaluation */
        asm volatile("" ::: "cc", "memory");
    }
    
    /* Block 2: ORDERED comparisons (no NaN) */
    {
        /* Using __builtin_isordered */
        if (__builtin_isordered(normal_val, neg_val)) {
            sum += 4;
        }
        
        /* Direct ordered comparison */
        if (!__builtin_isunordered(normal_val, zero_val)) {
            sum += 5;
        }
        
        /* Switch statement with ordered condition */
        switch (__builtin_isordered(inf_val, normal_val) ? 1 : 0) {
            case 1: sum += 6; break;
            default: sum += 0; break;
        }
        
        asm volatile("fwait" ::: "memory");
    }
    
    /* Block 3: UNEQ (Unordered or Equal) */
    {
        /* NaN == NaN is false, but UNEQ handles unordered case */
        if (!__builtin_isgreater(nan_val, nan_val) && 
            !__builtin_isless(nan_val, nan_val)) {
            sum += 7;  /* UNEQ should be true */
        }
        
        /* Using volatile to prevent optimization */
        volatile double a = nan_val;
        volatile double b = nan_val;
        sum += (a == b) ? 0 : 8;  /* Should add 8 since NaN != NaN */
        
        asm volatile("" ::: "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    }
    
    /* Block 4: UNGE (Unordered or Greater or Equal) */
    {
        /* Using __builtin_isgreaterequal with NaN */
        if (!__builtin_isless(nan_val, normal_val)) {
            sum += 9;  /* UNGE: not less means greater, equal, or unordered */
        }
        
        /* Mixed comparison */
        sum += (nan_val >= normal_val) ? 10 : 0;
        
        asm volatile("" ::: "cc");
    }
    
    /* Block 5: UNGT (Unordered or Greater Than) */
    {
        /* Using __builtin_isgreater with potential NaN */
        volatile double maybe_nan = (lcg_rand() % 2) ? nan_val : normal_val;
        if (!__builtin_islessequal(maybe_nan, zero_val)) {
            sum += 11;  /* UNGT: not less or equal */
        }
        
        sum += (maybe_nan > zero_val) ? 12 : 0;
    }
    
    /* Block 6: UNLE (Unordered or Less or Equal) */
    {
        /* Using __builtin_islessequal with NaN */
        if (!__builtin_isgreater(nan_val, normal_val)) {
            sum += 13;  /* UNLE: not greater */
        }
        
        sum += (nan_val <= normal_val) ? 14 : 0;
        
        asm volatile("fnop" ::: "memory");
    }
    
    /* Block 7: UNLT (Unordered or Less Than) */
    {
        /* Using __builtin_isless with NaN */
        if (!__builtin_isgreaterequal(nan_val, normal_val)) {
            sum += 15;  /* UNLT: not greater or equal */
        }
        
        sum += (nan_val < normal_val) ? 16 : 0;
    }
    
    /* Block 8: LTGT (Less Than or Greater Than, but not Equal and not Unordered) */
    {
        /* LTGT is true when operands are ordered and not equal */
        if (__builtin_islessgreater(normal_val, neg_val)) {
            sum += 17;  /* 3.14 != -2.71 and both are ordered */
        }
        
        /* Using volatile to ensure evaluation */
        volatile double a = normal_val;
        volatile double b = zero_val;
        sum += (a != b && !__builtin_isunordered(a, b)) ? 18 : 0;
        
        asm volatile("" ::: "fpsr", "memory");
    }
    
    /* Mixed integer-FP comparisons */
    {
        /* Integer to FP conversion in comparison */
        double converted = (double)(int)int_val;
        if (converted != normal_val) {
            sum += 19;
        }
        
        /* More complex mixed comparison */
        volatile int vi = zero_int;
        sum += ((double)vi == zero_val) ? 20 : 0;
    }
    
    /* Loop with varying conditions */
    {
        volatile double array[8] = {
            nan_val, inf_val, normal_val, neg_val,
            zero_val, -inf_val, __builtin_nan("0x1234"), 100.0
        };
        
        for (int i = 0; i < 8; i++) {
            uint32_t r = lcg_rand();
            volatile double a = array[i];
            volatile double b = array[(i + 1) % 8];
            
            /* Different comparisons based on random bits */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) sum += 21;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(a, b)) sum += 22;
                    break;
                case 2:  /* UNEQ */
                    if (!__builtin_isless(a, b) && !__builtin_isgreater(a, b)) 
                        sum += 23;
                    break;
                case 3:  /* UNGE */
                    if (!__builtin_isless(a, b)) sum += 24;
                    break;
                case 4:  /* UNGT */
                    if (!__builtin_islessequal(a, b)) sum += 25;
                    break;
                case 5:  /* UNLE */
                    if (!__builtin_isgreater(a, b)) sum += 26;
                    break;
                case 6:  /* UNLT */
                    if (!__builtin_isgreaterequal(a, b)) sum += 27;
                    break;
                case 7:  /* LTGT */
                    if (__builtin_islessgreater(a, b)) sum += 28;
                    break;
            }
            
            /* Inline assembly to clobber FP state each iteration */
            asm volatile("" ::: "cc", "memory");
            create_side_effect();
        }
    }
    
    /* Final complex expression mixing multiple condition types */
    {
        volatile double x = nan_val;
        volatile double y = normal_val;
        volatile double z = neg_val;
        
        /* Complex conditional expression */
        int result = (__builtin_isunordered(x, y) ? 1 : 0) +
                    (__builtin_islessgreater(y, z) ? 2 : 0) +
                    (!__builtin_isless(x, y) ? 4 : 0) +
                    (!__builtin_isgreater(z, y) ? 8 : 0);
        
        sum += result;
        
        /* Force compiler to generate conditional jumps */
        if (x != x) sum += 29;  /* UNORDERED */
        if (y == y) sum += 30;  /* ORDERED */
        if (!(y > z) && !(y < z)) sum += 31;  /* Potential UNEQ */
        if (!(x < y)) sum += 32;  /* UNGE */
        if (!(x <= y)) sum += 33;  /* UNGT */
        if (!(z > y)) sum += 34;  /* UNLE */
        if (!(z >= y)) sum += 35;  /* UNLT */
        if (y != z && y == y && z == z) sum += 36;  /* LTGT */
    }
    
    printf("Final sum: %d\n", sum);
    printf("Side effects: %d\n", side_effect_counter);
    
    return 0;
}

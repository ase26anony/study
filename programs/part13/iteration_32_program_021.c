#include <stdio.h>
#include <stdint.h>

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
    static volatile double values[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    return values[idx % 5];
}

int main(void) {
    /* Initialize volatile variables with NaN and normal values */
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile int volatile_int = 42;
    
    /* Force FPU status register clobbering */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Block 1: UNORDERED comparisons */
    {
        double a = nan_val;
        double b = normal1;
        
        /* Multiple ways to generate UNORDERED */
        if (__builtin_isunordered(a, b)) sum += 1;
        if (a != a) sum += 1;  /* NaN self-comparison */
        if (!(a == a)) sum += 1;
        
        /* Ternary operator with unordered */
        sum += __builtin_isunordered(a, b) ? 1 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        double a = normal1;
        double b = normal2;
        
        if (!__builtin_isunordered(a, b)) sum += 1;
        if (a == a) sum += 1;  /* Non-NaN is ordered */
        
        /* Mixed with integer conversion */
        double c = (double)(int)volatile_int;
        if (__builtin_islessequal(c, b)) sum += 1;
    }
    
    /* Block 3: UNEQ (Unordered or Equal) */
    {
        double a = nan_val;
        double b = nan_val;
        double c = normal1;
        
        /* Two NaNs are unordered but equal in the UNEQ sense */
        if (!(a < b) && !(a > b)) sum += 1;
        
        /* Normal equal values */
        if (c == normal1) sum += 1;
    }
    
    /* Block 4: UNGE (Unordered or Greater or Equal) */
    {
        double a = nan_val;
        double b = normal1;
        
        if (!(a < b)) sum += 1;  /* Generates UNGE (nlt) */
        if (__builtin_isgreaterequal(a, b)) sum += 1;
    }
    
    /* Block 5: UNGT (Unordered or Greater) */
    {
        double a = nan_val;
        double b = normal1;
        
        if (!(a <= b)) sum += 1;  /* Generates UNGT (nle) */
        if (__builtin_isgreater(a, b)) sum += 1;
    }
    
    /* Block 6: UNLE (Unordered or Less or Equal) */
    {
        double a = nan_val;
        double b = normal1;
        
        if (!(a > b)) sum += 1;  /* Generates UNLE (ule) */
        if (__builtin_islessequal(a, b)) sum += 1;
    }
    
    /* Block 7: UNLT (Unordered or Less) */
    {
        double a = nan_val;
        double b = normal1;
        
        if (!(a >= b)) sum += 1;  /* Generates UNLT (ult) */
        if (__builtin_isless(a, b)) sum += 1;
    }
    
    /* Block 8: LTGT (Less or Greater, but not Equal and not Unordered) */
    {
        double a = normal1;
        double b = normal2;
        
        if ((a < b) || (a > b)) sum += 1;  /* Generates LTGT (une) */
        if (a != b) sum += 1;
    }
    
    /* Clobber FPU again */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    /* Loop with varying conditions */
    for (int i = 0; i < 100; i++) {
        volatile double x = get_value(i);
        volatile double y = get_value(i + 1);
        
        /* Use hash of i to select different comparisons */
        uint32_t h = lcg_rand();
        
        switch (h % 8) {
            case 0:  /* UNORDERED */
                if (__builtin_isunordered(x, y)) sum += 1;
                break;
            case 1:  /* ORDERED */
                if (!__builtin_isunordered(x, y)) sum += 1;
                break;
            case 2:  /* UNEQ */
                if (!(x < y) && !(x > y)) sum += 1;
                break;
            case 3:  /* UNGE */
                if (!(x < y)) sum += 1;
                break;
            case 4:  /* UNGT */
                if (!(x <= y)) sum += 1;
                break;
            case 5:  /* UNLE */
                if (!(x > y)) sum += 1;
                break;
            case 6:  /* UNLT */
                if (!(x >= y)) sum += 1;
                break;
            case 7:  /* LTGT */
                if (x != y) sum += 1;
                break;
        }
        
        /* Mixed integer-FP comparison */
        double z = (double)(int)(volatile_int + i);
        if (z > x) sum += 1;
        if (z < y) sum += 1;
    }
    
    /* Switch statement with floating comparisons */
    {
        volatile double a = get_value(0);
        volatile double b = get_value(1);
        
        int cmp_result = 0;
        if (__builtin_isunordered(a, b)) cmp_result = 1;
        else if (a < b) cmp_result = 2;
        else if (a > b) cmp_result = 3;
        else cmp_result = 4;
        
        switch (cmp_result) {
            case 1: sum += 10; break;  /* UNORDERED */
            case 2: sum += 20; break;  /* UNLT/LT */
            case 3: sum += 30; break;  /* UNGT/GT */
            case 4: sum += 40; break;  /* UNEQ/EQ */
        }
    }
    
    /* Final clobber and output */
    asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
    
    printf("Result: %d\n", sum);
    return 0;
}

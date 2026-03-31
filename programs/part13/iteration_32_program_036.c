#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to create side effects and prevent constant folding */
volatile double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 0.0, -1.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main() {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 3.14159;
    volatile double zero_val = 0.0;
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED - unordered comparison */
    {
        double a = nan_val;
        double b = get_value(0);
        asm volatile ("" ::: "cc");  /* Clobber condition codes */
        if (__builtin_isunordered(a, b)) {
            global_sum += 1;
        }
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        double a = normal_val;
        double b = get_value(1);
        asm volatile ("" ::: "cc");
        if (!__builtin_isunordered(a, b)) {  /* Ordered is opposite of unordered */
            global_sum += 2;
        }
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        double a = nan_val;
        double b = nan_val;
        asm volatile ("" ::: "cc");
        /* Use ternary operator to force condition code */
        global_sum += (a == b) ? 3 : 0;
    }
    
    /* Block 4: UNGE - unordered or greater-or-equal */
    {
        double a = nan_val;
        double b = normal_val;
        asm volatile ("" ::: "cc");
        if (__builtin_isgreater(a, b) || __builtin_isunordered(a, b)) {
            global_sum += 4;
        }
    }
    
    /* Block 5: UNGT - unordered or greater */
    {
        double a = nan_val;
        double b = normal_val;
        asm volatile ("" ::: "cc");
        if (__builtin_isgreaterequal(a, b) || __builtin_isunordered(a, b)) {
            global_sum += 5;
        }
    }
    
    /* Block 6: UNLE - unordered or less-or-equal */
    {
        double a = normal_val;
        double b = nan_val;
        asm volatile ("" ::: "cc");
        if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) {
            global_sum += 6;
        }
    }
    
    /* Block 7: UNLT - unordered or less */
    {
        double a = normal_val;
        double b = nan_val;
        asm volatile ("" ::: "cc");
        if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) {
            global_sum += 7;
        }
    }
    
    /* Block 8: LTGT - less or greater (ordered and not equal) */
    {
        double a = normal_val;
        double b = get_value(2);
        asm volatile ("" ::: "cc");
        if ((a < b) || (a > b)) {  /* LTGT: ordered and not equal */
            global_sum += 8;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        double a = (double)(int)volatile_int;
        double b = get_value(3);
        asm volatile ("" ::: "cc");
        switch ((a != b) ? 1 : 0) {
            case 1: global_sum += 9; break;
            case 0: global_sum += 10; break;
        }
    }
    
    /* Loop with varying conditions */
    for (int i = 0; i < 100; i++) {
        double a = get_value(i);
        double b = get_value(i + 1);
        uint32_t r = lcg_rand();
        
        asm volatile ("" ::: "cc");  /* Force re-evaluation of condition codes */
        
        /* Different comparison based on pseudo-random value */
        switch (r % 8) {
            case 0:  /* UNORDERED */
                if (__builtin_isunordered(a, b)) global_sum++;
                break;
            case 1:  /* ORDERED */
                if (!__builtin_isunordered(a, b)) global_sum++;
                break;
            case 2:  /* UNEQ */
                global_sum += (a == b) ? 1 : 0;
                break;
            case 3:  /* UNGE */
                if (__builtin_isgreater(a, b) || __builtin_isunordered(a, b)) 
                    global_sum++;
                break;
            case 4:  /* UNGT */
                if (__builtin_isgreaterequal(a, b) || __builtin_isunordered(a, b)) 
                    global_sum++;
                break;
            case 5:  /* UNLE */
                if (__builtin_isless(a, b) || __builtin_isunordered(a, b)) 
                    global_sum++;
                break;
            case 6:  /* UNLT */
                if (__builtin_islessequal(a, b) || __builtin_isunordered(a, b)) 
                    global_sum++;
                break;
            case 7:  /* LTGT */
                if ((a < b) || (a > b)) global_sum++;
                break;
        }
    }
    
    /* Additional complex expressions to force condition code emission */
    {
        volatile double x = nan_val;
        volatile double y = normal_val;
        volatile double z = zero_val;
        
        /* Chain of comparisons */
        int result = (x < y) ? 1 : ((y > z) ? 2 : ((x != x) ? 3 : 4));
        global_sum += result;
        
        /* Compound condition */
        if ((x == x) && (y != y) || (z > 0.0)) {
            global_sum += 100;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return 0;
}

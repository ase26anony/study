#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;
volatile int global_counter = 0;

/* Function to prevent constant folding */
double get_value(int idx) {
    static volatile double values[] = {1.0, 2.0, 3.0, 0.0, -1.0};
    return values[idx % 5];
}

/* Simple pseudo-random generator for varying conditions */
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
    volatile double neg_val = -2.71828;
    
    volatile int volatile_int = 42;
    
    /* Block 1: UNORDERED comparisons */
    {
        asm volatile ("" ::: "cc", "memory");  /* Clobber flags */
        if (__builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1;
        }
        
        /* Ternary operator version */
        global_counter += (nan_val != nan_val) ? 1 : 0;
        
        /* With explicit NaN check */
        double temp = get_value(0);
        if (temp != temp) {  /* NaN check */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED comparisons */
    {
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_isordered(normal_val, zero_val)) {
            global_sum += 4;
        }
        
        /* Using !isunordered */
        if (!__builtin_isunordered(normal_val, normal_val)) {
            global_counter += 8;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        asm volatile ("" ::: "cc", "memory");
        /* NaN == NaN is false, but UNEQ handles unordered case */
        if (!__builtin_isgreater(nan_val, nan_val) && 
            !__builtin_isless(nan_val, nan_val)) {
            global_sum += 16;  /* Should trigger UNEQ for unordered */
        }
        
        /* Regular equality with NaN operand */
        double a = get_value(1);
        double b = get_value(2);
        if (!(a > b) && !(a < b)) {  /* UNEQ condition */
            global_counter += 32;
        }
    }
    
    /* Block 4: UNGE (not less than, including unordered) */
    {
        asm volatile ("" ::: "cc", "memory");
        if (!__builtin_isless(nan_val, normal_val)) {
            global_sum += 64;  /* NaN < 3.14 is false, so !(NaN < 3.14) is true */
        }
        
        /* Using >= with potential NaN */
        double x = get_value(3);
        double y = get_value(4);
        if (!(x < y)) {  /* Equivalent to UNGE */
            global_counter += 128;
        }
    }
    
    /* Block 5: UNGT (not less than or equal, including unordered) */
    {
        asm volatile ("" ::: "cc", "memory");
        if (!__builtin_islessequal(nan_val, normal_val)) {
            global_sum += 256;
        }
        
        /* Using > with potential NaN */
        if (!(nan_val <= normal_val)) {
            global_counter += 512;
        }
    }
    
    /* Block 6: UNLE (unordered or less than or equal) */
    {
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_islessequal(nan_val, normal_val) || 
            __builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1024;
        }
        
        /* Direct <= with NaN */
        if (!(nan_val > normal_val)) {  /* UNLE */
            global_counter += 2048;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_isless(nan_val, normal_val) || 
            __builtin_isunordered(nan_val, normal_val)) {
            global_sum += 4096;
        }
        
        /* Direct < with NaN */
        if (!(nan_val >= normal_val)) {  /* UNLT */
            global_counter += 8192;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, excluding equal/unordered) */
    {
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_isless(normal_val, zero_val) || 
            __builtin_isgreater(normal_val, zero_val)) {
            global_sum += 16384;  /* 3.14 != 0.0, so this is true */
        }
        
        /* Using != operator */
        if (normal_val != zero_val) {
            global_counter += 32768;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        asm volatile ("" ::: "cc", "memory");
        double converted = (double)(int)volatile_int;
        if (converted > get_value(0)) {
            global_sum += 65536;
        }
        
        if ((double)(int)volatile_int < get_value(1)) {
            global_counter += 131072;
        }
    }
    
    /* Switch statement with floating comparisons */
    {
        double a = get_value(lcg_rand() % 5);
        double b = get_value(lcg_rand() % 5);
        
        int result = 0;
        if (__builtin_isunordered(a, b)) result = 1;
        else if (a == b) result = 2;
        else if (a < b) result = 3;
        else result = 4;
        
        switch (result) {
            case 1: global_sum += 262144; break;
            case 2: global_sum += 524288; break;
            case 3: global_sum += 1048576; break;
            case 4: global_sum += 2097152; break;
        }
    }
    
    /* Loop with varying conditions */
    {
        double array[10];
        for (int i = 0; i < 10; i++) {
            array[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 10;
            int idx2 = (r >> 8) % 10;
            double x = array[idx1];
            double y = array[idx2];
            
            /* Different comparisons based on hash */
            switch (r % 8) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(x, y)) global_sum += 1;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(x, y)) global_sum += 2;
                    break;
                case 2:  /* UNEQ */
                    if (!(x > y) && !(x < y)) global_sum += 3;
                    break;
                case 3:  /* UNGE */
                    if (!(x < y)) global_sum += 4;
                    break;
                case 4:  /* UNGT */
                    if (!(x <= y)) global_sum += 5;
                    break;
                case 5:  /* UNLE */
                    if (!(x > y)) global_sum += 6;
                    break;
                case 6:  /* UNLT */
                    if (!(x >= y)) global_sum += 7;
                    break;
                case 7:  /* LTGT */
                    if (x != y) global_sum += 8;
                    break;
            }
            
            /* Inline assembly clobbering FP status */
            asm volatile ("" ::: "cc", "memory");
        }
    }
    
    printf("Final sum: %d\n", global_sum + global_counter);
    printf("Counter: %d\n", global_counter);
    
    return 0;
}

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Function to prevent constant folding */
double get_value(int seed) {
    volatile double x = 3.14159 * seed;
    return x;
}

/* Simple pseudo-random generator for loop variation */
static uint32_t lcg_state = 42;
static inline uint32_t lcg_rand(void) {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main(void) {
    volatile double nan_val = __builtin_nan("");
    volatile double inf_val = __builtin_inf();
    volatile double normal_val = 42.0;
    volatile double zero_val = 0.0;
    volatile double neg_val = -42.0;
    
    volatile int volatile_int = 7;
    
    /* Block 1: UNORDERED comparisons */
    {
        asm volatile ("" ::: "cc", "memory");
        if (__builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1;
        }
        
        /* Using explicit operators with NaN */
        double result = (nan_val == normal_val) ? 2.0 : 3.0;
        global_sum += (int)result;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        asm volatile ("" ::: "cc", "memory");
        if (!__builtin_isunordered(normal_val, zero_val)) {
            global_sum += 4;
        }
        
        /* Ordered comparison using builtin */
        if (__builtin_isless(normal_val, inf_val)) {
            global_sum += 5;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        asm volatile ("" ::: "cc", "memory");
        /* Compare NaN with itself - unordered equal */
        if (!(nan_val < nan_val) && !(nan_val > nan_val)) {
            global_sum += 6;
        }
        
        /* Using volatile to prevent optimization */
        volatile double a = get_value(1);
        volatile double b = get_value(1);
        if (!(a < b) && !(a > b)) {
            global_sum += 7;
        }
    }
    
    /* Block 4: UNGE (unordered or greater-or-equal) */
    {
        asm volatile ("" ::: "cc", "memory");
        /* !(a < b) covers UNGE */
        if (!(nan_val < normal_val)) {
            global_sum += 8;
        }
        
        if (!(get_value(2) < get_value(3))) {
            global_sum += 9;
        }
    }
    
    /* Block 5: UNGT (unordered or greater) */
    {
        asm volatile ("" ::: "cc", "memory");
        /* !(a <= b) covers UNGT */
        if (!(nan_val <= normal_val)) {
            global_sum += 10;
        }
        
        if (!(get_value(4) <= get_value(5))) {
            global_sum += 11;
        }
    }
    
    /* Block 6: UNLE (unordered or less-or-equal) */
    {
        asm volatile ("" ::: "cc", "memory");
        if (nan_val <= nan_val) {  /* This is true for UNLE with NaN */
            global_sum += 12;
        }
        
        /* Mixed integer-FP comparison */
        double converted = (double)(int)volatile_int;
        if (converted <= get_value(6)) {
            global_sum += 13;
        }
    }
    
    /* Block 7: UNLT (unordered or less) */
    {
        asm volatile ("" ::: "cc", "memory");
        if (nan_val < normal_val) {  /* False for ordered, but UNLT handles NaN */
            global_sum += 14;
        }
        
        if (get_value(7) < get_value(8)) {
            global_sum += 15;
        }
    }
    
    /* Block 8: LTGT (less or greater, but not equal and not unordered) */
    {
        asm volatile ("" ::: "cc", "memory");
        /* (a < b) || (a > b) but not equal and not unordered */
        if (normal_val != zero_val) {
            global_sum += 16;
        }
        
        /* Explicit check for LTGT */
        volatile double x = get_value(9);
        volatile double y = get_value(10);
        if (x < y || x > y) {
            global_sum += 17;
        }
    }
    
    /* Loop with varying conditions based on pseudo-random sequence */
    {
        double values[16];
        for (int i = 0; i < 16; i++) {
            values[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            int idx1 = r % 16;
            int idx2 = (r >> 8) % 16;
            int cond_type = (r >> 16) % 8;
            
            asm volatile ("" ::: "cc", "memory");
            
            switch (cond_type) {
                case 0: /* UNORDERED */
                    if (__builtin_isunordered(values[idx1], values[idx2])) {
                        global_sum += 1;
                    }
                    break;
                case 1: /* ORDERED */
                    if (!__builtin_isunordered(values[idx1], values[idx2])) {
                        global_sum += 2;
                    }
                    break;
                case 2: /* UNEQ */
                    if (!(values[idx1] < values[idx2]) && 
                        !(values[idx1] > values[idx2])) {
                        global_sum += 3;
                    }
                    break;
                case 3: /* UNGE */
                    if (!(values[idx1] < values[idx2])) {
                        global_sum += 4;
                    }
                    break;
                case 4: /* UNGT */
                    if (!(values[idx1] <= values[idx2])) {
                        global_sum += 5;
                    }
                    break;
                case 5: /* UNLE */
                    if (values[idx1] <= values[idx2]) {
                        global_sum += 6;
                    }
                    break;
                case 6: /* UNLT */
                    if (values[idx1] < values[idx2]) {
                        global_sum += 7;
                    }
                    break;
                case 7: /* LTGT */
                    if (values[idx1] != values[idx2]) {
                        global_sum += 8;
                    }
                    break;
            }
            
            /* Insert NaN occasionally */
            if ((r >> 24) % 16 == 0) {
                values[idx1] = nan_val;
            }
        }
    }
    
    /* Ternary operator usage to force conditional moves/sets */
    {
        volatile double a = get_value(11);
        volatile double b = get_value(12);
        
        /* These should generate SETcc instructions */
        int res1 = (a < b) ? 100 : 200;
        int res2 = (a > b) ? 300 : 400;
        int res3 = (a == b) ? 500 : 600;
        int res4 = (a != b) ? 700 : 800;
        int res5 = (a <= b) ? 900 : 1000;
        int res6 = (a >= b) ? 1100 : 1200;
        
        global_sum += res1 + res2 + res3 + res4 + res5 + res6;
        
        /* With NaN */
        int res7 = (nan_val < a) ? 1300 : 1400;
        int res8 = (nan_val > a) ? 1500 : 1600;
        int res9 = (nan_val == a) ? 1700 : 1800;
        
        global_sum += res7 + res8 + res9;
    }
    
    printf("Final sum: %d\n", global_sum);
    
    return 0;
}

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
        asm volatile ("" : : : "st");
        if (__builtin_isunordered(nan_val, normal_val)) {
            global_sum += 1;
        }
        if (nan_val != nan_val) {  /* NaN != NaN triggers unordered */
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED comparisons */
    {
        asm volatile ("" : : : "st");
        if (__builtin_isordered(normal_val, zero_val)) {
            global_sum += 4;
        }
        if (!__builtin_isunordered(inf_val, neg_val)) {
            global_sum += 8;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        asm volatile ("" : : : "st");
        double a = get_value(0);
        double b = get_value(1);
        /* Use ternary to force condition code generation */
        global_sum += (a == b || __builtin_isunordered(a, b)) ? 16 : 0;
    }
    
    /* Block 4: UNGE (unordered or greater or equal) */
    {
        asm volatile ("" : : : "st");
        volatile double x = get_value(2);
        volatile double y = get_value(3);
        if (__builtin_isunordered(x, y) || x >= y) {
            global_sum += 32;
        }
    }
    
    /* Block 5: UNGT (unordered or greater than) */
    {
        asm volatile ("" : : : "st");
        volatile double p = normal_val;
        volatile double q = zero_val;
        if (__builtin_isunordered(p, q) || p > q) {
            global_sum += 64;
        }
    }
    
    /* Block 6: UNLE (unordered or less or equal) */
    {
        asm volatile ("" : : : "st");
        volatile double m = neg_val;
        volatile double n = normal_val;
        global_sum += (__builtin_isunordered(m, n) || m <= n) ? 128 : 0;
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        asm volatile ("" : : : "st");
        volatile double u = get_value(4);
        volatile double v = get_value(0);
        if (__builtin_isunordered(u, v) || u < v) {
            global_sum += 256;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, but not equal and not unordered) */
    {
        asm volatile ("" : : : "st");
        volatile double r = 1.0;
        volatile double s = 2.0;
        /* LTGT is (a < b || a > b) && !isunordered(a,b) */
        if ((r < s || r > s) && !__builtin_isunordered(r, s)) {
            global_sum += 512;
        }
    }
    
    /* Mixed integer-FP comparisons */
    {
        asm volatile ("" : : : "st");
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
        volatile double arr[10];
        for (int i = 0; i < 10; i++) {
            arr[i] = get_value(i);
        }
        
        for (int i = 0; i < 100; i++) {
            uint32_t r = lcg_rand();
            volatile double a = arr[i % 10];
            volatile double b = arr[(i + 1) % 10];
            
            asm volatile ("" : : : "st");
            
            /* Switch based on random value to generate different condition codes */
            switch (r % 8) {
                case 0:
                    if (__builtin_isunordered(a, b)) global_sum++;
                    break;
                case 1:
                    if (__builtin_isordered(a, b)) global_sum++;
                    break;
                case 2:
                    if (a == b || __builtin_isunordered(a, b)) global_sum++;
                    break;
                case 3:
                    if (__builtin_isunordered(a, b) || a >= b) global_sum++;
                    break;
                case 4:
                    if (__builtin_isunordered(a, b) || a > b) global_sum++;
                    break;
                case 5:
                    if (__builtin_isunordered(a, b) || a <= b) global_sum++;
                    break;
                case 6:
                    if (__builtin_isunordered(a, b) || a < b) global_sum++;
                    break;
                case 7:
                    if ((a < b || a > b) && !__builtin_isunordered(a, b)) global_sum++;
                    break;
            }
            
            /* Additional ternary operations */
            global_sum += (a != b) ? 1 : 0;
            global_sum += (a < b) ? 1 : 0;
            global_sum += (a > b) ? 1 : 0;
        }
    }
    
    /* Complex nested comparisons */
    {
        volatile double x = nan_val;
        volatile double y = normal_val;
        volatile double z = zero_val;
        
        asm volatile ("" : : : "st");
        
        /* This should generate multiple condition code checks */
        if ((x < y && y > z) || (__builtin_isunordered(x, z) && y == normal_val)) {
            global_sum += 4096;
        }
        
        /* Switch statement with floating comparisons */
        int selector = volatile_int % 4;
        switch (selector) {
            case 0:
                if (x != y) global_sum += 8192;
                break;
            case 1:
                if (x == y) global_sum += 16384;
                break;
            case 2:
                if (x < y) global_sum += 32768;
                break;
            case 3:
                if (x > y) global_sum += 65536;
                break;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    return global_sum > 0 ? 0 : 1;
}

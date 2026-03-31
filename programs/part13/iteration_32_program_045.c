#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global accumulator to prevent optimization */
volatile int global_sum = 0;

/* Volatile variables to prevent constant folding */
volatile double vd1, vd2, vd3, vd4;
volatile float vf1, vf2;
volatile int vi1, vi2;

/* Function to get unpredictable values */
double get_val(int i) {
    return (i * 1.2345) - (i / 3.14159);
}

/* Simple LCG for pseudo-random sequence */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

int main() {
    /* Initialize with NaN and normal values */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    
    vd1 = 1.5;
    vd2 = 2.5;
    vd3 = nan_val;
    vd4 = 3.14159;
    vf1 = 2.0f;
    vf2 = nan_val;
    vi1 = 42;
    vi2 = -7;
    
    /* Block 1: UNORDERED - unordered comparison */
    {
        asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)");
        int result = __builtin_isunordered(vd1, vd3);
        global_sum += result ? 1 : 0;
        
        /* Also use in if statement */
        if (__builtin_isunordered(vf1, vf2)) {
            global_sum += 2;
        }
    }
    
    /* Block 2: ORDERED - ordered comparison */
    {
        asm volatile ("" : : : "cc", "memory");
        int result = !__builtin_isunordered(vd1, vd2);
        global_sum += result ? 1 : 0;
        
        /* Ternary operator */
        global_sum += (vd1 == vd1 && vd2 == vd2) ? 3 : 0;
    }
    
    /* Block 3: UNEQ - unordered or equal */
    {
        /* (a == b) || (a != a && b != b) */
        int result = (vd1 == vd2) || (vd1 != vd1 && vd2 != vd2);
        global_sum += result ? 1 : 0;
        
        /* With NaN */
        result = (vd3 == vd3) || (vd3 != vd3 && vd3 != vd3);
        global_sum += result ? 1 : 0;
    }
    
    /* Block 4: UNGE - unordered or greater-or-equal */
    {
        /* !(a < b) */
        int result = !(vd1 < vd2);
        global_sum += result ? 1 : 0;
        
        /* With NaN */
        result = !(vd3 < vd1);
        global_sum += result ? 1 : 0;
    }
    
    /* Block 5: UNGT - unordered or greater */
    {
        /* !(a <= b) */
        int result = !(vd1 <= vd2);
        global_sum += result ? 1 : 0;
        
        /* Using __builtin_isgreater */
        result = __builtin_isgreater(vd4, vd1) || __builtin_isunordered(vd4, vd1);
        global_sum += result ? 1 : 0;
    }
    
    /* Block 6: UNLE - unordered or less-or-equal */
    {
        /* (a <= b) || (a != a) || (b != b) */
        int result = (vd1 <= vd2) || (vd1 != vd1) || (vd2 != vd2);
        global_sum += result ? 1 : 0;
        
        /* With NaN */
        result = (vd3 <= vd1) || (vd3 != vd3) || (vd1 != vd1);
        global_sum += result ? 1 : 0;
    }
    
    /* Block 7: UNLT - unordered or less */
    {
        /* (a < b) || (a != a) || (b != b) */
        int result = (vd1 < vd2) || (vd1 != vd1) || (vd2 != vd2);
        global_sum += result ? 1 : 0;
        
        /* Using __builtin_isless */
        result = __builtin_isless(vd1, vd4) || __builtin_isunordered(vd1, vd4);
        global_sum += result ? 1 : 0;
    }
    
    /* Block 8: LTGT - less or greater (ordered and not equal) */
    {
        /* (a < b) || (a > b) */
        int result = (vd1 < vd2) || (vd1 > vd2);
        global_sum += result ? 1 : 0;
        
        /* Alternative: !(a == b) && !(a != a || b != b) */
        result = (vd1 != vd2) && (vd1 == vd1) && (vd2 == vd2);
        global_sum += result ? 1 : 0;
    }
    
    /* Mixed integer-FP comparisons */
    {
        double converted = (double)(int)vi1;
        if (converted > vd1) {
            global_sum += 5;
        }
        
        if ((float)vi2 < vf1) {
            global_sum += 3;
        }
    }
    
    /* Loop with varying conditions */
    double values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = get_val(i);
        if (i % 2 == 0) {
            values[i] = (i == 5) ? nan_val : values[i];
        }
    }
    
    for (int i = 0; i < 20; i++) {
        uint32_t r = lcg_rand();
        double a = values[i];
        double b = values[(i + 1) % 20];
        
        /* Switch based on random bits to generate different condition codes */
        switch (r % 8) {
            case 0: /* UNORDERED */
                if (__builtin_isunordered(a, b)) global_sum++;
                break;
            case 1: /* ORDERED */
                if (!__builtin_isunordered(a, b)) global_sum++;
                break;
            case 2: /* UNEQ */
                if ((a == b) || (a != a && b != b)) global_sum++;
                break;
            case 3: /* UNGE */
                if (!(a < b)) global_sum++;
                break;
            case 4: /* UNGT */
                if (!(a <= b)) global_sum++;
                break;
            case 5: /* UNLE */
                if ((a <= b) || (a != a) || (b != b)) global_sum++;
                break;
            case 6: /* UNLT */
                if ((a < b) || (a != a) || (b != b)) global_sum++;
                break;
            case 7: /* LTGT */
                if ((a < b) || (a > b)) global_sum++;
                break;
        }
        
        /* Clobber FP status register periodically */
        if (i % 4 == 0) {
            asm volatile ("" : : : "st", "st(1)", "st(2)", "st(3)", "cc");
        }
    }
    
    /* Complex nested comparisons */
    {
        volatile double x = vd1, y = vd2, z = vd3;
        
        /* This should generate multiple condition codes */
        int cond1 = (x < y) || __builtin_isunordered(x, z);
        int cond2 = !(y >= z) && (x == x);
        int cond3 = (z != z) ? 1 : (x > y);
        
        global_sum += cond1 ? 1 : 0;
        global_sum += cond2 ? 2 : 0;
        global_sum += cond3 ? 3 : 0;
        
        /* Switch statement with FP comparisons */
        switch (vi1 % 4) {
            case 0:
                if (x < y && !__builtin_isunordered(x, y)) global_sum += 10;
                break;
            case 1:
                if (x > y || __builtin_isunordered(x, z)) global_sum += 20;
                break;
            case 2:
                if (x == y || (x != x && y != y)) global_sum += 30;
                break;
            case 3:
                if (x != y && x == x && y == y) global_sum += 40;
                break;
        }
    }
    
    printf("Final sum: %d\n", global_sum);
    
    /* Additional test with signaling NaN if supported */
    #ifdef __SUPPORT_SNAN__
    {
        double snan = __builtin_nans("");
        if (snan != snan) {  /* This should be true for NaN */
            global_sum += 100;
        }
    }
    #endif
    
    return global_sum > 0 ? 0 : 1;
}

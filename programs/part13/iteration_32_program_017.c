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

/* Function to force re-evaluation */
int __attribute__((noinline)) get_int(void) {
    return 42;
}

double __attribute__((noinline)) get_double(void) {
    return 3.14159;
}

int main(void) {
    /* Volatile variables to prevent constant folding */
    volatile double v1, v2, v3, v4;
    volatile int vi1, vi2;
    
    /* Initialize with NaN and normal values */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    
    v1 = 1.5;
    v2 = 2.5;
    v3 = nan_val;
    v4 = 0.0;
    vi1 = 10;
    vi2 = -5;
    
    /* Block 1: UNORDERED comparisons */
    {
        /* Using __builtin_isunordered */
        if (__builtin_isunordered(v1, v3)) {
            sum += 1;
        }
        
        /* Direct comparison with NaN */
        asm volatile ("" ::: "cc", "memory");  /* Clobber flags */
        if (v1 != v3) {  /* NaN != anything is true, even itself */
            sum += 2;
        }
        
        /* Ternary with unordered */
        sum += __builtin_isunordered(v3, v4) ? 3 : 0;
    }
    
    /* Block 2: ORDERED comparisons */
    {
        /* Using __builtin_isordered */
        if (__builtin_isordered(v1, v2)) {
            sum += 4;
        }
        
        /* Ordered check via negation */
        asm volatile ("" ::: "cc", "memory");
        if (!__builtin_isunordered(v1, v2)) {
            sum += 5;
        }
        
        /* Switch based on ordered */
        switch (__builtin_isordered(v1, get_double()) ? 1 : 0) {
            case 1: sum += 6; break;
            default: sum += 7; break;
        }
    }
    
    /* Block 3: UNEQ (unordered or equal) */
    {
        /* !(a < b || a > b) handles NaN equality */
        if (!(v3 < v4 || v3 > v4)) {  /* UNEQ: either equal or unordered */
            sum += 8;
        }
        
        /* Using volatile to force re-evaluation */
        volatile double t1 = v3;
        volatile double t2 = v4;
        asm volatile ("" ::: "cc", "memory");
        sum += !(t1 < t2 || t1 > t2) ? 9 : 0;
    }
    
    /* Block 4: UNGE (not less than: !(a < b), unordered or greater-or-equal) */
    {
        if (!(v1 < v3)) {  /* UNGE: nlt */
            sum += 10;
        }
        
        /* Mixed with integer conversion */
        asm volatile ("" ::: "cc", "memory");
        if (!((double)vi1 < v3)) {
            sum += 11;
        }
    }
    
    /* Block 5: UNGT (not less-or-equal: !(a <= b), unordered or greater) */
    {
        if (!(v2 <= v3)) {  /* UNGT: nle */
            sum += 12;
        }
        
        /* Inline assembly clobber between comparisons */
        asm volatile ("" ::: "cc", "memory");
        sum += !(get_double() <= v3) ? 13 : 0;
    }
    
    /* Block 6: UNLE (unordered or less-or-equal) */
    {
        if (v3 <= v1 || __builtin_isunordered(v3, v1)) {
            sum += 14;
        }
        
        /* Alternative formulation */
        asm volatile ("" ::: "cc", "memory");
        if (!(v1 > v3)) {  /* UNLE: ule */
            sum += 15;
        }
    }
    
    /* Block 7: UNLT (unordered or less than) */
    {
        if (v3 < v2 || __builtin_isunordered(v3, v2)) {
            sum += 16;
        }
        
        /* Direct ult condition */
        asm volatile ("" ::: "cc", "memory");
        if (!(v2 >= v3)) {  /* UNLT: ult */
            sum += 17;
        }
    }
    
    /* Block 8: LTGT (less than or greater than, but not equal and not unordered) */
    {
        /* une: not equal and ordered */
        if ((v1 < v2 || v1 > v2) && __builtin_isordered(v1, v2)) {
            sum += 18;
        }
        
        /* Alternative with != and ordered check */
        asm volatile ("" ::: "cc", "memory");
        if (v1 != v2 && __builtin_isordered(v1, v2)) {
            sum += 19;
        }
    }
    
    /* Loop with varying conditions */
    {
        double arr[8] = {1.0, 2.0, nan_val, 3.0, nan_val, 4.0, 5.0, nan_val};
        
        for (int i = 0; i < 8; i++) {
            uint32_t r = lcg_rand();
            volatile double a = arr[i];
            volatile double b = arr[(i + 1) & 7];
            
            /* Select condition based on pseudo-random bits */
            switch (r & 7) {
                case 0:  /* UNORDERED */
                    if (__builtin_isunordered(a, b)) sum += 20;
                    break;
                case 1:  /* ORDERED */
                    if (__builtin_isordered(a, b)) sum += 21;
                    break;
                case 2:  /* UNEQ */
                    if (!(a < b || a > b)) sum += 22;
                    break;
                case 3:  /* UNGE */
                    if (!(a < b)) sum += 23;
                    break;
                case 4:  /* UNGT */
                    if (!(a <= b)) sum += 24;
                    break;
                case 5:  /* UNLE */
                    if (!(a > b)) sum += 25;
                    break;
                case 6:  /* UNLT */
                    if (!(a >= b)) sum += 26;
                    break;
                case 7:  /* LTGT */
                    if (a != b && __builtin_isordered(a, b)) sum += 27;
                    break;
            }
            
            /* Clobber flags to force condition code regeneration */
            asm volatile ("" ::: "cc", "memory");
            
            /* Mixed integer-FP comparison */
            volatile int ival = (int)a;
            if ((double)ival != b) {
                sum += 28;
            }
        }
    }
    
    /* Complex nested conditions */
    {
        volatile double x = v1;
        volatile double y = v3;
        volatile double z = v2;
        
        /* Chain of comparisons requiring different condition codes */
        int result = (__builtin_isunordered(x, y) ? 1 : 0) +
                    (__builtin_isordered(y, z) ? 2 : 0) +
                    (!(x < y) ? 4 : 0) +
                    (!(y <= z) ? 8 : 0) +
                    (!(x > y) ? 16 : 0) +
                    (!(y >= z) ? 32 : 0) +
                    ((x != z && __builtin_isordered(x, z)) ? 64 : 0);
        
        sum += result;
        
        /* Final check with signaling */
        asm volatile ("" ::: "cc", "memory");
        if (x != y || __builtin_isunordered(x, y) || !__builtin_isordered(x, z)) {
            sum += 128;
        }
    }
    
    printf("Final sum: %d\n", sum);
    return 0;
}

/* x87_comparison_test.c
 * Designed to trigger x87 floating-point comparison mnemonics in GCC's i386 backend
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math x87_comparison_test.c -lm
 */

#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ld[16];
volatile int global_index = 0;

/* Helper to get dynamic long double values */
long double get_ld(int idx) {
    return global_ld[idx % 16];
}

/* Complex comparison function designed to use multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons (should generate "ord" type codes) */
    if (a < b && !(a != a) && !(b != b)) {
        result |= 1;  /* LT with ordered operands */
    }
    
    if (c > d && c == c && d == d) {
        result |= 2;  /* GT with ordered operands */
    }
    
    /* Equality comparisons */
    if (a == b) {
        result |= 4;  /* EQ */
    }
    
    if (c != d) {
        result |= 8;  /* NEQ/UNE */
    }
    
    /* Unordered comparisons with explicit NaN checks */
    if (a != a || b != b) {
        result |= 16;  /* UNORDERED detection */
    }
    
    /* UNEQ: unordered or equal */
    if (!(a < b || a > b)) {
        result |= 32;  /* UNEQ (either equal or unordered) */
    }
    
    /* UNGE: not less than (greater or equal or unordered) */
    if (!(a < b)) {
        result |= 64;  /* UNGE/NLT */
    }
    
    /* UNLE: not greater than (less or equal or unordered) */
    if (!(a > b)) {
        result |= 128; /* UNLE/ULE? */
    }
    
    /* UNLT: less than or unordered */
    if (a < b || a != a || b != b) {
        result |= 256; /* UNLT/ULT */
    }
    
    /* UNGT: greater than or unordered */
    if (a > b || a != a || b != b) {
        result |= 512; /* UNGT/NLE */
    }
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((a < b || a > b) && a == a && b == b) {
        result |= 1024; /* LTGT/UNE */
    }
    
    return result;
}

/* Test function for ordered comparisons */
int test_ordered_comparisons(void) {
    volatile long double x = get_ld(0);
    volatile long double y = get_ld(1);
    volatile long double z = get_ld(2);
    volatile long double w = get_ld(3);
    
    int results = 0;
    
    /* Basic ordered comparisons */
    if (x < y) results |= 1;
    if (x > y) results |= 2;
    if (x <= y) results |= 4;
    if (x >= y) results |= 8;
    if (x == y) results |= 16;
    if (x != y) results |= 32;
    
    /* Compound ordered comparisons */
    if ((x < y) && (z > w)) results |= 64;
    if ((x <= y) || (z >= w)) results |= 128;
    if ((x == y) != (z == w)) results |= 256;
    
    /* Mixed with constants */
    if (x < 1.0L) results |= 512;
    if (y > -1.0L) results |= 1024;
    if (z == 0.0L) results |= 2048;
    if (w != 3.14159265358979323846L) results |= 4096;
    
    return results;
}

/* Test function for unordered comparisons with NaN */
int test_unordered_comparisons(void) {
    /* Generate NaN values using different methods */
    volatile long double nan1 = __builtin_nanl("");
    volatile long double nan2 = 0.0L / 0.0L;
    volatile long double nan3 = sqrtl(-1.0L);
    volatile long double inf = __builtin_infl();
    volatile long double normal = 42.0L;
    
    int results = 0;
    
    /* Compare NaN with normal numbers */
    if (nan1 < normal) results |= 1;      /* Should be false, unordered */
    if (nan1 > normal) results |= 2;      /* Should be false, unordered */
    if (nan1 <= normal) results |= 4;     /* Should be false, unordered */
    if (nan1 >= normal) results |= 8;     /* Should be false, unordered */
    if (nan1 == normal) results |= 16;    /* Should be false, unordered */
    if (nan1 != normal) results |= 32;    /* Should be true! NaN != normal */
    
    /* Compare NaN with infinity */
    if (nan2 < inf) results |= 64;
    if (nan2 > inf) results |= 128;
    if (nan2 == inf) results |= 256;
    if (nan2 != inf) results |= 512;
    
    /* Compare NaN with NaN */
    if (nan1 < nan2) results |= 1024;
    if (nan1 > nan2) results |= 2048;
    if (nan1 <= nan2) results |= 4096;
    if (nan1 >= nan2) results |= 8192;
    if (nan1 == nan2) results |= 16384;   /* NaN != NaN */
    if (nan1 != nan2) results |= 32768;   /* NaN != NaN is true */
    
    /* Compare infinity with normal */
    if (inf < normal) results |= 65536;
    if (inf > normal) results |= 131072;
    if (inf == normal) results |= 262144;
    if (inf != normal) results |= 524288;
    
    /* Mixed comparisons in expressions */
    if ((nan1 != normal) && (inf > 0.0L)) results |= 1048576;
    if ((nan1 == nan2) || (normal < inf)) results |= 2097152;
    
    return results;
}

/* Test function with mixed precision */
int test_mixed_precision(void) {
    volatile float f = 3.14f;
    volatile double d = 2.718281828459045;
    volatile long double ld = get_ld(4);
    
    int results = 0;
    
    /* Compare long double with float (promotion happens) */
    if (ld < (long double)f) results |= 1;
    if (ld > (long double)f) results |= 2;
    
    /* Compare long double with double */
    if (ld == (long double)d) results |= 4;
    if (ld != (long double)d) results |= 8;
    
    /* Compare with integer constants */
    if (ld < 10) results |= 16;
    if (ld > -5) results |= 32;
    if (ld == 0) results |= 64;
    if (ld != 1) results |= 128;
    
    /* Complex expression with mixed types */
    if ((ld < f) || (ld > d) || (ld == 0.0L)) results |= 256;
    
    return results;
}

/* Loop with long double termination condition */
int test_loop_comparisons(void) {
    volatile long double x = get_ld(5);
    volatile long double y = get_ld(6);
    volatile long double accumulator = 0.0L;
    int iterations = 0;
    int results = 0;
    
    /* Loop condition based on long double comparison */
    while (x > y && x == x && y == y && iterations < 100) {
        accumulator += x - y;
        y += 0.1L;
        iterations++;
        
        /* Nested if with long double comparison */
        if (accumulator > 100.0L) {
            results |= 1;
            break;
        }
        
        if (accumulator < -100.0L) {
            results |= 2;
            break;
        }
        
        /* Check for NaN during computation */
        if (accumulator != accumulator) {
            results |= 4;
            break;
        }
    }
    
    results |= (iterations << 8);
    
    /* Do-while with comparison */
    do {
        accumulator /= 2.0L;
        if (accumulator == 0.0L) {
            results |= 8;
        }
    } while (accumulator > 0.001L && accumulator == accumulator);
    
    return results;
}

/* Switch statement based on comparison results */
int test_switch_comparisons(void) {
    volatile long double a = get_ld(7);
    volatile long double b = get_ld(8);
    volatile long double c = get_ld(9);
    
    int result = 0;
    
    /* Use comparison results to drive a switch */
    int cmp_result = 0;
    if (a < b) cmp_result = 1;
    else if (a > b) cmp_result = 2;
    else if (a == b) cmp_result = 3;
    else cmp_result = 4;  /* unordered */
    
    switch (cmp_result) {
        case 1:  /* a < b */
            if (c > 0.0L) result |= 1;
            break;
        case 2:  /* a > b */
            if (c < 0.0L) result |= 2;
            break;
        case 3:  /* a == b */
            if (c == 0.0L) result |= 4;
            break;
        case 4:  /* unordered */
            if (c != c) result |= 8;  /* c is NaN */
            break;
    }
    
    /* Another switch based on complex comparison */
    int complex_cmp = 0;
    if (a < b && b < c) complex_cmp = 1;
    else if (a > b && b > c) complex_cmp = 2;
    else if (a != a || b != b || c != c) complex_cmp = 3;
    else complex_cmp = 4;
    
    switch (complex_cmp) {
        case 1:
            result |= 16;
            break;
        case 2:
            result |= 32;
            break;
        case 3:
            result |= 64;
            break;
        case 4:
            result |= 128;
            break;
    }
    
    return result;
}

int main(void) {
    /* Initialize global array with mix of values */
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: global_ld[i] = (long double)i * 1.5L; break;
            case 1: global_ld[i] = - (long double)i * 0.7L; break;
            case 2: global_ld[i] = __builtin_infl(); break;
            case 3: global_ld[i] = -__builtin_infl(); break;
            case 4: global_ld[i] = __builtin_nanl(""); break;
        }
    }
    
    int hash = 0;
    
    /* Run all test functions */
    hash ^= test_ordered_comparisons();
    hash ^= test_unordered_comparisons();
    hash ^= test_mixed_precision();
    hash ^= test_loop_comparisons();
    hash ^= test_switch_comparisons();
    
    /* Complex comparison with multiple operands */
    volatile long double p = get_ld(10);
    volatile long double q = get_ld(11);
    volatile long double r = get_ld(12);
    volatile long double s = get_ld(13);
    
    hash ^= complex_x87_comparison(p, q, r, s);
    
    /* Additional direct comparisons to ensure coverage */
    volatile long double nan_val = __builtin_nanl("");
    volatile long double inf_val = __builtin_infl();
    volatile long double zero = 0.0L;
    volatile long double one = 1.0L;
    
    /* Force generation of various condition codes */
    if (!(nan_val < one)) hash ^= 0x1000;      /* UNGE/NLT */
    if (!(nan_val > one)) hash ^= 0x2000;      /* UNLE/ULE? */
    if (nan_val < one || nan_val != nan_val) hash ^= 0x4000;  /* UNLT/ULT */
    if (nan_val > one || nan_val != nan_val) hash ^= 0x8000;  /* UNGT/NLE */
    if ((one < zero || one > zero) && one == one && zero == zero) hash ^= 0x10000; /* LTGT/UNE */
    
    /* Print hash to prevent dead code elimination */
    printf("Result hash: 0x%08x\n", hash);
    
    return 0;
}

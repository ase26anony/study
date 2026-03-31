/* x87_comparison_test.c
 * Designed to trigger x87 comparison mnemonics in i386.cc lines 13992-14017
 * Compile with: gcc -m32 -O2 -ffloat-store -mfpmath=387 -fno-fast-math -o x87_test x87_comparison_test.c
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Global volatile array to prevent constant folding */
volatile long double global_ldbl_array[16];
volatile int array_index = 0;

/* Helper to get dynamic long double values */
long double get_ldbl(int idx) {
    return global_ldbl_array[idx % 16];
}

/* Complex comparison function using multiple x87 condition codes */
int complex_x87_comparison(long double a, long double b, long double c, long double d) {
    int result = 0;
    
    /* Ordered comparisons */
    if (a < b) result |= 1;      /* LT */
    if (a > c) result |= 2;      /* GT */
    if (a <= d) result |= 4;     /* LE */
    if (b >= c) result |= 8;     /* GE */
    if (a == b) result |= 16;    /* EQ */
    if (c != d) result |= 32;    /* NEQ */
    
    /* Unordered comparisons with explicit NaN checks */
    volatile long double nan_val = __builtin_nanl("");
    volatile long double inf_val = __builtin_infl();
    
    /* UNORDERED: compare NaN with normal number */
    if (!(a == nan_val)) result |= 64;  /* Should be true for non-NaN a */
    
    /* UNEQ: unordered or equal */
    if (nan_val == nan_val) result |= 128;  /* Always false, but compiler might generate UNEQ */
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!(a < nan_val)) result |= 256;
    
    /* UNGT: not less than or equal (unordered or greater) */
    if (!(a <= nan_val)) result |= 512;
    
    /* UNLE: unordered or less or equal */
    if (!(nan_val > b)) result |= 1024;
    
    /* UNLT: unordered or less than */
    if (!(nan_val >= b)) result |= 2048;
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if (a < b || a > b) result |= 4096;
    
    return result;
}

/* Function with nested comparisons */
int nested_x87_comparisons(long double x, long double y, long double z) {
    if (x != y) {
        if (z <= x) {
            return (y > z) && !(x == y);
        } else if (z >= y) {
            return (x < z) || (y != z);
        }
    }
    
    /* Create NaN through arithmetic */
    volatile long double zero = 0.0L;
    volatile long double nan1 = zero / zero;  /* 0.0/0.0 = NaN */
    volatile long double nan2 = sqrtl(-1.0L); /* sqrt(-1) = NaN */
    
    /* Compare NaNs with various values */
    if (nan1 < 1.0L) {
        /* This should never be taken for NaN */
        return -1;
    }
    
    if (!(nan2 == 2.0L)) {
        /* This should always be taken */
        return (x > y) ? 1 : 0;
    }
    
    return 0;
}

/* Loop with long double termination condition */
int x87_controlled_loop(long double start, long double limit) {
    volatile long double counter = start;
    int iterations = 0;
    
    /* Use mixed precision */
    volatile float f_limit = (float)limit;
    volatile double d_increment = 0.5;
    
    while (counter < limit && iterations < 100) {
        /* Mixed-type comparison forces promotion to long double */
        if ((long double)counter < (long double)f_limit) {
            counter += (long double)d_increment;
        }
        
        /* Compare with integer constant cast to long double */
        if (counter > (long double)10) {
            counter -= 1.0L;
        }
        
        iterations++;
        
        /* Early exit on NaN detection */
        volatile long double temp = counter;
        if (temp != temp) {  /* NaN check: NaN != NaN */
            break;
        }
    }
    
    return iterations;
}

/* Switch statement based on comparison results */
int x87_switch_logic(long double a, long double b) {
    int result = 0;
    
    /* Force runtime evaluation */
    volatile long double va = a;
    volatile long double vb = b;
    
    if (va < vb) {
        result = 1;
    } else if (va > vb) {
        result = 2;
    } else if (va <= vb) {
        result = 3;
    } else if (va >= vb) {
        result = 4;
    }
    
    /* Unordered comparisons */
    volatile long double nan_val = __builtin_nanl("");
    if (!(va < nan_val)) {  /* UNGE: not less than */
        result |= 0x10;
    }
    
    if (!(nan_val <= vb)) {  /* UNGT: not less than or equal */
        result |= 0x20;
    }
    
    return result;
}

/* Test function focusing on unordered comparisons */
int test_unordered_comparisons(void) {
    volatile long double nan = __builtin_nanl("");
    volatile long double inf = __builtin_infl();
    volatile long double normal = 3.14159265358979323846L;
    volatile long double zero = 0.0L;
    
    int results = 0;
    
    /* Generate various NaN values */
    volatile long double nan1 = 0.0L / 0.0L;
    volatile long double nan2 = inf - inf;
    volatile long double nan3 = sqrtl(-1.0L);
    
    /* Compare NaN with different values */
    results |= (nan1 == nan2) ? 0 : 1;           /* Should be 1 (NaN != NaN) */
    results |= (nan < normal) ? 0 : 2;           /* Should be 2 (unordered) */
    results |= (normal > nan) ? 0 : 4;           /* Should be 4 (unordered) */
    results |= (!(nan <= inf)) ? 8 : 0;          /* UNGT pattern */
    results |= (!(inf >= nan)) ? 16 : 0;         /* UNLT pattern */
    results |= (!(nan == zero)) ? 32 : 0;        /* Should be 32 */
    results |= (!(zero != nan)) ? 64 : 0;        /* Should be 64 */
    
    /* Ordered comparisons with infinity */
    results |= (normal < inf) ? 128 : 0;
    results |= (-inf < normal) ? 256 : 0;
    results |= (inf == inf) ? 512 : 0;
    
    return results;
}

int main(void) {
    /* Initialize array with mixed values */
    global_ldbl_array[0] = 1.0L;
    global_ldbl_array[1] = 2.0L;
    global_ldbl_array[2] = 3.14159265358979323846L;
    global_ldbl_array[3] = __builtin_infl();
    global_ldbl_array[4] = -__builtin_infl();
    global_ldbl_array[5] = __builtin_nanl("");
    global_ldbl_array[6] = 0.0L;
    global_ldbl_array[7] = -0.0L;
    global_ldbl_array[8] = 100.0L;
    global_ldbl_array[9] = 1.0e-10L;
    global_ldbl_array[10] = 1.0e10L;
    global_ldbl_array[11] = 0.0L / 0.0L;  /* NaN */
    global_ldbl_array[12] = sqrtl(-1.0L); /* NaN */
    global_ldbl_array[13] = 42.0L;
    global_ldbl_array[14] = -273.15L;
    global_ldbl_array[15] = 6.02214076e23L;
    
    int bool_results[64];
    int result_index = 0;
    
    /* Test 1: Complex comparisons */
    bool_results[result_index++] = complex_x87_comparison(
        get_ldbl(0), get_ldbl(1), get_ldbl(2), get_ldbl(3)
    ) != 0;
    
    /* Test 2: Nested comparisons */
    bool_results[result_index++] = nested_x87_comparisons(
        get_ldbl(4), get_ldbl(5), get_ldbl(6)
    ) != 0;
    
    /* Test 3: Loop with x87 control */
    bool_results[result_index++] = x87_controlled_loop(
        get_ldbl(7), get_ldbl(8)
    ) > 0;
    
    /* Test 4: Switch logic */
    bool_results[result_index++] = x87_switch_logic(
        get_ldbl(9), get_ldbl(10)
    ) != 0;
    
    /* Test 5: Unordered comparisons */
    bool_results[result_index++] = test_unordered_comparisons() != 0;
    
    /* Additional direct comparisons to cover all mnemonics */
    volatile long double v1 = get_ldbl(11);
    volatile long double v2 = get_ldbl(12);
    volatile long double v3 = get_ldbl(13);
    
    /* Generate various comparison patterns */
    for (int i = 0; i < 8 && result_index < 60; i++) {
        long double a = get_ldbl(i * 2);
        long double b = get_ldbl(i * 2 + 1);
        
        /* Ordered comparisons */
        bool_results[result_index++] = (a < b);
        bool_results[result_index++] = (a > b);
        bool_results[result_index++] = (a <= b);
        bool_results[result_index++] = (a >= b);
        bool_results[result_index++] = (a == b);
        bool_results[result_index++] = (a != b);
        
        /* Mixed precision comparisons */
        float f_a = (float)a;
        double d_b = (double)b;
        bool_results[result_index++] = ((long double)f_a < (long double)d_b);
        
        /* Compare with NaN */
        volatile long double nan = __builtin_nanl("");
        bool_results[result_index++] = !(a < nan);  /* UNGE */
        bool_results[result_index++] = !(nan <= b); /* UNGT */
        bool_results[result_index++] = !(nan > a);  /* UNLE */
        bool_results[result_index++] = !(b >= nan); /* UNLT */
    }
    
    /* Compute verification hash (XOR of all results) */
    int verification_hash = 0;
    for (int i = 0; i < result_index; i++) {
        verification_hash ^= (bool_results[i] & 1) << (i % 32);
    }
    
    printf("Verification hash: %d\n", verification_hash);
    printf("Total comparisons performed: %d\n", result_index);
    
    return verification_hash != 0 ? 0 : 1;
}

/* test_i386_condition_codes.c */
#include <stdio.h>
#include <math.h>
#include <stdint.h>

/* Prevent optimization */
extern void sink(int);
extern void sink_ptr(void*);

/* Force separate code generation */
#define NOINLINE __attribute__((noinline))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = 0.0/0.0;
volatile double g_inf = 1.0/0.0;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_two = 2.0;

/* Arrays for memory operand variations */
volatile float farr[4] = {0.0f, 1.0f, 2.0f, 3.0f};
volatile double darr[4] = {0.0, 1.0, 2.0, 3.0};
volatile long double ldarr[4] = {0.0L, 1.0L, 2.0L, 3.0L};

/* Struct for complex memory access */
struct fp_pair {
    volatile float f;
    volatile double d;
    volatile long double ld;
};

volatile struct fp_pair fp_struct = {0.0f, 0.0, 0.0L};

/* Pattern 1: UNORDERED comparisons */
NOINLINE int test_unordered(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile float f1 = farr[0];
    f1 = g_nan;  /* Force NaN */
    
    int result = 0;
    
    /* Direct NaN comparison */
    if (d1 != d1) {
        result |= 1;  /* unordered */
    }
    
    /* Using isunordered */
    if (isunordered(d1, d2)) {
        result |= 2;
    }
    
    /* Float unordered */
    if (f1 != f1) {
        result |= 4;
    }
    
    /* Complex expression with unordered */
    result |= (isunordered(d1, d2) && (d2 > g_zero)) ? 8 : 0;
    
    sink(result);
    return result;
}

/* Pattern 2: ORDERED comparisons */
NOINLINE int test_ordered(void) {
    volatile double d1 = g_one;
    volatile double d2 = g_two;
    volatile float f1 = farr[1];
    
    int result = 0;
    
    /* Direct ordered comparison */
    if (d1 == d1) {
        result |= 1;  /* ordered */
    }
    
    /* Using isordered */
    if (isordered(d1, d2)) {
        result |= 2;
    }
    
    /* Memory operand */
    volatile double* pd = &darr[1];
    if (isordered(*pd, g_one)) {
        result |= 4;
    }
    
    /* Struct member */
    if (isordered(fp_struct.d, g_one)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 3: UNEQ (unordered or equal) */
NOINLINE int test_uneq(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* Generate UNEQ: !(a != b) which is (a == b) including NaN cases */
    if (!(a != b)) {
        result |= 1;
    }
    
    /* With NaN operand */
    if (!(nan != nan)) {  /* NaN != NaN is true, so !(true) is false */
        result |= 2;
    }
    
    /* Complex expression */
    result |= (!(a != b) || (b > a)) ? 4 : 0;
    
    /* Using inline assembly to force condition code */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : taken_label
    );
    
    result |= 8;
    
taken_label:
    sink(result);
    return result;
}

/* Pattern 4: UNGE (not less than) = !(a < b) */
NOINLINE int test_unge(void) {
    volatile double a = darr[0];
    volatile double b = darr[1];
    volatile long double ld1 = ldarr[2];
    volatile long double ld2 = ldarr[3];
    
    int result = 0;
    
    /* Direct UNGE: !(a < b) */
    if (!(a < b)) {
        result |= 1;
    }
    
    /* With float */
    volatile float fa = farr[0];
    volatile float fb = farr[1];
    if (!(fa < fb)) {
        result |= 2;
    }
    
    /* Long double */
    if (!(ld1 < ld2)) {
        result |= 4;
    }
    
    /* Complex: !(a < b) && (a != b) */
    if (!(a < b) && (a != b)) {
        result |= 8;
    }
    
    sink(result);
    return result;
}

/* Pattern 5: UNGT (not less than or equal) = !(a <= b) */
NOINLINE int test_ungt(void) {
    volatile double a = g_two;
    volatile double b = g_one;
    
    int result = 0;
    
    /* Direct UNGT: !(a <= b) */
    if (!(a <= b)) {
        result |= 1;
    }
    
    /* With memory operand */
    if (!(darr[2] <= darr[1])) {
        result |= 2;
    }
    
    /* Using inline assembly */
    volatile double x = g_one;
    volatile double y = g_zero;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : gt_label
    );
    
    result |= 4;
    goto after_label;
    
gt_label:
    result |= 8;
    
after_label:
    sink(result);
    return result;
}

/* Pattern 6: UNLE (unordered or less than or equal) */
NOINLINE int test_unle(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* UNLE: !(a > b) */
    if (!(a > b)) {
        result |= 1;
    }
    
    /* With NaN */
    if (!(nan > b)) {
        result |= 2;
    }
    
    /* Complex: !(a > b) ? 1 : 0 */
    result |= (!(a > b)) ? 4 : 0;
    
    sink(result);
    return result;
}

/* Pattern 7: UNLT (unordered or less than) */
NOINLINE int test_unlt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    
    int result = 0;
    
    /* UNLT: !(a >= b) */
    if (!(a >= b)) {
        result |= 1;
    }
    
    /* With memory operands */
    if (!(darr[0] >= darr[2])) {
        result |= 2;
    }
    
    /* Nested in expression */
    int temp = (!(a >= b)) + (!(b >= a));
    result |= (temp << 2);
    
    sink(result);
    return result;
}

/* Pattern 8: LTGT (less than or greater than - ordered and not equal) */
NOINLINE int test_ltgt(void) {
    volatile double a = g_one;
    volatile double b = g_two;
    volatile double nan = g_nan;
    
    int result = 0;
    
    /* LTGT: (a < b) || (a > b) */
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    /* With NaN (should be false) */
    if ((nan < b) || (nan > b)) {
        result |= 2;
    }
    
    /* Complex ternary */
    result |= ((a < b) || (a > b)) ? 4 : 0;
    
    /* Using inline assembly */
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "st", "cc"
        : ltgt_label
    );
    
    result |= 8;
    goto ltgt_end;
    
ltgt_label:
    result |= 16;
    
ltgt_end:
    sink(result);
    return result;
}

/* Pattern 9: Mixed condition codes in complex expression */
NOINLINE int test_mixed(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_two;
    
    int result = 0;
    
    /* Complex nested conditionals */
    if (isunordered(x, y)) {
        result = 1;
    } else if (!(y < z)) {  /* UNGE */
        result = 2;
    } else if (!(z <= y)) { /* UNGT */
        result = 3;
    } else if ((y < z) || (y > z)) { /* LTGT */
        result = 4;
    }
    
    /* Ternary with multiple conditions */
    int r2 = (x != x) ? 1 : ((y > z) ? 2 : 3);
    result |= (r2 << 4);
    
    /* Logical combination */
    if ((!(y >= z)) && (!(z <= y))) { /* UNLT && UNGT */
        result |= (1 << 8);
    }
    
    sink(result);
    return result;
}

/* Pattern 10: Memory-intensive comparisons */
NOINLINE int test_memory_ops(void) {
    int result = 0;
    
    /* Array element comparisons */
    for (int i = 0; i < 3; i++) {
        if (!(darr[i] < darr[i+1])) {  /* UNGE */
            result |= (1 << i);
        }
    }
    
    /* Struct member comparisons */
    if (!(fp_struct.f < farr[1])) {  /* UNGE for float */
        result |= 0x10;
    }
    
    if ((fp_struct.d < g_one) || (fp_struct.d > g_one)) {  /* LTGT */
        result |= 0x20;
    }
    
    /* Pointer chasing */
    volatile double* p1 = &darr[0];
    volatile double* p2 = &darr[2];
    if (!(*p1 >= *p2)) {  /* UNLT */
        result |= 0x40;
    }
    
    sink(result);
    return result;
}

/* Main function that exercises all patterns */
int main(void) {
    int checksum = 0;
    
    /* Initialize with potential NaN */
    volatile double seed = 0.0;
    if (seed == 0.0) {
        /* Force potential NaN generation */
        g_nan = 0.0/0.0;
        g_inf = 1.0/0.0;
    }
    
    /* Call all test functions */
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_mixed();
    checksum ^= test_memory_ops();
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile int dummy = 0;
    for (int i = 0; i < 10; i++) {
        dummy += checksum;
    }
    sink(dummy);
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* External sink function to prevent optimization */
void sink(int x) {
    volatile static int sink_var;
    sink_var = x;
}

void sink_ptr(void* p) {
    volatile static void* sink_ptr_var;
    sink_ptr_var = p;
}

#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Force non-inlined functions
#define NOINLINE __attribute__((noinline))

// Pattern 1: UNORDERED and ORDERED comparisons
NOINLINE int test_unordered_ordered(void) {
    volatile double nan_val = 0.0 / 0.0;
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal = 3.14;
    
    int result = 0;
    
    // UNORDERED patterns
    if (isunordered(nan_val, normal)) {
        result |= 1;
    }
    
    volatile double d1 = nan_val;
    volatile double d2 = normal;
    if (d1 != d1) {  // Should be true for NaN
        result |= 2;
    }
    
    // ORDERED patterns
    if (isordered(normal, normal)) {
        result |= 4;
    }
    
    if (normal == normal) {  // Should be true for non-NaN
        result |= 8;
    }
    
    // Mixed ordered/unordered
    if (isunordered(nan_val, nan_val) && isordered(normal, normal)) {
        result |= 16;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 2: UNEQ (unordered or equal)
NOINLINE int test_uneq(void) {
    volatile float f1 = 0.0f / 0.0f;  // NaN
    volatile float f2 = 5.0f;
    volatile float f3 = 5.0f;
    
    int result = 0;
    
    // Generate UNEQ: !(a < b) && !(a > b) which is a >= b && a <= b
    // For unordered values, this should also be true
    if (!(f1 < f2) && !(f1 > f2)) {
        result |= 1;
    }
    
    // Direct equality with NaN (always false for ordered comparison)
    // but UNEQ should be true for unordered
    if (f1 == f1) {  // False for NaN with ordered comparison
        // This won't execute, but the comparison generates code
    } else {
        result |= 2;
    }
    
    // Using inline assembly to force condition code
    volatile int flag = 0;
    __asm__ goto (
        "fucomip %%st(1), %%st(0)\n\t"
        "j%c0 %l[uneq_label]"
        : /* no outputs */
        : /* no inputs */
        : "st", "st(1)", "cc"
        : uneq_label
    );
    
    result |= 4;
    goto after_label;
    
uneq_label:
    result |= 8;
    
after_label:
    sink(result);
    checksum += result;
    return result;
}

// Pattern 3: UNGE (not less than) and UNLT (unordered or less than)
NOINLINE int test_unge_unlt(void) {
    volatile double arr[4] = {1.0, 2.0, 0.0/0.0, 4.0};
    volatile double x = arr[0];
    volatile double y = arr[1];
    volatile double nan = arr[2];
    
    int result = 0;
    
    // UNGE: !(a < b) which is a >= b or unordered
    if (!(x < y)) {  // x=1.0, y=2.0, so 1.0 < 2.0 is true, !(true) is false
        // Won't execute for these values
    } else {
        result |= 1;
    }
    
    // UNGE with NaN
    if (!(nan < y)) {  // NaN < 2.0 is false, !(false) is true
        result |= 2;
    }
    
    // UNLT: unordered or less than
    // Using complex expression
    result |= ((x < y) || (x != x)) ? 4 : 0;
    
    // Load from memory to affect addressing
    volatile double* ptr = &arr[3];
    if (!(*ptr >= x)) {  // Generate UNLT: !(a >= b) = a < b or unordered
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 4: UNGT (not less than or equal) and UNLE (unordered or less than or equal)
NOINLINE int test_ungt_unle(void) {
    volatile long double ld1 = 3.14159265358979323846L;
    volatile long double ld2 = 2.71828182845904523536L;
    volatile long double ld_nan = ld1 / 0.0L;  // Infinity
    ld_nan = ld_nan - ld_nan;  // Create NaN
    
    int result = 0;
    
    // UNGT: !(a <= b) which is a > b or unordered
    if (!(ld1 <= ld2)) {  // 3.14 <= 2.71 is false, !(false) is true
        result |= 1;
    }
    
    // UNLE: unordered or less than or equal
    // Using nested ternary
    result |= (ld_nan <= ld1) ? 2 : 
              ((!(ld1 > ld2)) ? 4 : 0);
    
    // Complex expression combining multiple comparisons
    volatile int temp = 0;
    temp = (ld1 != ld1) || (ld1 <= ld2);  // UNLE
    temp = temp && (!(ld2 >= ld1));       // UNGT in another form
    
    result |= temp ? 8 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 5: LTGT (not equal and ordered)
NOINLINE int test_ltgt(void) {
    volatile struct {
        double a;
        double b;
    } s = {5.0, 5.0};
    
    volatile double* pa = &s.a;
    volatile double* pb = &s.b;
    
    int result = 0;
    
    // LTGT: (a < b) || (a > b)  // not equal and ordered
    if ((*pa < *pb) || (*pa > *pb)) {
        // Equal values, so this is false
    } else {
        result |= 1;
    }
    
    // Modify to make unequal
    *pa = 6.0;
    if ((*pa < *pb) || (*pa > *pb)) {
        result |= 2;  // Now true
    }
    
    // With NaN
    volatile double nan = 0.0/0.0;
    if ((nan < s.a) || (nan > s.a)) {
        // NaN comparisons are false, so this won't execute
    } else {
        result |= 4;
    }
    
    // Force condition code with inline assembly
    volatile double a = 10.0;
    volatile double b = 20.0;
    __asm__ goto (
        "fldl %[b]\n\t"
        "fldl %[a]\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        "j%c0 %l[ltgt_label]"
        : /* no outputs */
        : [a] "m" (a), [b] "m" (b)
        : "st", "cc"
        : ltgt_label
    );
    
    result |= 8;
    goto after_ltgt;
    
ltgt_label:
    result |= 16;
    
after_ltgt:
    sink(result);
    checksum += result;
    return result;
}

// Pattern 6: Mixed types and complex expressions
NOINLINE int test_mixed_complex(void) {
    volatile float fa[3] = {1.0f, 2.0f, 0.0f/0.0f};
    volatile double db[3] = {3.0, 4.0, 0.0/0.0};
    
    int result = 0;
    
    // Complex nested expression with multiple condition codes
    result = (fa[0] != fa[0]) ? 1 : 
             ((!(db[0] < db[1])) ? 2 : 
             ((fa[1] <= fa[2] || !(fa[2] >= fa[0])) ? 4 : 8));
    
    // Multiple comparisons in logical expression
    int r1 = !(fa[0] > fa[1]);  // UNLE
    int r2 = !(db[1] <= db[0]); // UNGT
    int r3 = (fa[2] == fa[2]);  // Should be false for NaN
    
    result |= (r1 && r2 && !r3) ? 16 : 0;
    
    // Memory operations with different types
    volatile double* dp = db;
    volatile float* fp = fa;
    
    if (!(*dp < *(dp+1)) && (*fp != *fp)) {
        result |= 32;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 7: Direct inline assembly with condition codes
NOINLINE int test_asm_condition_codes(void) {
    volatile double v1, v2;
    volatile int res = 0;
    
    // Test various condition codes through inline assembly
    v1 = 1.0;
    v2 = 2.0;
    
    // UNORDERED
    __asm__ volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (res)
        : "m" (v1), "m" (v2)
        : "st", "cc", "eax"
    );
    
    checksum += res;
    
    // ORDERED
    v1 = 0.0/0.0;
    __asm__ volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "fstp %%st(0)\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (res)
        : "m" (v1), "m" (v2)
        : "st", "cc", "eax"
    );
    
    checksum += res;
    
    return res;
}

int main(void) {
    printf("Starting condition code tests...\n");
    
    // Initialize with volatile to prevent constant folding
    volatile double seed = 12345.6789;
    volatile float fseed = (float)seed;
    volatile long double ldseed = (long double)seed;
    
    // Call all test functions
    int r1 = test_unordered_ordered();
    int r2 = test_uneq();
    int r3 = test_unge_unlt();
    int r4 = test_ungt_unle();
    int r5 = test_ltgt();
    int r6 = test_mixed_complex();
    int r7 = test_asm_condition_codes();
    
    // Use results to prevent dead code elimination
    volatile int total = r1 + r2 + r3 + r4 + r5 + r6 + r7;
    
    printf("Checksum: %d\n", checksum);
    printf("Total: %d\n", total);
    
    return 0;
}

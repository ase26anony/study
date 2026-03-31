#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Function to generate NaN
double make_nan() {
    volatile double zero = 0.0;
    return zero / zero;
}

// Function to generate infinity
double make_inf() {
    volatile double large = 1e308;
    return large * large;
}

// Pattern 1: UNORDERED comparisons
__attribute__((noinline))
int test_unordered() {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    volatile float f_nan = 0.0f / 0.0f;
    volatile long double ld_nan = 0.0L / 0.0L;
    
    int result = 0;
    
    // Direct unordered checks
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    if (nan2 != nan2) {  // Should be true for NaN
        result |= 2;
    }
    
    // Complex unordered expression
    if (isunordered(f_nan, normal) || isunordered(ld_nan, normal)) {
        result |= 4;
    }
    
    // Inline assembly to force condition code use
    volatile double a = nan1;
    volatile double b = normal;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unordered_label
    );
    
    // Not taken path
    result |= 8;
    goto end;
    
unordered_label:
    // Taken path
    result |= 16;
    
end:
    sink(result);
    checksum += result;
    return result;
}

// Pattern 2: ORDERED comparisons
__attribute__((noinline))
int test_ordered() {
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.5;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Direct ordered checks
    if (isordered(normal1, normal2)) {
        result |= 1;
    }
    
    if (normal1 == normal1) {  // Should be true for non-NaN
        result |= 2;
    }
    
    // Complex ordered expression
    if (isordered(normal1, normal2) && !isunordered(normal1, nan)) {
        result |= 4;
    }
    
    // Inline assembly with ordered condition
    volatile double a = normal1;
    volatile double b = normal2;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ordered_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : ordered_label
    );
    
    result |= 8;
    goto end2;
    
ordered_label:
    result |= 16;
    
end2:
    sink(result);
    checksum += result;
    return result;
}

// Pattern 3: UNEQ (unordered or equal)
__attribute__((noinline))
int test_uneq() {
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Generate UNEQ: !(a != b) which is a == b or unordered
    if (!(a != b)) {
        result |= 1;
    }
    
    // Another UNEQ pattern
    if (isunordered(a, b) || (a == b)) {
        result |= 2;
    }
    
    // Complex expression that should generate UNEQ
    volatile double arr[3] = {1.0, 1.0, make_nan()};
    if (!(arr[0] != arr[1])) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 4: UNGE (not less than)
__attribute__((noinline))
int test_unge() {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Generate UNGE: !(a < b)
    if (!(a < b)) {
        result |= 1;
    }
    
    // UNGE with equal values
    if (!(c < a)) {
        result |= 2;
    }
    
    // Complex UNGE expression
    if (!(a < b) && !(b < a)) {
        result |= 4;
    }
    
    // Inline assembly for UNGE
    volatile double x = a;
    volatile double y = b;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unge_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unge_label
    );
    
    result |= 8;
    goto end4;
    
unge_label:
    result |= 16;
    
end4:
    sink(result);
    checksum += result;
    return result;
}

// Pattern 5: UNGT (not less than or equal)
__attribute__((noinline))
int test_ungt() {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    
    int result = 0;
    
    // Generate UNGT: !(a <= b)
    if (!(a <= b)) {
        result |= 1;
    }
    
    // UNGT with !(b >= a) transformed
    if (!(b >= a)) {
        result |= 2;
    }
    
    // Complex expression
    volatile double arr[2] = {4.0, 2.0};
    if (!(arr[1] <= arr[0])) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 6: UNLE (unordered or less than or equal)
__attribute__((noinline))
int test_unle() {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Generate UNLE: !(a > b)
    if (!(a > b)) {
        result |= 1;
    }
    
    // UNLE with equal values
    if (!(c > a)) {
        result |= 2;
    }
    
    // UNLE with NaN (unordered case)
    if (!(nan > a)) {
        result |= 4;
    }
    
    // Complex ternary with UNLE
    result |= (!(a > b)) ? 8 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 7: UNLT (unordered or less than)
__attribute__((noinline))
int test_unlt() {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Generate UNLT: !(a >= b)
    if (!(a >= b)) {
        result |= 1;
    }
    
    // UNLT with NaN
    if (!(nan >= a)) {
        result |= 2;
    }
    
    // Nested UNLT expression
    if (!(a >= b) && (a < b)) {
        result |= 4;
    }
    
    // Inline assembly for UNLT
    volatile double x = a;
    volatile double y = b;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unlt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unlt_label
    );
    
    result |= 8;
    goto end7;
    
unlt_label:
    result |= 16;
    
end7:
    sink(result);
    checksum += result;
    return result;
}

// Pattern 8: LTGT (less than or greater than - not equal and ordered)
__attribute__((noinline))
int test_ltgt() {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Generate LTGT: (a < b) || (a > b)
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    // LTGT with equal values (should be false)
    if ((c < a) || (c > a)) {
        result |= 2;
    }
    
    // LTGT with NaN (should be false due to unordered)
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    // Complex LTGT expression
    volatile double arr[3] = {1.0, 2.0, 3.0};
    if ((arr[0] < arr[1]) || (arr[0] > arr[1])) {
        result |= 8;
    }
    
    // Inline assembly for LTGT
    volatile double x = a;
    volatile double y = b;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ltgt_label]\n\t"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : ltgt_label
    );
    
    result |= 16;
    goto end8;
    
ltgt_label:
    result |= 32;
    
end8:
    sink(result);
    checksum += result;
    return result;
}

// Pattern 9: Mixed condition codes in complex expressions
__attribute__((noinline))
int test_mixed() {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double nan = make_nan();
    volatile float f1 = 1.0f;
    volatile float f2 = 2.0f;
    volatile long double ld1 = 3.0L;
    volatile long double ld2 = 4.0L;
    
    int result = 0;
    
    // Complex nested ternary with multiple condition codes
    result = (a != a) ? 1 : 
             ((b > a) ? 2 : 
             ((!(a >= b)) ? 3 : 
             ((isunordered(f1, f2)) ? 4 : 
             ((ld1 < ld2) || (ld1 > ld2) ? 5 : 6))));
    
    // Multiple comparisons in logical expression
    if ((!(a < b)) && (isordered(a, b)) && (!(nan == nan))) {
        result |= 8;
    }
    
    // Array-based comparisons with different types
    volatile double darr[4] = {1.0, make_nan(), 3.0, 4.0};
    volatile float farr[4] = {1.0f, 2.0f, 0.0f/0.0f, 4.0f};
    
    for (int i = 0; i < 3; i++) {
        if (!(darr[i] >= darr[i+1])) {
            result |= (1 << (i + 4));
        }
        if (isunordered(farr[i], farr[i+1]) || (farr[i] == farr[i+1])) {
            result |= (1 << (i + 8));
        }
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 10: Struct-based comparisons
typedef struct {
    volatile double x;
    volatile double y;
    volatile float z;
} Point;

__attribute__((noinline))
int test_struct() {
    Point p1 = {1.0, make_nan(), 2.0f};
    Point p2 = {2.0, 3.0, 4.0f};
    
    int result = 0;
    
    // Compare struct members
    if (!(p1.x >= p2.x)) {
        result |= 1;
    }
    
    if (isunordered(p1.y, p2.y)) {
        result |= 2;
    }
    
    if (!(p1.z > p2.z)) {
        result |= 4;
    }
    
    // Complex struct comparison
    if ((!(p1.x >= p2.x)) && isunordered(p1.y, p2.y) && (!(p1.z > p2.z))) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
    return result;
}

int main() {
    printf("Testing i386 condition codes...\n");
    
    // Initialize volatile seed
    volatile int seed = 42;
    srand(seed);
    
    // Call all test functions
    int r1 = test_unordered();
    int r2 = test_ordered();
    int r3 = test_uneq();
    int r4 = test_unge();
    int r5 = test_ungt();
    int r6 = test_unle();
    int r7 = test_unlt();
    int r8 = test_ltgt();
    int r9 = test_mixed();
    int r10 = test_struct();
    
    // Print checksum to ensure all code is live
    printf("Results: %d %d %d %d %d %d %d %d %d %d\n", 
           r1, r2, r3, r4, r5, r6, r7, r8, r9, r10);
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

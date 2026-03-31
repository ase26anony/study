/* test_i386_condition_codes.c
 * Generates RTL with various floating-point condition codes for i386 backend
 * Compile with: gcc -m32 -mfpmath=387 -O2 -da test_i386_condition_codes.c -o test
 */

/* Prevent inlining to keep RTL structure intact */
#define NOINLINE __attribute__((noinline))

/* Helper to create NaN values */
static double make_nan(void) {
    return 0.0 / 0.0;
}

/* Helper to create infinity */
static double make_inf(void) {
    return 1.0 / 0.0;
}

/* Main test function with various FP comparisons */
NOINLINE static int fp_test(volatile double a, volatile double b, 
                           volatile double c, volatile double d) {
    volatile int counter = 0;
    int i;
    
    /* Loop to create multiple RTL instructions */
    for (i = 0; i < 10; i++) {
        /* UNORDERED checks (case UNORDERED: fputs ("unord", file)) */
        if (__builtin_isunordered(a, b)) {
            counter++;
            asm volatile("" : : : "memory");  /* Compiler barrier */
        }
        
        /* ORDERED checks (case ORDERED: fputs ("ord", file)) */
        if (!__builtin_isunordered(a, c)) {
            counter++;
        }
        
        /* UNEQ checks (case UNEQ: fputs ("ueq", file)) */
        /* (a == b) || (a != a && b != b) - unordered equal */
        if (__builtin_isunordered(a, b) || a == b) {
            counter++;
        }
        
        /* UNGE checks (case UNGE: fputs ("nlt", file)) */
        /* !(a < b) - not less than (including unordered) */
        if (!(a < b)) {
            counter++;
        }
        
        /* UNGT checks (case UNGT: fputs ("nle", file)) */
        /* !(a <= b) - not less or equal (including unordered) */
        if (!(a <= b)) {
            counter++;
        }
        
        /* UNLE checks (case UNLE: fputs ("ule", file)) */
        /* (a <= b) || (a != a && b != b) - unordered less or equal */
        if (__builtin_isunordered(a, b) || a <= b) {
            counter++;
        }
        
        /* UNLT checks (case UNLT: fputs ("ult", file)) */
        /* (a < b) || (a != a && b != b) - unordered less than */
        if (__builtin_isunordered(a, b) || a < b) {
            counter++;
        }
        
        /* LTGT checks (case LTGT: fputs ("une", file)) */
        /* (a < b) || (a > b) - less or greater (ordered, not equal) */
        if (a < b || a > b) {
            counter++;
        }
        
        /* Standard ordered comparisons */
        if (a == c) counter++;
        if (a != d) counter++;
        if (a < c)  counter++;
        if (a > d)  counter++;
        if (a <= c) counter++;
        if (a >= d) counter++;
        
        /* Mix in some integer operations to prevent optimization */
        asm volatile("" : "+g"(counter) : : "memory");
    }
    
    return counter;
}

int main(void) {
    /* Use volatile to prevent constant folding */
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    /* Create various combinations of NaN, Inf, and normal values */
    int result = 0;
    
    /* Test 1: NaN vs normal */
    result += fp_test(nan_val, normal1, normal2, zero);
    
    /* Test 2: NaN vs NaN */
    result += fp_test(nan_val, nan_val, normal1, normal2);
    
    /* Test 3: Inf vs normal */
    result += fp_test(inf_val, normal1, normal2, zero);
    
    /* Test 4: normal vs normal */
    result += fp_test(normal1, normal2, zero, inf_val);
    
    /* Test 5: zero vs NaN */
    result += fp_test(zero, nan_val, inf_val, normal1);
    
    /* Use result to prevent dead code elimination */
    if (result > 1000) {
        /* This should never happen, but prevents optimization */
        asm volatile("" : : "r"(result));
    }
    
    return result % 256;  /* Return non-zero to indicate execution */
}

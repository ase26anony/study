#include <stdio.h>
#include <math.h>

#define ARRAY_SIZE 10
static int array[ARRAY_SIZE] = {0};

int main(void) {
    // 1. Declare and initialize variables
    const int n = 5;
    const double base = 2.0;
    int int_result;
    double dbl_result;
    int checksum = 0;
    
    // 2. Direct constant arguments with integer results
    // These should trigger integer_valued_real_call_p during folding
    int_result = pow(2.0, 3.0);           // 8
    checksum += int_result;
    
    dbl_result = sqrt(25.0);              // 5.0
    checksum += (int)dbl_result;
    
    // Use __builtin_ versions explicitly
    int_result = __builtin_exp2(4.0);     // 16
    checksum += int_result;
    
    dbl_result = __builtin_log2(8.0);     // 3.0
    checksum += (int)dbl_result;
    
    // 3. Symbolic arguments with constant relationships
    // n is const, so pow(base, n) should be foldable
    dbl_result = pow(base, n);            // 32.0
    checksum += (int)dbl_result;
    
    // 4. Mixed integer/floating contexts
    // Array indexing with math function results
    array[(int)sqrt(16.0)] = 42;          // array[4] = 42
    checksum += array[4];
    
    // 5. Conditional constant propagation
    // Loop with invariant arguments
    for (int i = 0; i < 3; i++) {
        // pow(3.0, 2.0) is loop invariant and should fold
        double val = pow(3.0, 2.0);       // 9.0
        checksum += (int)val;
        
        // sqrt of perfect square with constant index
        if (i == 1) {
            dbl_result = sqrt(36.0);      // 6.0
            checksum += (int)dbl_result;
        }
    }
    
    // 6. Multiple and nested calls
    // Combined expressions that should fold
    dbl_result = pow(sqrt(64.0), log2(8.0) / log2(2.0));  // 8^3 = 512
    checksum += (int)dbl_result;
    
    // 7. One-argument functions
    dbl_result = __builtin_cbrt(27.0);    // 3.0
    checksum += (int)dbl_result;
    
    dbl_result = __builtin_exp(0.0);      // 1.0
    checksum += (int)dbl_result;
    
    // 8. Trigonometric functions with special values
    dbl_result = sin(0.0);                // 0.0
    checksum += (int)dbl_result;
    
    dbl_result = cos(0.0);                // 1.0
    checksum += (int)dbl_result;
    
    // 9. Comparisons with integer results
    if (pow(2.0, 3.0) == 8) {
        checksum += 100;
    }
    
    if (sqrt(49.0) == 7) {
        checksum += 200;
    }
    
    // 10. More complex expressions
    // atan2 with arguments that yield π/4 (0.785...), not integer
    // but included to test the two-argument path
    dbl_result = atan2(1.0, 1.0);
    checksum += (int)(dbl_result * 1000);  // Scale to get integer part
    
    // 11. Use in control flow
    int index = (int)__builtin_exp2(3.0);  // 8
    if (index < ARRAY_SIZE) {
        array[index] = 99;
        checksum += array[index];
    }
    
    // 12. Print checksum to prevent elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

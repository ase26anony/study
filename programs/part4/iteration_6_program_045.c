#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main() {
    /* 1. Declare variables with constant values */
    const int n = 5;
    const double base = 2.0;
    const int exp_int = 4;
    const double exp_dbl = 4.0;
    int result_int;
    double result_dbl;
    
    /* Array for indexing tests */
    int array[ARRAY_SIZE] = {0};
    int checksum = 0;
    
    /* 2. Direct constant arguments producing integer results */
    /* Two-argument calls (pow) */
    result_int = pow(2.0, 3.0);  /* 8 */
    result_dbl = __builtin_pow(3.0, 2.0);  /* 9 */
    checksum += result_int + (int)result_dbl;
    
    /* One-argument calls */
    result_int = sqrt(25.0);  /* 5 */
    result_dbl = __builtin_sqrt(36.0);  /* 6 */
    checksum += result_int + (int)result_dbl;
    
    result_int = exp2(3.0);  /* 8 */
    result_dbl = __builtin_exp2(4.0);  /* 16 */
    checksum += result_int + (int)result_dbl;
    
    result_int = cbrt(27.0);  /* 3 */
    result_dbl = __builtin_cbrt(64.0);  /* 4 */
    checksum += result_int + (int)result_dbl;
    
    /* 3. Symbolic arguments with constant relationships */
    /* Using const variables */
    result_dbl = pow(base, exp_dbl);  /* 16 */
    result_int = __builtin_pow(base, n);  /* 32 */
    checksum += (int)result_dbl + result_int;
    
    /* Logarithm of powers */
    result_int = log2(8.0);  /* 3 */
    result_dbl = __builtin_log(256.0) / __builtin_log(2.0);  /* 8 */
    checksum += result_int + (int)result_dbl;
    
    /* 4. Use in integer contexts */
    /* Array indexing with math function results */
    array[(int)sqrt(16.0)] = 100;  /* array[4] */
    array[(int)__builtin_pow(2.0, 3.0)] = 200;  /* array[8] */
    
    /* Comparisons with integers */
    if (pow(2.0, 3.0) == 8) {
        checksum += 50;
    }
    if (__builtin_sqrt(81.0) == 9) {
        checksum += 30;
    }
    
    /* 5. Nested calls and combined expressions */
    result_dbl = pow(sqrt(16.0), log(8.0) / log(2.0));  /* 4^3 = 64 */
    result_int = __builtin_exp2(__builtin_log2(32.0));  /* 32 */
    checksum += (int)result_dbl + result_int;
    
    /* Mixed one and two argument calls */
    result_dbl = pow(exp2(2.0), sqrt(9.0));  /* 4^3 = 64 */
    result_int = __builtin_cbrt(pow(8.0, 3.0));  /* 512^(1/3) = 8 */
    checksum += (int)result_dbl + result_int;
    
    /* 6. Conditional constant propagation */
    /* Loop with invariant arguments */
    for (int i = 0; i < 3; i++) {
        /* These calls have constant arguments despite being in loop */
        double loop_val = pow(2.0, i + 1.0);  /* 2, 4, 8 */
        int idx = (int)__builtin_sqrt(loop_val * loop_val);  /* 2, 4, 8 */
        if (idx < ARRAY_SIZE) {
            array[idx] = i * 10;
        }
        checksum += (int)loop_val;
    }
    
    /* 7. Trigonometric functions with special cases */
    /* sin(π) = 0, cos(0) = 1 */
    result_int = sin(3.14159265358979323846);  /* ~0 */
    result_dbl = __builtin_cos(0.0);  /* 1 */
    checksum += (int)(result_int * 1000) + (int)result_dbl;
    
    /* 8. More complex expressions */
    /* (2^3 + √25) * ∛27 = (8 + 5) * 3 = 39 */
    result_int = (pow(2.0, 3.0) + sqrt(25.0)) * cbrt(27.0);
    checksum += result_int;
    
    /* 9. Calculate array checksum */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += array[i];
    }
    
    /* 10. Print final checksum */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

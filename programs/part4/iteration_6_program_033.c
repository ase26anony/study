#include <math.h>
#include <stdio.h>

#define ARRAY_SIZE 20

int main() {
    // Initialize results array
    int results[ARRAY_SIZE] = {0};
    int idx = 0;
    
    // 1. Direct constant arguments producing integer results
    double d1 = pow(2.0, 3.0);          // 8.0
    double d2 = sqrt(25.0);             // 5.0
    double d3 = exp2(4.0);              // 16.0
    double d4 = cbrt(27.0);             // 3.0
    double d5 = log(256.0) / log(2.0);  // 8.0 (log2 via change of base)
    
    // Assign to integer variables (triggers integer-valued check)
    int i1 = pow(3.0, 2.0);             // 9
    int i2 = sqrt(16.0);                // 4
    int i3 = __builtin_exp2(5.0);       // 32 (using __builtin_ version)
    int i4 = __builtin_pow(4.0, 1.5);   // 8 (4^(3/2) = sqrt(4^3) = 8)
    
    results[idx++] = d1;  // Implicit conversion
    results[idx++] = d2;
    results[idx++] = i1;
    results[idx++] = i2;
    
    // 2. Symbolic arguments with constant relationships
    const int n = 6;
    const int m = 3;
    
    // These should fold with constant propagation
    double d6 = pow(2.0, n);            // 64.0
    double d7 = sqrt(n * n);            // 6.0
    double d8 = exp2(m);                // 8.0
    
    int i5 = pow(2.0, m);               // 8
    int i6 = __builtin_sqrt(m * m * 4); // 6 (sqrt(36))
    
    results[idx++] = d6;
    results[idx++] = i5;
    results[idx++] = i6;
    
    // 3. Use in array indexing (triggers integer context)
    double array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    double val1 = array[(int)sqrt(81.0)];          // array[9]
    double val2 = array[(int)__builtin_pow(2.0, 2.0)]; // array[4]
    
    results[idx++] = val1;
    results[idx++] = val2;
    
    // 4. Conditional constant propagation
    int total = 0;
    for (int iter = 0; iter < 3; iter++) {
        // Loop-invariant constants
        const double base = 2.0;
        const double exponent = iter + 3.0;  // 3, 4, 5
        
        // These should fold inside the loop
        double loop_val = pow(base, exponent);  // 8, 16, 32
        total += loop_val;
        
        // Nested call
        if (iter == 1) {
            double nested = sqrt(pow(4.0, 3.0));  // sqrt(64) = 8
            results[idx++] = nested;
        }
    }
    results[idx++] = total;
    
    // 5. Compare to integer (creates integer context)
    int cmp_results = 0;
    if (pow(2.0, 3.0) == 8) cmp_results += 1;
    if (sqrt(100.0) == 10) cmp_results += 2;
    if (exp2(4.0) == 16) cmp_results += 4;
    if (cbrt(125.0) == 5) cmp_results += 8;
    
    results[idx++] = cmp_results;
    
    // 6. Mixed one-argument and two-argument functions
    double mixed1 = pow(sqrt(64.0), log(8.0) / log(2.0));  // 8^3 = 512
    double mixed2 = exp2(log2(32.0));                      // 32
    double mixed3 = __builtin_sqrt(__builtin_pow(6.0, 2.0)); // 6
    
    // Using atan2 which takes two arguments
    double mixed4 = atan2(0.0, 1.0);  // 0.0 (integer result)
    
    results[idx++] = mixed1 / 64;  // Scale down to avoid overflow
    results[idx++] = mixed2;
    results[idx++] = mixed3;
    results[idx++] = mixed4 * 100;  // Scale up for integer storage
    
    // 7. More complex expressions
    const double x = 3.0;
    const double y = 2.0;
    
    // Should fold to integer: (3^2)^(log2(8)/log2(2)) = 9^3 = 729
    double complex_expr = pow(pow(x, y), log(8.0) / log(2.0));
    results[idx++] = complex_expr / 81;  // Scale down
    
    // 8. Trigonometric functions with integer results
    double trig1 = sin(0.0);      // 0
    double trig2 = cos(0.0);      // 1
    double trig3 = __builtin_sin(M_PI);  // ~0 (should be treated as 0 with -ffast-math)
    
    results[idx++] = trig1 * 10;
    results[idx++] = trig2 * 10;
    results[idx++] = trig3 * 10;
    
    // Calculate checksum
    int checksum = 0;
    for (int j = 0; j < idx && j < ARRAY_SIZE; j++) {
        checksum += results[j];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Number of stored results: %d\n", idx);
    
    return 0;
}

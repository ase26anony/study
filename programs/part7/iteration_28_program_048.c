#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

// Global arrays with special floating-point values
float fvals[] = {1.0f, 2.0f, NAN, INFINITY, -INFINITY, 0.0f, -1.0f, 3.0f};
double dvals[] = {1.0, 2.0, NAN, INFINITY, -INFINITY, 0.0, -1.0, 3.0};

// Test scalar comparisons with all relational operators
int test_scalar_cmps(void) {
    int results[64];
    int idx = 0;
    
    // Mix float and double comparisons
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // Use ternary operator to force CMOV/SET generation
            results[idx++] = (fvals[i] < fvals[j]) ? 1 : 0;      // LT
            results[idx++] = (fvals[i] > fvals[j]) ? 2 : 0;      // GT
            results[idx++] = (fvals[i] <= fvals[j]) ? 3 : 0;     // LE
            results[idx++] = (fvals[i] >= fvals[j]) ? 4 : 0;     // GE
            results[idx++] = (fvals[i] == fvals[j]) ? 5 : 0;     // EQ
            results[idx++] = (fvals[i] != fvals[j]) ? 6 : 0;     // NEQ
            
            // Double comparisons
            results[idx++] = (dvals[i] < dvals[j]) ? 7 : 0;
            results[idx++] = (dvals[i] > dvals[j]) ? 8 : 0;
            results[idx++] = (dvals[i] <= dvals[j]) ? 9 : 0;
            results[idx++] = (dvals[i] >= dvals[j]) ? 10 : 0;
            results[idx++] = (dvals[i] == dvals[j]) ? 11 : 0;
            results[idx++] = (dvals[i] != dvals[j]) ? 12 : 0;
        }
    }
    
    // Complex control flow with nested conditionals
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        switch (results[i] % 7) {
            case 0:
                if (fvals[i % 8] < dvals[i % 8]) {
                    sum += results[i];
                    break;
                } else {
                    sum -= results[i];
                    continue;
                }
            case 1:
                sum += results[i] * 2;
                goto skip_mult;
            case 2:
                if (!isnan(fvals[i % 8])) {
                    sum += results[i] * 3;
                }
                break;
            default:
                sum += results[i];
        }
        skip_mult:
        if (isinf(dvals[i % 8])) {
            sum += 100;
        }
    }
    
    return sum;
}

// Test built-in unordered comparison functions
int test_builtins(void) {
    int results[32];
    int idx = 0;
    
    // Test all __builtin_is* functions
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            // These directly map to condition codes
            results[idx++] = __builtin_isgreater(fvals[i], fvals[j]) ? 1 : 0;
            results[idx++] = __builtin_isless(fvals[i], fvals[j]) ? 2 : 0;
            results[idx++] = __builtin_isgreaterequal(fvals[i], fvals[j]) ? 3 : 0;
            results[idx++] = __builtin_islessequal(fvals[i], fvals[j]) ? 4 : 0;
            results[idx++] = __builtin_isunordered(fvals[i], fvals[j]) ? 5 : 0;
            
            // Double versions
            results[idx++] = __builtin_isgreater(dvals[i], dvals[j]) ? 6 : 0;
            results[idx++] = __builtin_isless(dvals[i], dvals[j]) ? 7 : 0;
            results[idx++] = __builtin_isgreaterequal(dvals[i], dvals[j]) ? 8 : 0;
            results[idx++] = __builtin_islessequal(dvals[i], dvals[j]) ? 9 : 0;
            results[idx++] = __builtin_isunordered(dvals[i], dvals[j]) ? 10 : 0;
        }
    }
    
    // Use classification functions
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        switch (fpclassify(fvals[i])) {
            case FP_NAN:
                sum += 1;
                break;
            case FP_INFINITE:
                sum += 2;
                break;
            case FP_ZERO:
                sum += 3;
                break;
            case FP_SUBNORMAL:
                sum += 4;
                break;
            case FP_NORMAL:
                sum += 5;
                break;
        }
        
        // Nested if-else chain with comparisons
        if (isnan(dvals[i])) {
            sum += results[i % 32];
        } else if (isinf(dvals[i])) {
            sum += results[(i + 1) % 32] * 2;
        } else if (dvals[i] > 0) {
            sum += results[(i + 2) % 32] * 3;
        } else if (dvals[i] < 0) {
            sum += results[(i + 3) % 32] * 4;
        } else {
            sum += results[(i + 4) % 32] * 5;
        }
    }
    
    return sum;
}

// Test vector comparisons
int test_vector(void) {
    v4sf va = {1.0f, 2.0f, NAN, INFINITY};
    v4sf vb = {2.0f, 1.0f, INFINITY, NAN};
    v2df vc = {1.0, NAN};
    v2df vd = {NAN, 1.0};
    
    // Vector comparisons generate packed comparison RTL
    v4sf vcmp_lt = va < vb;
    v4sf vcmp_gt = va > vb;
    v4sf vcmp_eq = va == vb;
    v4sf vcmp_neq = va != vb;
    v4sf vcmp_le = va <= vb;
    v4sf vcmp_ge = va >= vb;
    
    v2df vcmp_lt_d = vc < vd;
    v2df vcmp_gt_d = vc > vd;
    v2df vcmp_eq_d = vc == vd;
    
    // Reduce to scalar mask
    int mask = 0;
    float* fptr = (float*)&vcmp_lt;
    for (int i = 0; i < 4; i++) {
        mask |= (*(int*)&fptr[i] != 0) << i;
    }
    
    // Complex control flow with vector results
    int sum = mask;
    for (int i = 0; i < 4; i++) {
        if (fptr[i] != 0.0f) {
            switch (i) {
                case 0:
                    sum += (va[0] < vb[0]) ? 1 : 0;
                    break;
                case 1:
                    sum += (va[1] > vb[1]) ? 2 : 0;
                    break;
                case 2:
                    sum += isnan(va[2]) ? 3 : 0;
                    break;
                case 3:
                    sum += isinf(va[3]) ? 4 : 0;
                    break;
            }
        }
    }
    
    return sum;
}

// Test inline assembly with condition code constraints
int test_asm(void) {
    unsigned char results[16];
    double a = 1.0, b = 2.0;
    double c = NAN, d = INFINITY;
    
    // Test various condition codes via inline assembly
    for (int i = 0; i < 8; i++) {
        // Compare two values
        double x = dvals[i];
        double y = dvals[(i + 1) % 8];
        
        // Use SETcc with different condition codes
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "seta %0\n\t"      // above (greater than, unordered)
            : "=r"(results[i*2])
            : "x"(x), "x"(y)
            : "cc"
        );
        
        __asm__ volatile (
            "comisd %1, %2\n\t"
            "setb %0\n\t"      // below (less than)
            : "=r"(results[i*2 + 1])
            : "x"(x), "x"(y)
            : "cc"
        );
        
        // Test unordered/ordered conditions
        unsigned char unord_result, ord_result;
        __asm__ volatile (
            "comisd %2, %3\n\t"
            "setp %0\n\t"      // parity (unordered)
            "setnp %1\n\t"     // no parity (ordered)
            : "=r"(unord_result), "=r"(ord_result)
            : "x"(c), "x"(d)
            : "cc"
        );
        results[8 + i] = unord_result | (ord_result << 1);
    }
    
    // Sum results
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += results[i];
    }
    
    return sum;
}

int main(void) {
    int checksum = 0;
    
    // Call all test functions
    checksum += test_scalar_cmps();
    checksum += test_builtins();
    checksum += test_vector();
    checksum += test_asm();
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

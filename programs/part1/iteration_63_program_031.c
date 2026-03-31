#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>

// Prevent inlining to force clear call boundaries
#define NOINLINE __attribute__((noinline))
#define OPTIMIZE_O3 __attribute__((optimize("O3")))

// Global volatile variables to prevent optimizations
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;

// External function declarations to force caller-save
extern void external_func1(int, double);
extern void external_func2(long long, float);

// Jump buffer for setjmp/longjmp testing
static jmp_buf jump_buffer;

// ============================================
// Test 1: Complex floating-point calculations with inline asm clobbering
// ============================================
NOINLINE OPTIMIZE_O3
double test1_callee(double a, double b, double c, double d) {
    // Extensive FP calculations using many registers
    double t1 = a * b + c;
    double t2 = b * c - d;
    double t3 = c * d / a;
    double t4 = d * a + b;
    
    // Inline asm that clobbers multiple FP registers
    // For x86: clobber xmm0-xmm5
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory");
    
    double t5 = t1 * t2 + t3;
    double t6 = t2 / t3 - t4;
    
    // More clobbering
    asm volatile("" : : : "xmm6", "xmm7", "memory");
    
    return t5 * t6 + t1 - t2 + t3 * t4;
}

NOINLINE
void test1_caller() {
    // Many live variables across function call
    double v1 = 1.234, v2 = 2.345, v3 = 3.456, v4 = 4.567;
    double v5 = 5.678, v6 = 6.789, v7 = 7.890, v8 = 8.901;
    
    // Use variables before call
    double sum1 = v1 + v2 + v3 + v4;
    double prod1 = v5 * v6 * v7 * v8;
    
    // Function call - requires saving all live FP registers
    double result = test1_callee(v1, v2, v3, v4);
    
    // Use variables after call - they must be restored
    double sum2 = v5 + v6 + v7 + v8;
    double prod2 = v1 * v2 * v3 * v4;
    
    global_accumulator += result + sum1 + sum2 + prod1 + prod2;
}

// ============================================
// Test 2: Mixed int/float operations with loops
// ============================================
NOINLINE
void test2_helper(int a, float b, long long c, double d) {
    // Complex mixed-type expression
    double result = (a * b) + (c / d) - sqrt(fabs(b));
    
    // Clobber general purpose and FP registers
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "memory");
    
    global_accumulator += result;
}

NOINLINE
void test2_caller() {
    // Many variables of different types
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400;
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    long long l1 = 1000LL, l2 = 2000LL, l3 = 3000LL, l4 = 4000LL;
    double d1 = 10.1, d2 = 20.2, d3 = 30.3, d4 = 40.4;
    
    // Loop with function call - creates pressure on caller-save
    for (int i = 0; i < 10; i++) {
        // Modify variables that must survive across call
        i1 += i; i2 -= i; i3 *= (i + 1); i4 /= (i + 2);
        f1 += 0.1f; f2 -= 0.2f; f3 *= 1.1f; f4 /= 1.2f;
        
        // Function call with mixed arguments
        test2_helper(i1, f1, l1, d1);
        
        // Continue using modified variables
        l1 += i1; l2 -= i2; l3 *= i3; l4 /= (i4 + 1);
        d1 += f1; d2 -= f2; d3 *= f3; d4 /= f4;
        
        // Another call with different arguments
        test2_helper(i2, f2, l2, d2);
    }
    
    // Final computation using all variables
    double final_result = i1 + i2 + i3 + i4 + f1 + f2 + f3 + f4 +
                         l1 + l2 + l3 + l4 + d1 + d2 + d3 + d4;
    global_accumulator += final_result;
}

// ============================================
// Test 3: Switch statement with multiple control flow paths
// ============================================
NOINLINE
double test3_case1(int a, double b) {
    asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "memory");
    return a * b + sin(b);
}

NOINLINE
double test3_case2(float a, long long b) {
    asm volatile("" : : : "rcx", "rdx", "xmm2", "xmm3", "memory");
    return a * sqrt(b) + cos(a);
}

NOINLINE
double test3_case3(double a, int b) {
    asm volatile("" : : : "rsi", "rdi", "xmm4", "xmm5", "memory");
    return exp(a) * b + tan(a);
}

NOINLINE
void test3_caller(int mode) {
    // Live variables that must be preserved across switch cases
    int ivar1 = 42, ivar2 = 84, ivar3 = 168;
    float fvar1 = 3.14f, fvar2 = 6.28f, fvar3 = 9.42f;
    double dvar1 = 2.71828, dvar2 = 5.43656, dvar3 = 8.15484;
    long long llvar1 = 1000000LL, llvar2 = 2000000LL;
    
    double result = 0.0;
    
    // Switch creates multiple control flow edges
    switch (mode % 4) {
        case 0:
            // Use some variables before call
            ivar1 *= 2; fvar1 += 1.0f;
            result = test3_case1(ivar1, dvar1);
            // Continue using variables
            ivar2 += ivar1; dvar2 *= result;
            break;
            
        case 1:
            // Different variable usage pattern
            fvar2 /= 2.0f; llvar1 >>= 1;
            result = test3_case2(fvar2, llvar1);
            ivar3 -= (int)result; dvar3 += fvar2;
            break;
            
        case 2:
            // Yet another pattern
            dvar1 = sqrt(dvar1); ivar2 %= 17;
            result = test3_case3(dvar1, ivar2);
            fvar3 *= (float)result; llvar2 += (long long)dvar1;
            break;
            
        case 3:
            // Multiple calls in one case
            result = test3_case1(ivar3, dvar3);
            double r2 = test3_case2(fvar3, llvar2);
            result += test3_case3(result, (int)r2);
            // Complex continuation
            ivar1 = (int)(result * 100);
            fvar1 = (float)(result / 100.0);
            break;
    }
    
    // All variables used after switch - must be restored
    global_accumulator += ivar1 + ivar2 + ivar3 + fvar1 + fvar2 + fvar3 +
                         dvar1 + dvar2 + dvar3 + llvar1 + llvar2 + result;
}

// ============================================
// Test 4: setjmp/longjmp with caller-save requirements
// ============================================
NOINLINE
int test4_callee_with_jmp(int a, double b, float c, long long d) {
    // Many live variables
    int x1 = a * 2, x2 = a + 100;
    double y1 = b * 3.14, y2 = b / 2.71828;
    float z1 = c * 1.5f, z2 = c + 2.5f;
    long long w1 = d << 2, w2 = d >> 1;
    
    if (global_counter++ > 100) {
        // longjmp back - caller must restore registers
        longjmp(jump_buffer, 1);
    }
    
    // Clobber registers
    asm volatile("" : : : "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "memory");
    
    // Use all variables
    return x1 + x2 + (int)y1 + (int)y2 + (int)z1 + (int)z2 + (int)w1 + (int)w2;
}

NOINLINE
void test4_caller() {
    // Variables that must survive longjmp
    int a1 = 10, a2 = 20, a3 = 30;
    double b1 = 1.1, b2 = 2.2, b3 = 3.3;
    float c1 = 4.4f, c2 = 5.5f, c3 = 6.6f;
    long long d1 = 100LL, d2 = 200LL, d3 = 300LL;
    
    if (setjmp(jump_buffer) == 0) {
        // First call
        int r1 = test4_callee_with_jmp(a1, b1, c1, d1);
        
        // Modify variables
        a2 += r1; b2 *= 1.5; c2 -= 0.5f; d2 <<= 1;
        
        // Second call
        int r2 = test4_callee_with_jmp(a2, b2, c2, d2);
        
        // More modifications
        a3 = a1 + a2 + r2;
        b3 = b1 * b2 * r2;
        c3 = c1 + c2 + (float)r2;
        d3 = d1 | d2 | (long long)r2;
        
        // Third call
        int r3 = test4_callee_with_jmp(a3, b3, c3, d3);
        
        global_accumulator += a1 + a2 + a3 + b1 + b2 + b3 + 
                             c1 + c2 + c3 + d1 + d2 + d3 + r1 + r2 + r3;
    } else {
        // After longjmp - all variables must be restored
        global_accumulator += a1 * 2 + a2 * 3 + a3 * 4 +
                             b1 * 5.0 + b2 * 6.0 + b3 * 7.0 +
                             c1 * 8.0f + c2 * 9.0f + c3 * 10.0f +
                             d1 * 11LL + d2 * 12LL + d3 * 13LL;
    }
}

// ============================================
// Test 5: Unreachable code after call
// ============================================
NOINLINE OPTIMIZE_O3
void test5_callee_that_never_returns(int* ptr) {
    *ptr += 100;
    // Inline asm that might affect block termination analysis
    asm volatile("" : : "r"(ptr) : "rax", "rbx", "memory");
    
    if (*ptr > 1000) {
        // This might affect basic block layout
        __builtin_unreachable();
    }
    
    *ptr *= 2;
}

NOINLINE
void test5_caller() {
    int values[10];
    for (int i = 0; i < 10; i++) {
        values[i] = i * 10;
    }
    
    // Multiple calls with live variables
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        int old_val = values[i];
        float fval = (float)values[i] * 1.5f;
        double dval = (double)values[i] * 2.5;
        
        // Call that might have unreachable code
        test5_callee_that_never_returns(&values[i]);
        
        // Continue using variables - must be restored
        sum += values[i] + (int)fval + (int)dval + old_val;
        
        // Additional computation
        if (i % 2 == 0) {
            // More register pressure
            long long lval = (long long)values[i] * 3LL;
            double complex_calc = sin(dval) * cos(fval) * exp(values[i] / 100.0);
            sum += (int)(lval % 1000) + (int)(complex_calc * 100);
        }
    }
    
    global_counter += sum;
}

// ============================================
// Test 6: Deep expression nesting with calls
// ============================================
NOINLINE
double test6_helper1(double a, double b) {
    asm volatile("" : : : "xmm0", "xmm1", "xmm2", "memory");
    return a * b + sin(a) * cos(b);
}

NOINLINE
double test6_helper2(double a, double b) {
    asm volatile("" : : : "xmm3", "xmm4", "xmm5", "memory");
    return a / b + tan(a) * atan(b);
}

NOINLINE OPTIMIZE_O3
void test6_caller() {
    // Deeply nested expressions with function calls
    double a = 1.0, b = 2.0, c = 3.0, d = 4.0;
    double e = 5.0, f = 6.0, g = 7.0, h = 8.0;
    
    // Complex expression with calls interspersed
    double result = 
        (test6_helper1(a, b) * test6_helper2(c, d)) +
        (test6_helper1(e, f) / test6_helper2(g, h)) -
        (test6_helper2(a, c) * test6_helper1(b, d)) +
        (test6_helper2(e, g) / test6_helper1(f, h));
    
    // More computations using original variables
    result += a + b + c + d + e + f + g + h;
    
    // Nested conditional with calls
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            result += test6_helper1(result, a + i);
        } else {
            result -= test6_helper2(result, b + i);
        }
        
        // Modify variables in loop
        a += 0.1; b -= 0.2; c *= 1.1; d /= 1.2;
    }
    
    global_accumulator += result;
}

// ============================================
// Main orchestrator
// ============================================
int main() {
    printf("Starting caller-save restoration test...\n");
    
    // Run all tests multiple times to increase coverage
    for (int iteration = 0; iteration < 3; iteration++) {
        printf("Iteration %d:\n", iteration + 1);
        
        // Test 1: FP calculations with asm clobbering
        test1_caller();
        printf("  Test1 complete, accumulator: %f\n", global_accumulator);
        
        // Test 2: Mixed types with loops
        test2_caller();
        printf("  Test2 complete, accumulator: %f\n", global_accumulator);
        
        // Test 3: Switch with multiple paths
        test3_caller(iteration);
        printf("  Test3 complete, accumulator: %f\n", global_accumulator);
        
        // Test 4: setjmp/longjmp
        test4_caller();
        printf("  Test4 complete, accumulator: %f\n", global_accumulator);
        
        // Test 5: Unreachable code
        test5_caller();
        printf("  Test5 complete, counter: %d\n", global_counter);
        
        // Test 6: Deep expression nesting
        test6_caller();
        printf("  Test6 complete, accumulator: %f\n", global_accumulator);
    }
    
    // Final validation
    double final_check = global_accumulator + global_counter;
    printf("\nFinal validation checksum: %f\n", final_check);
    
    // Prevent dead code elimination
    if (final_check != 0.0) {
        printf("Test completed successfully.\n");
    } else {
        printf("Warning: Possible dead code elimination detected.\n");
    }
    
    return 0;
}

// Dummy external functions
void external_func1(int a, double b) {
    global_counter += a + (int)b;
}

void external_func2(long long a, float b) {
    global_accumulator += (double)a + (double)b;
}

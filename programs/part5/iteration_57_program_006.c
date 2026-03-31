#include <stdio.h>
#include <stdlib.h>

// Global variables to create loop-invariant values
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

// Non-inlineable function with higher latency
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    // Division has higher latency than basic arithmetic
    return (x % y) ? (x / y) : (x * y);
}

// Hot function containing the target loop
__attribute__((hot, noinline))
int target_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    // Loop with high iteration count
    for (int i = 2; i < n - 2; i++) {
        // Many scalar temporaries creating register pressure
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
        
        // Chain of dependent operations with loop-invariant values
        t0 = a[i] + g_invariant1;           // Uses invariant
        t1 = t0 * b[i];
        t2 = t1 - g_invariant2;             // Uses another invariant
        t3 = high_latency_op(t2, 3);        // Higher latency operation
        t4 = t3 + c[i];
        t5 = t4 * g_invariant3;             // Uses third invariant
        t6 = t5 - a[i-1];                   // Array access with offset
        t7 = t6 + b[i+1];                   // Another offset access
        t8 = t7 * c[i-2];                   // Different offset
        t9 = t8 - a[i+2];
        t10 = t9 + b[i-1];
        t11 = high_latency_op(t10, 5);      // Another high latency op
        t12 = t11 * c[i+1];
        t13 = t12 - g_invariant1;           // Reuse invariant
        t14 = t13 + g_invariant2;           // Reuse another invariant
        t15 = high_latency_op(t14, 7);      // Final high latency
        
        // Conditional creating multiple basic blocks
        if (t15 & 1) {
            // Path 1: More dependent operations
            int u0 = t15 * 3;
            int u1 = u0 + g_invariant3;
            int u2 = high_latency_op(u1, 11);
            int u3 = u2 - a[i];
            result += u3;
        } else {
            // Path 2: Different chain of operations
            int v0 = t15 / 2;
            int v1 = v0 * g_invariant1;
            int v2 = high_latency_op(v1, 13);
            int v3 = v2 + b[i];
            result -= v3;
        }
        
        // Additional computation to increase register pressure
        int extra1 = t5 * t10;
        int extra2 = extra1 + t15;
        int extra3 = high_latency_op(extra2, 17);
        result ^= extra3;
    }
    
    return result;
}

// Initialize arrays with deterministic pseudo-random values
void init_arrays(int* a, int* b, int* c, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        // Simple LCG
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    const int SIZE = 1024;
    int a[SIZE], b[SIZE], c[SIZE];
    
    // Initialize arrays
    init_arrays(a, b, c, SIZE);
    
    // Call hot function multiple times to ensure it's compiled as hot
    int total_result = 0;
    for (int iter = 0; iter < 10; iter++) {
        // Modify invariants slightly each iteration
        g_invariant1 = (g_invariant1 * 3) % 31;
        g_invariant2 = (g_invariant2 * 5) % 29;
        g_invariant3 = (g_invariant3 * 7) % 23;
        
        total_result += target_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}

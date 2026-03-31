#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure - marked to prevent optimizations
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                         volatile int c, volatile int d) {
    // Force many pseudo registers by declaring many local variables
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
    
    // Complex expression that will be reused - good rematerialization candidate
    volatile int base_expr = (a * b) + (c << 2) - d;
    
    // Loop with volatile iteration count to create multiple basic blocks
    volatile int iterations = 10;
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        // Compiler barrier to prevent reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // Long sequence of independent arithmetic operations
        // Each writes to a distinct variable to keep them all live
        r0 = base_expr + i;
        r1 = base_expr * i;
        r2 = r0 ^ r1;
        r3 = r1 - r0;
        r4 = (r2 << 3) | r3;
        r5 = r3 * r4;
        r6 = r4 ^ r5;
        r7 = r5 - r6;
        r8 = (r6 << 2) | r7;
        r9 = r7 * r8;
        r10 = r8 ^ r9;
        r11 = r9 - r10;
        r12 = (r10 << 1) | r11;
        r13 = r11 * r12;
        r14 = r12 ^ r13;
        r15 = r13 - r14;
        r16 = (r14 << 4) | r15;
        r17 = r15 * r16;
        r18 = r16 ^ r17;
        r19 = r17 - r18;
        r20 = (r18 << 2) | r19;
        r21 = r19 * r20;
        r22 = r20 ^ r21;
        r23 = r21 - r22;
        r24 = (r22 << 3) | r23;
        r25 = r23 * r24;
        r26 = r24 ^ r25;
        r27 = r25 - r26;
        r28 = (r26 << 1) | r27;
        r29 = r27 * r28;
        
        // Reuse the complex expression multiple times - creates copies
        // that are candidates for rematerialization
        int copy1 = base_expr;
        int copy2 = base_expr;
        int copy3 = base_expr;
        int copy4 = base_expr;
        int copy5 = base_expr;
        
        // More operations using the copies
        r0 = copy1 + r29;
        r1 = copy2 * r28;
        r2 = copy3 ^ r27;
        r3 = copy4 - r26;
        r4 = copy5 | r25;
        
        // Compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Conditional branch to split basic blocks
        volatile int condition = a & (1 << i);
        if (condition) {
            // Different computation path
            r5 = (copy1 * 3) + (copy2 >> 1);
            r6 = (copy3 << 2) ^ copy4;
            r7 = copy5 - (i * 7);
            result += r5 + r6 + r7;
        } else {
            // Alternative path
            r8 = (copy1 >> 2) * copy2;
            r9 = copy3 ^ (copy4 << 1);
            r10 = copy5 + (i * 11);
            result += r8 + r9 + r10;
        }
        
        // More operations to extend live ranges
        r11 = r0 + r1 + r2 + r3 + r4;
        r12 = r5 * r6 * r7 * r8 * r9;
        r13 = r10 ^ r11 ^ r12;
        r14 = (r13 << i) | (r13 >> (32 - i));
        r15 = r14 * base_expr;
        
        // Use all variables to prevent dead code elimination
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                 r10 + r11 + r12 + r13 + r14 + r15;
                 
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Use volatile inputs to prevent constant propagation
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    
    // Call the high-pressure function
    volatile int result = high_pressure_compute(v1, v2, v3, v4);
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}

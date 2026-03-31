#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Function with high register pressure and rematerialization opportunities
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int e, volatile int f)
{
    volatile int iterations = 10;
    volatile int result = 0;
    
    // Force many pseudo registers with independent computations
    for (volatile int i = 0; i < iterations; i++) {
        // Declare many local variables to create register pressure
        int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
        int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
        
        // Complex expression that will be reused (rematerialization candidate)
        // This creates a pattern where recomputing might be cheaper than spilling
        int common_expr = (a * b) + (c << 2) - (d & 0xFF) + (e | 0x1F);
        
        // First set of independent computations
        r0 = a + b + i;
        r1 = b * c - i;
        r2 = c + d * 3;
        r3 = d - e / 2;
        r4 = e | f;
        r5 = f & a;
        r6 = a ^ b ^ c;
        r7 = b << (c & 3);
        r8 = c >> (d & 3);
        r9 = d * e + f;
        
        // Compiler barrier to prevent reordering/coalescing
        __asm__ volatile ("" : : : "memory");
        
        // Reuse the common expression multiple times
        // This creates register copies that are candidates for rematerialization
        r10 = common_expr + r0;
        r11 = common_expr - r1;
        r12 = common_expr * r2;
        r13 = common_expr & r3;
        r14 = common_expr | r4;
        
        // More independent computations
        r15 = r0 * r1 + r2;
        r16 = r3 - r4 * r5;
        r17 = r6 & r7 | r8;
        r18 = r9 ^ r10 ^ r11;
        r19 = r12 + r13 - r14;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // Conditional branch to split basic blocks
        volatile int condition = a & 1;
        if (condition) {
            // Different computations in the taken branch
            r20 = r15 * 2 + r16;
            r21 = r17 / 2 - r18;
            r22 = r19 & 0xFFFF;
            r23 = r20 | 0xFF00;
            r24 = r21 ^ 0xAAAA;
            
            // Reuse common expression again in this branch
            r25 = common_expr + r22;
            r26 = common_expr - r23;
            r27 = common_expr * r24;
            
            // More computations
            r28 = r25 + r26 + r27;
            r29 = r28 * 3 - i;
            
            result += r29;
        } else {
            // Different computations in the not-taken branch
            r20 = r16 * 3 + r17;
            r21 = r18 / 3 - r19;
            r22 = r20 & 0xAAAA;
            r23 = r21 | 0x5555;
            r24 = r22 ^ 0xCCCC;
            
            // Reuse common expression in this branch too
            r25 = common_expr + r20;
            r26 = common_expr - r21;
            r27 = common_expr * r22;
            
            // More computations
            r28 = r25 - r26 + r27;
            r29 = r28 * 5 + i;
            
            result += r29;
        }
        
        // Final computations merging both paths
        int final_calc = r29 * 2 + common_expr;
        
        // Another compiler barrier
        __asm__ volatile ("" : : : "memory");
        
        // More independent expressions to increase pressure
        int t1 = final_calc + a;
        int t2 = final_calc - b;
        int t3 = final_calc * c;
        int t4 = final_calc & d;
        int t5 = final_calc | e;
        int t6 = t1 + t2 + t3;
        int t7 = t4 - t5 + t6;
        int t8 = t7 * 7 - i;
        int t9 = t8 & 0xFFFFFF;
        
        result += t9;
        
        // Modify inputs slightly for next iteration
        a = a + 1;
        b = b - 1;
        c = c ^ 0x01;
    }
    
    return result;
}

int main(void)
{
    srand(time(NULL));
    
    // Initialize volatile inputs to prevent constant propagation
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int v5 = rand() % 100 + 1;
    volatile int v6 = rand() % 100 + 1;
    
    printf("Inputs: %d, %d, %d, %d, %d, %d\n", 
           v1, v2, v3, v4, v5, v6);
    
    // Call the high-pressure function
    int result = high_pressure_compute(v1, v2, v3, v4, v5, v6);
    
    printf("Result: %d\n", result);
    
    return 0;
}

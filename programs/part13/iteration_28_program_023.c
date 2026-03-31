#include <stdio.h>
#include <stdlib.h>

#define SIZE 100
#define INNER_SIZE 50

volatile int g_outer_cond = 1;
volatile int g_middle_cond = 1;
volatile int g_inner_cond = 1;
volatile int g_counter = 0;
volatile int g_prevent_opt = 0;

int main() {
    int array1[SIZE][SIZE];
    int array2[SIZE][INNER_SIZE];
    int result = 0;
    
    // Initialize arrays with some values
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            array1[i][j] = (i * j) % 100;
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            array2[i][j] = (i + j) % 50;
        }
    }
    
    // Complex nested loop structure with partial overlaps
    // Outer loop - Level 1
    for (int outer = 0; outer < SIZE; outer += 2) {
        // Multiple basic blocks in outer loop body
        if (g_outer_cond) {
            // First basic block in outer loop
            int temp = array1[outer][0];
            
            // Middle loop - Level 2 (not strictly contained in outer)
            // This loop starts in outer but has blocks outside
            int middle_start = outer % 3;
            while (middle_start < INNER_SIZE) {
                // Multiple basic blocks in middle loop
                if (g_middle_cond) {
                    // First basic block in middle loop
                    int middle_temp = array2[outer][middle_start];
                    
                    // Inner loop - Level 3 (not strictly contained in middle)
                    // This loop sometimes executes based on condition
                    for (int inner = 0; inner < middle_start + 5; inner++) {
                        // Complex inner loop body with multiple basic blocks
                        if (inner % 2 == 0) {
                            // Branch 1
                            result += array1[inner][outer];
                            g_counter++;
                            
                            if (g_inner_cond && inner > 2) {
                                // Nested if creates another basic block
                                result -= array2[outer][inner % INNER_SIZE];
                            }
                        } else {
                            // Branch 2
                            result ^= array1[outer][inner % SIZE];
                            g_prevent_opt = result;
                            
                            // Early continue creates control flow complexity
                            if (result % 7 == 0) continue;
                            
                            // Additional computation
                            result |= array2[inner % SIZE][middle_start % INNER_SIZE];
                        }
                        
                        // Another basic block after the if-else
                        g_counter += (inner % 3);
                    }
                    
                    // Block after inner loop in middle loop
                    middle_temp = result % 100;
                    array2[outer][middle_start] = middle_temp;
                    
                    // Break condition that depends on volatile
                    if (g_counter > 1000) {
                        // goto to create additional basic blocks
                        goto middle_loop_end;
                    }
                } else {
                    // Alternative path in middle loop
                    result = (result * 3) % 1000;
                }
                
                middle_start += (g_counter % 4) + 1;
                
                // Label for goto target
                middle_loop_end:
                if (middle_start > INNER_SIZE / 2) {
                    // Sometimes break early
                    break;
                }
            }
            
            // Block after middle loop in outer loop
            temp = result;
            array1[outer][0] = temp;
            
        } else {
            // Alternative path in outer loop - NO inner loops here
            // This creates partial overlap scenario
            for (int alt = 0; alt < 10; alt++) {
                result += alt * outer;
                g_prevent_opt = result;
            }
        }
        
        // Another loop in outer but not containing the middle loop
        // This creates complex bitmap relationships
        do {
            result = (result + 1) % 10000;
            g_counter++;
            
            if (result % 13 == 0) {
                // Nested if in do-while
                break;
            }
        } while (g_counter < 100 && result < 5000);
        
        // Final basic block in outer loop
        g_outer_cond = (result % 2);
    }
    
    // Additional loop structure to create more complex relationships
    // This loop shares some blocks with previous structure
    int control = 0;
    while (control < SIZE / 2) {
        // This loop body partially overlaps with previous outer loop
        if (control % 3 == 0) {
            // Reuse some computation pattern
            for (int k = 0; k < control + 2; k++) {
                result += array1[control][k];
                g_prevent_opt = result;
                
                // Nested conditional
                if (k % 4 == 0) {
                    result -= array2[k % SIZE][control % INNER_SIZE];
                }
            }
        }
        
        control += (g_counter % 3) + 1;
        
        // Volatile condition prevents optimization
        if (g_inner_cond) {
            g_counter = (g_counter * 2) % 100;
        }
    }
    
    // Prevent dead code elimination
    printf("Result: %d\n", result);
    printf("Counter: %d\n", g_counter);
    printf("PreventOpt: %d\n", g_prevent_opt);
    
    // Use results to prevent optimization
    return (result + g_counter + g_prevent_opt) % 100;
}

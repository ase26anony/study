#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

volatile int global_counter = 0;
int A[SIZE], B[SIZE];
int C[SIZE], D[SIZE];

__attribute__((noinline, noclone))
void modify_globals(int* ptr1, int* ptr2, int idx) {
    global_counter++;
    *ptr1 = *ptr2 + idx + global_counter;
}

__attribute__((noinline, noclone))
int complex_condition(int x, int y, int* restrict p1, int* restrict p2) {
    // Use restrict to prevent certain optimizations while maintaining defined behavior
    return (x * y + *p1 - *p2) & 0x3F;
}

int main(void) {
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = SIZE - i;
        C[i] = i * 2;
        D[i] = i * 3;
    }
    
    int *p1 = A;
    int *p2 = B;
    int *p3 = C;
    int *p4 = D;
    
    int i = 0, j = 0, k = 0;
    int checksum = 0;
    
    // Start of complex loop structure with partial overlap
    
    // LOOP L1 - for loop with irregular control flow
    for (i = 0; i < SIZE; i++) {
        // Basic block shared with L2
        int cond1 = complex_condition(i, j, &A[i], &B[j]);
        
        if (cond1 < 10) {
            // Jump into middle of L2
            goto ENTER_L2_MIDDLE;
        }
        
        // Another basic block that might be shared
        modify_globals(&A[i], &B[j], i);
        
        if (cond1 > 50) {
            // Early continue that jumps to a label
            goto L1_CONTINUE_POINT;
        }
        
        // Regular computation
        A[i] = B[j] * C[k] + global_counter;
        
        // Switch statement creating multiple exit points
        switch (cond1 & 0x3) {
            case 0:
                // Fall through to L2
                j = (j + 1) % SIZE;
                goto L2_HEADER;
            case 1:
                // Continue in L1
                break;
            case 2:
                // Jump to error handling block (shared)
                goto ERROR_HANDLER;
            case 3:
                // Complex path
                k = (k + 1) % SIZE;
                if (k % 7 == 0) {
                    goto L2_FOOTER;
                }
                break;
        }
        
        L1_CONTINUE_POINT:
        // Shared basic block
        checksum += A[i] & 0xFF;
    }
    
    // After L1 normally
    goto FINAL_COMPUTATION;
    
    // LOOP L2 - while loop that overlaps with L1
    L2_HEADER:
    while (j < SIZE) {
        ENTER_L2_MIDDLE:  // Entry point from L1
        // Shared basic block with L1
        int cond2 = complex_condition(j, i, &B[j], &A[i]);
        
        // Multiple exit conditions
        if (cond2 == 0) {
            // Jump back to L1 header
            i = (i + 1) % SIZE;
            goto L1_HEADER_RETURN;
        }
        
        // Complex computation with pointer aliasing
        // p1 and p3 may alias in compiler's view
        *p1++ = *p3++ + cond2;
        p1 = (p1 == &A[SIZE]) ? A : p1;
        p3 = (p3 == &C[SIZE]) ? C : p3;
        
        // Nested do-while inside while
        int m = 0;
        do {
            // Another shared block
            modify_globals(&B[j], &D[m], j);
            m++;
            
            if (m > 3) {
                // Break to outer scope
                goto BREAK_DO_WHILE;
            }
            
            // Conditional continue
            if ((j + m) % 5 == 0) {
                continue;
            }
            
            D[m] = A[i] * B[j];
        } while (m < 4);
        
        BREAK_DO_WHILE:
        
        if (cond2 > 30 && cond2 < 40) {
            // Jump to shared error handler
            goto ERROR_HANDLER;
        }
        
        // Another possible path to L1
        if (cond2 % 7 == 0) {
            i++;
            if (i < SIZE) {
                goto L1_HEADER_RETURN;
            }
        }
        
        j++;
        
        L2_FOOTER:
        // Footer logic - shared with some L1 paths
        checksum += B[j-1] & 0xFF;
        
        // Conditional break
        if (global_counter > 1000) {
            goto FINAL_COMPUTATION;
        }
    }
    
    // Jump back to L1 if we exit L2 normally
    i++;
    if (i < SIZE) {
        goto L1_HEADER_RETURN;
    }
    
    goto FINAL_COMPUTATION;
    
    // Shared error handling block
    ERROR_HANDLER:
    {
        // This block is reachable from both loops
        volatile int error_code = global_counter % 256;
        checksum += error_code;
        
        // Different return paths based on conditions
        if (i < j) {
            j++;
            goto L2_HEADER;
        } else {
            i++;
            goto L1_HEADER_RETURN;
        }
    }
    
    // Return point to L1 header
    L1_HEADER_RETURN:
    if (i < SIZE) {
        // This creates a back edge to L1 but not through the normal for loop
        goto L1_BODY_START;
    }
    
    goto FINAL_COMPUTATION;
    
    // Start label for L1 body (bypassing initialization)
    L1_BODY_START:
    // This creates an irregular loop structure
    {
        int cond1 = complex_condition(i, j, &A[i], &B[j]);
        
        if (cond1 < 10) {
            goto ENTER_L2_MIDDLE;
        }
        
        modify_globals(&A[i], &B[j], i);
        
        // Continue with the rest of L1 body...
        // (In a full implementation, this would duplicate the L1 body logic)
        // For brevity, we'll jump to the actual computation
        A[i] = B[j] * C[k] + global_counter;
        checksum += A[i] & 0xFF;
        i++;
        
        if (i < SIZE) {
            goto L1_BODY_START;
        }
    }
    
    FINAL_COMPUTATION:
    // Final checksum computation to prevent elimination
    int final_sum = 0;
    for (int idx = 0; idx < SIZE; idx++) {
        final_sum += A[idx] + B[idx] + C[idx] + D[idx];
    }
    
    printf("Checksum: %d, Final sum: %d, Global counter: %d\n", 
           checksum, final_sum, global_counter);
    
    return 0;
}

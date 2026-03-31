#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

volatile int global_counter = 0;
int A[SIZE], B[SIZE], C[SIZE];

__attribute__((noinline, noclone))
void modify_global(int *ptr1, int *ptr2, int idx) {
    global_counter++;
    *ptr1 = *ptr2 + idx + global_counter;
}

__attribute__((noinline, noclone))
int complex_condition(int x, int y, volatile int *mod) {
    (*mod)++;
    return (x ^ y) & (*mod);
}

int main() {
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = SIZE - i;
        C[i] = 0;
    }
    
    int *ptr1 = A;
    int *ptr2 = B;
    int *ptr3 = C;
    
    volatile int mod = 1;
    int checksum = 0;
    
    // Loop L1 - for loop with irregular control flow
    int i = 0;
    int j = 0;
    
L1_HEADER:
    for (; i < SIZE; i++) {
        // Basic block shared with L2
        int cond1 = complex_condition(i, j, &mod);
        
        if (cond1 & 0x1) {
            // Jump into middle of L2
            goto L2_MIDDLE;
        }
        
        // Another shared basic block
        modify_global(&A[i], &B[j], i);
        
        if (cond1 & 0x2) {
            // Early continue
            continue;
        }
        
        // Complex switch that can jump to different loop headers
        switch (cond1 & 0x3) {
            case 0:
                // Normal path
                C[i] = A[i] * B[j];
                break;
            case 1:
                // Jump to L2 header
                j++;
                goto L2_HEADER;
            case 2:
                // Break to outer scope
                goto AFTER_L1;
            case 3:
                // Continue with special handling
                modify_global(&B[j], &C[i], j);
                continue;
        }
        
        // Conditional break
        if (A[i] > SIZE/2) {
            break;
        }
        
        // Loop L2 - while loop that partially overlaps with L1
        j = 0;
L2_HEADER:
        while (j < SIZE) {
            // Shared basic block
            int cond2 = complex_condition(i, j, &mod);
            
            if (cond2 & 0x4) {
                // Jump back to L1 header
                goto L1_HEADER;
            }
            
L2_MIDDLE:
            // Another shared block
            modify_global(&B[j], &C[i], j);
            
            // Multiple exit points
            if (cond2 & 0x8) {
                // Exit to L1's body
                i++;
                goto L1_CONTINUE;
            }
            
            // Do-while style check
            do {
                // Innermost block that can be reached from both loops
                int temp = A[i] + B[j];
                if (temp & 0x1) {
                    // Jump to switch in L1
                    goto L1_SWITCH;
                }
                C[i] += temp;
                j++;
            } while (j % 10 != 0 && j < SIZE);
            
            // Conditional continue with goto
            if (j % 7 == 0) {
                goto L2_HEADER;
            }
            
            // Normal increment
            j++;
        }
        
L1_CONTINUE:
        // Continuation of L1
        if (i % 3 == 0) {
            // Another shared block
            modify_global(&C[i], &A[i], i);
        }
        
L1_SWITCH:
        // Empty label for goto target
        ;
    }

AFTER_L1:
    // Second pair of overlapping loops
    int x = 0, y = 0;
    int *p1 = A;
    int *p2 = B;
    
LOOP_X:
    while (x < SIZE/2) {
        // Shared initialization
        int val = complex_condition(x, y, &mod);
        
        if (val & 0x10) {
            // Enter LOOP_Y in the middle
            y = x;
            goto LOOP_Y_MID;
        }
        
LOOP_Y:
        for (y = 0; y < SIZE/4; y++) {
            // Shared computation
            *p1 = *p2 + val;
            
LOOP_Y_MID:
            // Middle entry point
            if (complex_condition(x, y, &mod) & 0x20) {
                // Jump to LOOP_X header
                x++;
                goto LOOP_X;
            }
            
            // Pointer aliasing to prevent optimization
            if (x == y) {
                p1 = &B[y];
                p2 = &C[x];
            } else {
                p1 = &A[x];
                p2 = &B[y];
            }
            
            // Memory access with variant stride
            p1[x % 16] = p2[y % 16] * (x + y);
            
            // Conditional break to different scope
            if ((x * y) % 100 == 0) {
                goto LOOP_X_BODY;
            }
        }
        
LOOP_X_BODY:
        // Body continuation
        x++;
        
        // Nested do-while that shares blocks
        int k = 0;
        do {
            // Block reachable from both loops
            if (k == x % 5) {
                goto LOOP_Y;
            }
            C[k] += A[x] * B[y];
            k++;
        } while (k < 5);
    }
    
    // Compute checksum
    for (int idx = 0; idx < SIZE; idx++) {
        checksum ^= A[idx] + B[idx] + C[idx];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}

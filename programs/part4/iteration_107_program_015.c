#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

volatile int global_counter = 0;
int A[SIZE], B[SIZE], C[SIZE];

__attribute__((noinline, noclone))
void modify_global(int *ptr1, int *ptr2) {
    global_counter++;
    *ptr1 = *ptr2 + global_counter;
}

__attribute__((noinline, noclone))
void side_effect(int x) {
    volatile static int sink = 0;
    sink += x;
}

int main(void) {
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = SIZE - i;
        C[i] = 0;
    }
    
    int *p1 = A;
    int *p2 = B;
    int *p3 = C;
    
    int i = 0, j = 0, k = 0;
    
    // Loop L1 - for loop with irregular control flow
    for (i = 0; i < SIZE; i++) {
        side_effect(i);
        
        // Shared basic block with L2
        if (i % 3 == 0) {
            // Jump into middle of L2
            goto enter_L2_mid;
        }
        
        // Normal L1 processing
        modify_global(&A[i], &B[i]);
        
        if (A[i] % 7 == 0) {
            // Another entry point to L2
            j = i % 100;
            goto L2_start;
        }
        
        continue;
        
    L2_continue_back:
        // Return point from L2
        p3[i] = A[i] + B[i];
    }
    
    goto after_loops;
    
    // Loop L2 - while loop with multiple exits
    L2_start:
    while (j < SIZE) {
        side_effect(j * 2);
        
        // Shared computation block
        int temp = B[j] * 3;
        
        if (temp % 5 == 0) {
            // Jump back to L1 header
            i = (i + 1) % SIZE;
            goto L1_continue;
        }
        
        // Pointer aliasing creates complex dependencies
        p1 = (j % 2) ? &A[j] : &B[j];
        p2 = (j % 3) ? &B[j] : &C[j];
        
        modify_global(p1, p2);
        
        // Complex switch with goto to different loops
        switch (j % 4) {
            case 0:
                // Continue normally
                break;
            case 1:
                // Jump to L1's processing block
                goto L1_processing;
            case 2:
                // Jump to shared block
                goto shared_block;
            case 3:
                // Early exit to after loops
                if (j > SIZE/2) goto after_loops;
                break;
        }
        
        j++;
        continue;
        
    enter_L2_mid:
        // Entry from L1
        temp = A[i] + B[j];
        goto L2_continue;
        
    shared_block:
        // Block shared by both loops
        C[j] = A[i] + B[j];
        side_effect(C[j]);
        goto L2_continue;
        
    L2_continue:
        j++;
        if (j < SIZE && (j % 50 == 0)) {
            // Jump back to L1's continue point
            goto L2_continue_back;
        }
    }
    
    goto after_loops;
    
L1_processing:
    // Another block reachable from both loops
    A[i] = B[j] * 2;
    if (i < SIZE - 1) {
        i++;
        goto L1_continue;
    }
    
L1_continue:
    // L1's continue target
    if (i < SIZE) {
        // Do-while style check
        do {
            side_effect(A[i]);
            i++;
        } while (i % 10 != 0 && i < SIZE);
        goto L2_continue_back;
    }
    
after_loops:
    // Third loop that overlaps with both L1 and L2
    int m = 0, n = 0;
    
    // Loop L3 - do-while with irregular nesting
    m = 0;
    do {
        if (m % 2 == 0) {
            // Jump into L1's range
            i = m;
            goto L1_processing;
        }
        
        // Mixed loop types within
        for (n = m; n < m + 10 && n < SIZE; n++) {
            if (C[n] % 2 == 0) {
                // Jump to L2
                j = n;
                goto L2_start;
            }
            
            // Complex pointer arithmetic
            int *ptr = &A[(m * n) % SIZE];
            int *qtr = &B[(m + n) % SIZE];
            
            if (ptr == qtr) { // Potential aliasing
                *ptr = *ptr + 1;
            }
            
            modify_global(ptr, qtr);
            
            // Multiple exit points
            if (global_counter > 1000) break;
            if (n > SIZE/2) goto partial_exit;
        }
        
    partial_exit:
        m += 5;
    } while (m < SIZE);
    
    // Compute checksum to prevent elimination
    unsigned long long checksum = 0;
    for (int idx = 0; idx < SIZE; idx++) {
        checksum += A[idx] + B[idx] * 2 + C[idx] * 3;
        // Prevent optimization with volatile
        side_effect(checksum & 0xFF);
    }
    
    printf("Checksum: %llu\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}

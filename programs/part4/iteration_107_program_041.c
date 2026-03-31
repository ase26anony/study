#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

volatile int global_counter = 0;
int A[SIZE];
int B[SIZE];

__attribute__((noinline, noclone))
void modify_global(int *ptr1, int *ptr2) {
    global_counter++;
    *ptr1 += *ptr2;
}

__attribute__((noinline, noclone))
int complex_condition(int x, int y) {
    volatile int v = x * y;
    return (v % 7) == 0;
}

__attribute__((noinline, noclone))
void side_effect(int val) {
    static int counter = 0;
    counter += val;
    global_counter = counter;
}

int main() {
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = SIZE - i;
    }
    
    int *ptr1 = A;
    int *ptr2 = B;
    int *ptr3 = A + SIZE/2;  // Create potential aliasing
    
    int sum = 0;
    int i = 0, j = 0;
    
    // Start of Loop L1 (for-like structure implemented with goto)
L1_header:
    if (i >= SIZE) goto L1_exit;
    
    // Shared basic block S1 - reachable from both loops
S1:
    modify_global(&A[i], &B[j]);
    side_effect(i + j);
    
    // Complex conditional with goto into L2
    if (complex_condition(i, j)) {
        goto L2_middle;  // Jump into middle of L2
    }
    
    // L1 body continues
    ptr1[i] = ptr2[j] * 2;
    
    // Another conditional that might exit to L2
    if ((i ^ j) & 1) {
        goto L2_header;
    }
    
    i++;
    goto L1_header;
    
L1_exit:
    goto after_loops;

    // Loop L2 (while-like structure)
L2_header:
    if (j >= SIZE) goto L2_exit;
    
L2_middle:  // Entry point from L1
    // Shared basic block S2
    int temp = ptr3[i % (SIZE/2)];  // Potential aliasing with A
    B[j] = temp * 3 + global_counter;
    
    // Switch statement creating multiple exit points
    switch ((i + j) % 4) {
        case 0:
            goto L1_header;  // Jump back to L1
        case 1:
            goto S1;         // Jump to shared block
        case 2:
            if (j > SIZE/2) goto L1_exit;
            break;
        default:
            break;
    }
    
    // Do-while style check
    do {
        if (complex_condition(j, i)) {
            goto L1_header;
        }
        j++;
    } while (0);  // Single iteration do-while
    
    // Conditional continue/break
    if (j % 3 == 0) {
        goto L2_header;
    } else if (j % 7 == 0) {
        goto L1_exit;
    }
    
    // Another shared block S3
S3:
    sum += A[i % SIZE] + B[j % SIZE];
    
    // Pointer arithmetic with stride
    int *p = &A[(i * 7 + j * 3) % SIZE];
    int *q = &B[(j * 5 + i * 2) % SIZE];
    
    // Potential aliasing
    if (p == q) {
        *p = global_counter;
        goto S1;
    }
    
    j++;
    goto L2_header;
    
L2_exit:
    // Try to re-enter L1 if conditions met
    if (i < SIZE/2 && global_counter < 1000) {
        goto S3;
    }

after_loops:
    // Third irregular loop that overlaps with both
    int k = 0;
    while (k < SIZE) {
        // Jump to shared blocks
        if (k % 5 == 0) goto S1;
        if (k % 9 == 0) goto S3;
        
        // Modify both arrays
        A[k] += B[SIZE - k - 1];
        B[k] -= A[SIZE - k - 1];
        
        // Conditional jump into L2's region
        if (complex_condition(k, global_counter)) {
            goto L2_middle;
        }
        
        k += (global_counter % 3) + 1;
    }
    
    // Compute checksum
    int checksum = 0;
    for (int idx = 0; idx < SIZE; idx++) {
        checksum ^= A[idx];
        checksum += B[idx];
    }
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return checksum != 0 ? 0 : 1;
}

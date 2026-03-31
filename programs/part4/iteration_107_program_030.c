#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Global variables to prevent optimizations */
volatile int global_counter = 0;
int checksum = 0;

/* Noinline functions with side effects */
__attribute__((noinline, noclone)) 
void side_effect_func(int* ptr) {
    *ptr += global_counter++;
}

__attribute__((noinline, noclone))
int maybe_swap(int* a, int* b, int cond) {
    if (cond) {
        int temp = *a;
        *a = *b;
        *b = temp;
        return 1;
    }
    return 0;
}

/* Complex loop structure with partial overlap */
void complex_loops(int* A, int* B) {
    int i = 0, j = 0;
    int* ptr1 = A;
    int* ptr2 = B;
    
    /* Loop L1 - for loop with irregular control flow */
    for (i = 0; i < SIZE; i++) {
        /* Basic block shared with L2 */
        side_effect_func(&A[i]);
        
        /* Conditional jump into middle of L2 */
        if (A[i] % 7 == 0) {
            goto enter_l2_mid;
        }
        
        /* Another shared basic block */
        checksum += A[i] * 3;
        
        /* Nested while loop that can exit to L1 */
        while (j < SIZE) {
            /* Shared computation */
            int cond = (i + j) % 11;
            
            /* Pointer aliasing - ptr1 and ptr2 may alias */
            if (maybe_swap(ptr1 + i, ptr2 + j, cond)) {
                /* Jump back to L1 header under certain conditions */
                if (cond % 3 == 0) {
                    j++;
                    continue;  /* Continue L2 */
                } else {
                    /* Exit to L1 */
                    goto l1_continue;
                }
            }
            
            /* Complex memory access with stride */
            B[j] += A[(i * 17 + j * 13) % SIZE];
            
            /* Multiple exit points */
            switch (B[j] % 5) {
                case 0:
                    j++;
                    break;
                case 1:
                    goto l2_exit;
                case 2:
                    goto l1_continue;
                case 3:
                    j += 2;
                    continue;  /* Continue L2 */
                default:
                    goto enter_l2_mid;  /* Re-enter L2 mid */
            }
            
        l2_exit:
            j = 0;
            break;
        }
        
    enter_l2_mid:
        /* Middle of L2 - shared with L1 */
        do {
            /* Do-while loop inside the overlap region */
            int k = i % 64;
            do {
                /* Loop-invariant with side effect */
                global_counter += (k % 2);
                k--;
                
                /* Conditional that can jump to L1 */
                if (k == i % 8) {
                    goto l1_continue;
                }
            } while (k > 0);
            
            /* Another shared block */
            checksum += B[j % SIZE];
            j++;
            
            /* Exit back to L1 under condition */
            if (j >= SIZE || A[i] % 13 == 0) {
                goto l1_continue;
            }
        } while (j < i + 10);
        
    l1_continue:
        /* Continue L1 */
        if (i % 19 == 0) {
            /* Additional basic block reachable from both loops */
            ptr1 = (ptr1 == A) ? B : A;
            ptr2 = (ptr2 == B) ? A : B;
        }
    }
}

/* Another function with switch-based loop interconnection */
void interconnected_loops(int* A, int* B) {
    int state = 0;
    int x = 0, y = 0;
    
    /* Loop L3 - while with goto into L4 */
    while (x < SIZE) {
        A[x] = x * 3;
        
        /* Shared block */
        side_effect_func(&A[x]);
        
        /* Switch that jumps between loops */
        switch (state) {
            case 0:
                if (A[x] % 17 == 0) {
                    state = 1;
                    goto l4_start;  /* Jump to L4 */
                }
                x++;
                break;
            case 1:
                goto l4_body;  /* Jump to L4 middle */
            case 2:
                x += 2;
                continue;  /* Continue L3 */
            default:
                goto l3_end;
        }
        
        /* This block is in L3 but not in L4 */
        checksum += x * 7;
        
    l4_start:
        /* Start of L4 - shared with L3 */
        y = 0;
        
    l4_body:
        /* Body of L4 - do-while with exit to L3 */
        do {
            /* Block shared between L3 and L4 */
            B[y] += A[x % SIZE];
            
            /* Conditional that can exit to L3 */
            if (B[y] % 23 == 0) {
                state = 2;
                y++;
                goto l3_continue;  /* Back to L3 */
            }
            
            y++;
            
            /* Another shared block */
            checksum += y * 11;
            
        } while (y < SIZE && y < x + 50);
        
        /* This block is in L4 but not always in L3 */
        if (y >= SIZE) {
            state = 0;
            x++;
        }
        
    l3_continue:
        /* Continue L3 */
        if (x % 29 == 0) {
            /* Another shared block */
            maybe_swap(&A[x], &B[y % SIZE], 1);
        }
    }
    
l3_end:
    return;
}

int main() {
    int A[SIZE];
    int B[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        A[i] = i;
        B[i] = SIZE - i;
    }
    
    /* Execute complex loops */
    complex_loops(A, B);
    interconnected_loops(A, B);
    
    /* Compute final checksum to prevent elimination */
    int final_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_checksum += A[i] + B[i];
    }
    final_checksum += checksum + global_counter;
    
    printf("Result: %d\n", final_checksum);
    return 0;
}

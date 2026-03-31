#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int* arr, int n) {
    int sum = 0;
    int stride = 3;  /* Loop-invariant variable */
    
    /* Outer loop - will contain inner loops */
    for (int i = 1; i < n - 1; i++) {
        int accumulator = 0;
        
        /* Middle loop - fully contained in outer */
        for (int j = 1; j < i; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 1; k < j; k++) {
                /* Complex access pattern with loop-invariant stride */
                sum += arr[i * stride + k - j];
                accumulator += arr[k];
                
                /* Multiple exit point */
                if (accumulator > LIMIT) {
                    break;  /* Exits innermost loop only */
                }
            }
            
            /* Another exit point for middle loop */
            if (sum > LIMIT * 10) {
                break;
            }
        }
        
        /* Additional computation in outer loop */
        sum += arr[i] * 2;
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto */
static int overlapping_loops(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    for (i = 0; i < n / 2; i++) {
        result += arr[i] * 3;
        
        /* Conditional jump to shared block */
        if (arr[i] % 7 == 0) {
            goto shared_block;
        }
        
        /* Continue with loop body */
        result -= arr[i] / 2;
        continue;
        
    shared_block:
        /* Shared basic block between two loops */
        result += 100;
        arr[i] = result % 256;
    }
    
    /* Second loop that shares the shared_block */
    for (int j = n / 2; j < n; j++) {
        result += arr[j] * 2;
        
        /* Also jumps to same shared block */
        if (arr[j] % 5 == 0) {
            goto shared_block;
        }
        
        result -= arr[j] / 3;
    }
    
    return result;
}

/* Function C: Loop with switch and break to label */
static int switch_in_loop(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    /* Loop with switch that breaks to outer label */
    for (int i = 0; i < n; i++) {
        state = arr[i] % 4;
        
        switch (state) {
            case 0:
                total += arr[i];
                break;  /* Normal switch break */
                
            case 1:
                total += arr[i] * 2;
                /* This break goes to switch, not loop */
                break;
                
            case 2:
                total += arr[i] * 3;
                /* Complex control flow: conditional break from loop */
                if (total > LIMIT) {
                    goto loop_exit;  /* Exits the entire loop */
                }
                break;
                
            case 3:
                total -= arr[i];
                /* Another loop exit point */
                if (i > n / 2) {
                    break;  /* This only exits switch, not loop */
                }
                break;
        }
        
        /* Loop-invariant computation */
        int invariant = n / 4;
        total += invariant;
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int adjacent_loops(int* arr, int n) {
    int sum1 = 0, sum2 = 0;
    
    /* First loop */
    int i;
    for (i = 0; i < n; i++) {
        sum1 += arr[i];
        
        /* Early exit creates multiple basic blocks */
        if (sum1 > LIMIT) {
            break;
        }
        
        /* Common computation block */
        if (i % 2 == 0) {
            arr[i] = arr[i] * 2;
        }
    }
    
    /* Second loop - partially overlaps in control flow */
    for (int j = 0; j < i; j++) {  /* Note: uses i from first loop */
        sum2 += arr[j];
        
        /* Same common computation pattern */
        if (j % 2 == 0) {
            arr[j] = arr[j] * 3;
        }
        
        /* Different exit condition */
        if (sum2 > LIMIT / 2) {
            break;
        }
    }
    
    return sum1 + sum2;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += triple_nested_loop(arr, SIZE / 4);
    
    /* Function B: Overlapping loops with goto */
    result += overlapping_loops(arr + SIZE / 4, SIZE / 4);
    
    /* Function C: Loop with switch and complex breaks */
    result += switch_in_loop(arr + SIZE / 2, SIZE / 4);
    
    /* Function D: Adjacent loops with partial overlap */
    result += adjacent_loops(arr + 3 * SIZE / 4, SIZE / 4);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

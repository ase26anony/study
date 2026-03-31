#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            sum += arr[i * n + j];
            
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < j; k++) {
                sum -= arr[k];
                if (sum > LIMIT) break;  /* Multiple exit point */
            }
            
            if (sum < -LIMIT) break;  /* Another exit point */
        }
        
        /* Second inner loop - also fully contained in outer */
        int stride = 2;  /* Loop-invariant variable */
        for (int j = 0; j < n; j += stride) {
            sum += arr[i * stride + j % n];
            if (sum > LIMIT * 2) break;
        }
    }
    
    return sum;
}

/* Function B: Overlapping loops with shared basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will share a basic block with second loop */
loop1_start:
    for (; i < n / 2; i++) {
        result += arr[i];
        
        /* Conditional that might jump to shared block */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        if (result > LIMIT) {
            break;
        }
    }
    
    /* Jump to avoid executing shared block from here */
    goto after_shared;
    
shared_block:
    /* Shared basic block between two loops */
    result ^= 0x55;
    i++;
    
after_shared:
    
    /* Second loop - partially overlaps with first loop's blocks */
    for (; j < n; j++) {
        result -= arr[j];
        
        /* Same shared block as first loop */
        if (result % 11 == 0) {
            goto shared_block;
        }
        
        if (result < -LIMIT) {
            break;
        }
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                if (total > 100) {
                    state = 1;
                }
                break;
            case 1:
                if (total > 200) {
                    state = 2;
                }
                /* This break exits the switch, not the loop */
                break;
            case 2:
                /* This label is used for switch case */
                if (total > 300) {
                    /* Break to label outside the loop */
                    goto loop_exit;
                }
                break;
            default:
                total *= 2;
                break;
        }
        
        /* Multiple exit points */
        if (total > 500) {
            break;
        }
        
        /* Loop-invariant computation */
        int stride = 3;
        total += arr[(i * stride) % n];
    }
    
    return total;
    
loop_exit:
    /* Label outside the loop */
    return total * 2;
}

/* Function D: Adjacent loops with potential overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n; i += 2) {
        acc1 += arr[i];
        if (acc1 > LIMIT / 2) {
            /* Jump to code that could be considered overlapping */
            goto middle_computation;
        }
    }
    
    goto after_middle;
    
middle_computation:
    /* This block could be reached from both loops */
    acc1 = (acc1 * 3) / 2;
    
after_middle:
    
    /* Second adjacent loop - not nested, but may share middle_computation */
    for (int i = 1; i < n; i += 2) {
        acc2 -= arr[i];
        if (acc2 < -LIMIT / 2) {
            goto middle_computation;
        }
    }
    
    return acc1 + acc2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values moderate */
    }
    
    /* Call functions with different slices of the array */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, 16);  /* Smaller size for nested loops */
    
    /* Function B: Overlapping loops with goto */
    result += function_b(arr + 256, 128);
    
    /* Function C: Loop with complex switch */
    result += function_c(arr + 512, 64);
    
    /* Function D: Adjacent loops with shared block */
    result += function_d(arr + 768, 96);
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n / 4; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < 8; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < 4; k++) {
                sum += arr[i * 16 + j * 4 + k];
                
                /* Multiple exit points */
                if (sum > LIMIT) {
                    sum = LIMIT;
                    break;  /* Breaks innermost loop */
                }
            }
            
            /* Loop-invariant code */
            int stride = 2;  /* Loop invariant */
            if (j % stride == 0) {
                sum += arr[i * 16 + j];
            }
            
            if (sum > LIMIT * 2) {
                break;  /* Breaks middle loop */
            }
        }
        
        /* Another conditional break in outer loop */
        if (i > n / 8 && sum > LIMIT / 2) {
            break;
        }
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will share blocks with second loop via goto */
    for (i = 0; i < n / 2; i++) {
        result += arr[i] * 2;
        
        /* Conditional goto to shared block */
        if (arr[i] % 3 == 0) {
            goto shared_computation;
        }
        
        if (result > LIMIT) {
            result = result % LIMIT;
        }
        
        continue;
        
    shared_computation:
        /* Shared basic block between two loops */
        result += arr[i] * 3;
        if (result > LIMIT * 2) {
            result = LIMIT;
        }
    }
    
    /* Second loop - partially overlaps with first via shared block */
    for (j = n / 2; j < n; j++) {
        result -= arr[j];
        
        /* Same goto target as first loop */
        if (arr[j] % 5 == 0) {
            goto shared_computation2;
        }
        
        if (result < 0) {
            result = 0;
        }
        
        continue;
        
    shared_computation2:
        /* Another shared block - different from first but creates complexity */
        result += arr[j] * 5;
        /* Jump back to the shared block from first loop */
        if (arr[j] % 7 == 0) {
            goto shared_computation;
        }
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int loop_with_switch(int *arr, int n) {
    int total = 0;
    int i = 0;
    
    while (i < n) {
        total += arr[i];
        
        /* Switch with break that can exit the loop */
        switch (arr[i] % 4) {
            case 0:
                total += 1;
                break;  /* This breaks the switch, not the loop */
            case 1:
                total += 2;
                /* Complex control flow */
                if (total > LIMIT) {
                    goto exit_loop;  /* Exits loop via goto */
                }
                break;
            case 2:
                total += 3;
                /* This break statement is ambiguous - could be analyzed as loop exit */
                if (total > LIMIT / 2) {
                    i++;
                    break;  /* Actually breaks the switch, but might confuse analysis */
                }
                break;
            case 3:
                total += 4;
                /* Direct break from loop inside switch */
                if (total > LIMIT * 3 / 4) {
                    i = n;  /* Force loop exit */
                    break;
                }
                break;
        }
        
        /* Multiple exit points */
        if (total < 0) {
            break;  /* Breaks the while loop */
        }
        
        i++;
        continue;
        
    exit_loop:
        /* Label target for goto from switch */
        break;  /* Exits the while loop */
    }
    
    return total;
}

/* Function D: Adjacent loops with irreducible control flow */
static int irreducible_loops(int *arr, int n) {
    int acc = 0;
    int counter = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n / 3; i++) {
        acc += arr[i];
        
        /* Jump to code shared with next loop */
        if (acc % 2 == 0) {
            goto middle_block;
        }
        
        counter++;
    }
    
    /* Second adjacent loop - shares blocks with first */
    for (int j = n / 3; j < 2 * n / 3; j++) {
        acc -= arr[j];
        
    middle_block:
        /* Shared block between the two loops */
        acc = acc * 2 % 100;
        
        if (acc % 3 == 0) {
            goto end_block;
        }
        
        counter--;
    }
    
    /* Third loop that partially overlaps with shared blocks */
    for (int k = 2 * n / 3; k < n; k++) {
        acc += arr[k] * 3;
        
    end_block:
        /* Another shared block */
        if (acc > 1000) {
            acc = 1000;
        }
        
        /* Jump back to middle_block creating cycle */
        if (k % 7 == 0 && counter < 0) {
            goto middle_block;
        }
    }
    
    return acc + counter;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call functions with different loop structures */
    int result1 = triple_nested_loop(arr, SIZE);
    int result2 = overlapping_loops(arr + SIZE/4, SIZE/2);
    int result3 = loop_with_switch(arr + SIZE/2, SIZE/4);
    int result4 = irreducible_loops(arr, SIZE);
    
    /* Combine results to prevent elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;  /* Return non-zero to ensure execution */
}

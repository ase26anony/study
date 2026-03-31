#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested loops with fully contained sub-loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer loop */
        for (int j = 0; j < i; j++) {
            sum += arr[i * n + j];
            
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < j; k++) {
                sum -= arr[k];
                if (sum > LIMIT) break;  /* Multiple exit point */
            }
            
            /* Loop-invariant code to encourage strength reduction */
            int stride = 2;  /* Loop invariant */
            sum += arr[j * stride];
        }
        
        /* Another inner loop at same nesting level */
        int count = 0;
        while (count < 5) {
            sum += arr[i + count];
            count++;
            if (sum < -LIMIT) break;  /* Another exit point */
        }
    }
    
    return sum;
}

/* Function B: Two loops that share common basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with goto to shared block */
loop1:
    for (; i < n/2; i++) {
        result += arr[i];
        
        /* Conditional goto to shared block */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        /* Continue with loop1 */
        result -= arr[i] / 2;
        
        if (i > n/4 && result > 500) {
            break;  /* Early exit */
        }
    }
    
    /* Reset for second loop */
    i = n/2;
    
    /* Second loop that also jumps to shared block */
    while (i < n) {
        result *= arr[i] + 1;
        
        /* Different condition to goto shared block */
        if (result % 11 == 0) {
            goto shared_block;
        }
        
        i++;
        
        /* Multiple exit conditions */
        if (result > 10000 || result < -10000) {
            break;
        }
    }
    
    return result;

/* Shared basic block - creates overlapping control flow */
shared_block:
    result = (result * 3) / 2;
    
    /* Jump back to appropriate loop */
    if (i < n/2) {
        i++;
        goto loop1;
    } else {
        i++;
        /* Continue in while loop */
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int idx = 0; idx < n; idx++) {
        total += arr[idx];
        
        /* Switch inside loop with break to exit loop */
        switch (state) {
            case 0:
                total += 10;
                if (total > 2000) {
                    state = 1;
                }
                break;
            case 1:
                total -= 20;
                if (total < 1000) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* Fall through */
            case 2:
                /* This label and goto create additional basic blocks */
                if (total % 13 == 0) {
                    goto special_case;
                }
                total /= 2;
                break;
            default:
                /* Break statement that exits the entire loop */
                if (total > 3000) {
                    goto loop_exit;  /* Exit loop via goto */
                }
                break;
        }
        
        /* Continue normal loop processing */
        if (idx % 3 == 0) {
            total += arr[idx] * 2;
        }
        
        continue;
        
    special_case:
        total += 100;
        /* Continue loop */
    }
    
loop_exit:
    
    /* Additional loop that partially overlaps with previous one */
    int j = 0;
    while (j < n && j < 10) {
        total -= arr[j];
        
        /* Shared computation with for loop above */
        if (total % 5 == 0) {
            total += j * 3;
        }
        
        j++;
    }
    
    return total;
}

/* Function D: Complex nested loops with irreducible control flow */
static int function_d(int *arr, int n) {
    int acc = 0;
    
    /* Outer loop */
    for (int x = 0; x < n; x += 3) {
        acc += arr[x];
        
        /* Inner loop that's fully contained */
        int y = 0;
        do {
            acc += arr[y];
            y++;
            
            /* Deeply nested loop */
            for (int z = 0; z < y; z++) {
                acc -= arr[z];
                
                /* Multiple exit points */
                if (acc > 5000) {
                    goto partial_exit;
                }
                
                if (z % 4 == 0) {
                    acc += x * z;
                }
            }
            
        } while (y < 5);
        
        /* Another partially overlapping loop */
        if (x % 2 == 0) {
            int w = x;
            while (w < n && w < x + 10) {
                acc += arr[w] * 2;
                w++;
                
                /* This creates control flow that overlaps with do-while */
                if (acc % 17 == 0) {
                    goto partial_exit;
                }
            }
        }
        
        continue;
        
    partial_exit:
        /* Shared exit path for multiple loops */
        acc /= 2;
        if (x < n - 1) {
            continue;  /* Continue outer loop */
        }
    }
    
    return acc;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call functions with different loop patterns */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, 16);
    
    /* Function B: Loops with shared basic blocks via goto */
    result += function_b(arr + 100, 200);
    
    /* Function C: Loop with switch and break */
    result += function_c(arr + 300, 150);
    
    /* Function D: Complex nested loops with irreducible flow */
    result += function_d(arr + 500, 100);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

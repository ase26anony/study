#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 100

/* Function A: Triple-nested loops with fully contained sub-loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer loop */
        for (int j = 0; j < i; j++) {
            sum += arr[j];
            
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < j; k++) {
                sum -= arr[k];
                if (sum > LIMIT * 10) {
                    break;  /* Multiple exit point */
                }
            }
            
            /* Loop-invariant code */
            int stride = 2;  /* Loop invariant */
            if (i + stride < n) {
                sum += arr[i * stride % n];
            }
        }
        
        /* Another inner loop at same nesting level */
        int count = 0;
        while (count < 5) {
            sum += arr[i] * count;
            count++;
            if (sum < -LIMIT) {
                break;  /* Another exit point */
            }
        }
    }
    
    return sum;
}

/* Function B: Two loops that share a common basic block via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with goto to shared block */
loop1:
    for (; i < n/2; i++) {
        result += arr[i];
        
        /* Conditional break */
        if (result > LIMIT) {
            goto shared_block;  /* Jump to shared basic block */
        }
        
        /* Continue with loop */
        result -= arr[i] / 2;
    }
    
    /* Reset for second loop */
    i = n/2;
    
    /* Second loop that also jumps to the shared block */
    while (i < n) {
        result *= arr[i] + 1;
        
        if (result < -LIMIT) {
            goto shared_block;  /* Same shared block as loop1 */
        }
        
        /* Different computation to create different basic blocks */
        for (int j = 0; j < 3; j++) {
            result += j;
            if (j == 2 && result % 7 == 0) {
                goto shared_block;  /* Another path to shared block */
            }
        }
        
        i++;
    }
    
    return result;

shared_block:
    /* Shared basic block between the two loops */
    result = (result * 3) / 2;
    
    /* Could return to either loop or exit */
    if (i < n) {
        i++;
        if (i < n/2) {
            goto loop1;  /* Return to first loop */
        } else {
            /* Continue with second loop logic */
            while (i < n) {
                result += arr[i++];
            }
        }
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int idx = 0; idx < n; idx++) {
        total += arr[idx];
        
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                if (total > 50) {
                    state = 1;
                    total += 10;
                }
                break;  /* This break is for switch, not loop */
                
            case 1:
                if (total > 100) {
                    state = 2;
                    /* This will exit the entire loop */
                    goto loop_exit;
                }
                total -= 5;
                break;
                
            case 2:
                /* Nested loop inside switch case */
                for (int k = 0; k < 3; k++) {
                    total += k * arr[idx];
                    if (total > 200) {
                        break;  /* Exits inner for loop only */
                    }
                }
                
                /* Switch break that actually exits the outer loop */
                if (total > 150) {
                    idx = n;  /* Force loop exit */
                    break;
                }
                break;
                
            default:
                /* Direct break from switch to exit loop */
                if (total < 0) {
                    idx = n;
                    break;
                }
        }
        
        /* Multiple exit points */
        if (total > 300) {
            break;  /* Exits the outer for loop */
        }
        
        /* Loop-invariant computation */
        int invariant = n / 4;
        total += arr[invariant % n];
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int val = 0;
    int i = 0;
    
    /* First adjacent loop */
adjacent_loop1:
    while (i < n/3) {
        val += arr[i] * 2;
        i++;
        
        /* Jump to overlapping section */
        if (val % 11 == 0) {
            goto overlapping_section;
        }
    }
    
    /* Second adjacent loop */
    while (i < 2*n/3) {
        val -= arr[i];
        i++;
        
        if (val % 13 == 0) {
            goto overlapping_section;
        }
        
        /* Small inner loop */
        for (int j = 0; j < 2; j++) {
            val += j;
        }
    }
    
    return val;

overlapping_section:
    /* This block is reachable from both adjacent loops */
    val = (val * 3) % 97;
    
    /* Can return to either loop based on conditions */
    if (val > 0 && i < n/3) {
        goto adjacent_loop1;
    } else if (i < 2*n/3) {
        /* Continue with second loop logic */
        val += arr[i++] * 3;
    }
    
    /* Finish any remaining iterations */
    while (i < n) {
        val += arr[i++];
    }
    
    return val;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully contained nested loops */
    result += function_a(arr, SIZE / 4);
    
    /* Function B: Loops sharing basic blocks via goto */
    result += function_b(arr + SIZE / 4, SIZE / 4);
    
    /* Function C: Loop with complex switch and break */
    result += function_c(arr + SIZE / 2, SIZE / 4);
    
    /* Function D: Adjacent loops with partial overlap */
    result += function_d(arr + 3 * SIZE / 4, SIZE / 4);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

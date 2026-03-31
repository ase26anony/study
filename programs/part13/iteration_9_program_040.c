#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested loops with fully contained sub-loops */
static int func_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += 4) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < 8; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < 4; k++) {
                sum += arr[i + j + k];
                
                /* Multiple exit points */
                if (sum > LIMIT) {
                    break;  /* Exits innermost loop */
                }
            }
            
            /* Loop-invariant code */
            int stride = 2;  /* Loop invariant */
            if (j % stride == 0) {
                sum += arr[i * stride];
            }
            
            if (sum > LIMIT * 2) {
                break;  /* Exits middle loop */
            }
        }
        
        /* Another conditional exit */
        if (i > n / 2 && sum > LIMIT / 2) {
            break;
        }
    }
    
    return sum;
}

/* Function B: Two loops that share a common basic block via goto */
static int func_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop - partially overlaps with second */
loop1_start:
    while (i < n / 2) {
        result += arr[i];
        i++;
        
        /* Shared basic block via goto */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        if (i > n / 4) {
            break;
        }
    }
    
    i = n / 2;
    
    /* Second loop - shares block with first */
    while (i < n) {
        result -= arr[i];
        i++;
        
        /* Same shared block */
        if (result % 5 == 0) {
            goto shared_block;
        }
        
        if (i > 3 * n / 4) {
            break;
        }
    }
    
    goto done;

shared_block:
    /* Common basic block for both loops */
    result *= 2;
    if (i < n) {
        /* Jump back to appropriate loop */
        if (i < n / 2) {
            goto loop1_start;
        }
    }

done:
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int func_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break that can exit the loop */
        switch (state) {
            case 0:
                if (total > LIMIT) {
                    state = 1;
                }
                total += i;
                break;
            case 1:
                if (total % 11 == 0) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* Fall through */
            case 2:
                /* Complex exit condition */
                if (total > LIMIT * 3) {
                    goto loop_exit;  /* Explicit loop exit */
                }
                total -= arr[i] / 2;
                break;
            default:
                /* Direct break from switch to outside loop */
                if (i > n / 3) {
                    i = n;  /* Force loop exit */
                    continue;
                }
        }
        
        /* Multiple basic blocks within loop */
        if (i % 3 == 0) {
            total += 1;
        } else if (i % 3 == 1) {
            total += 2;
        } else {
            total += 3;
        }
        
        /* Another exit point */
        if (total < 0) {
            break;
        }
        
        continue;
        
    loop_exit:
        /* Label for goto exit */
        break;
    }
    
    return total;
}

/* Function D: Nested loops with irreducible control flow */
static int func_d(int *arr, int n) {
    int acc = 0;
    int i = 0, j = 0;
    
outer_loop:
    for (; i < n; i++) {
        acc += arr[i];
        
        /* Inner loop with multiple exits */
        j = 0;
        while (j < 10) {
            acc += arr[i + j];
            j++;
            
            /* Conditional that might jump outside inner but inside outer */
            if (acc % 13 == 0) {
                goto middle_block;
            }
            
            if (j > 5 && i > n / 2) {
                goto outer_loop_end;
            }
        }
        
        continue;
        
    middle_block:
        /* Block that's inside outer loop but not always in inner loop */
        acc -= 5;
        if (j < 10) {
            continue;  /* Goes back to while test */
        }
    }
    
outer_loop_end:
    
    /* Another separate loop that shares no blocks with outer_loop */
    for (int k = 0; k < n / 4; k++) {
        acc += arr[k] * 2;
    }
    
    return acc;
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
    result += func_a(arr, SIZE / 2);
    
    /* Function B: Overlapping loops with shared block */
    result += func_b(arr + SIZE / 2, SIZE / 2);
    
    /* Function C: Loop with switch and complex exits */
    result += func_c(arr, SIZE / 3);
    
    /* Function D: Irreducible control flow */
    result += func_d(arr + SIZE / 3, SIZE / 3);
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result);
    
    return result % 256;  /* Ensure value is used */
}

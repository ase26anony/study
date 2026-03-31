#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 8
#define MIDDLE_SIZE 16

static volatile int sink;

/* Function A: Triple-nested loops with fully contained sub-loops */
static int function_a(int* arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += MIDDLE_SIZE) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < MIDDLE_SIZE && (i + j) < n; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[i + j] * k;
            }
            
            /* Multiple exit point */
            if (sum > 1000000) {
                break;
            }
        }
        
        /* Loop-invariant code */
        int stride = 2;
        for (int j = 0; j < MIDDLE_SIZE && (i + j) < n; j++) {
            sum += arr[i + j * stride];
        }
    }
    
    return sum;
}

/* Function B: Two loops that share common basic blocks via goto */
static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with goto to shared block */
loop1:
    while (i < n / 2) {
        result += arr[i] * 3;
        i++;
        
        /* Conditional goto to shared code */
        if (i % 7 == 0) {
            goto shared_block;
        }
        
        if (result > 50000) {
            break;
        }
    }
    
    i = n / 2;
    
    /* Second loop that also uses the shared block */
loop2:
    while (i < n) {
        result -= arr[i] * 2;
        i++;
        
        /* Same shared block as loop1 */
        if (i % 5 == 0) {
            goto shared_block;
        }
        
        if (result < -50000) {
            break;
        }
    }
    
    goto done;

/* Shared basic block between the two loops */
shared_block:
    result *= 2;
    if (i < n) {
        if (i < n / 2) {
            goto loop1;
        } else {
            goto loop2;
        }
    }

done:
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break that can exit the loop */
        switch (state) {
            case 0:
                if (total > 10000) {
                    state = 1;
                }
                break;
            case 1:
                if (arr[i] % 13 == 0) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* This break exits the entire loop */
                if (total > 20000) {
                    goto loop_exit;
                }
                break;
            case 2:
                /* Complex control flow */
                for (int j = 0; j < 5; j++) {
                    total += j;
                    if (total > 30000) {
                        /* Break from inner loop only */
                        break;
                    }
                }
                break;
        }
        
        /* Another exit point */
        if (i > n / 3 && total < 0) {
            break;
        }
    }
    
loop_exit:
    
    /* Another loop that partially overlaps with previous through shared code */
    int k = 0;
    while (k < n / 2) {
        total -= arr[k];
        k++;
        
        /* Shared computation that could be in its own basic block */
        int temp = total * 2;
        if (temp > 1000) {
            total = temp / 3;
        }
    }
    
    return total;
}

/* Function D: Complex nested loops with partial overlap */
static int function_d(int* arr, int n) {
    int acc = 0;
    
    /* First loop structure */
    for (int i = 0; i < n; i += 4) {
        /* Inner loop that's fully contained */
        for (int j = 0; j < 4 && (i + j) < n; j++) {
            acc += arr[i + j];
        }
        
        /* Jump to shared code block */
        if (acc % 100 == 0) {
            goto shared_computation;
        }
    }
    
    /* Second loop structure that shares some blocks */
    for (int i = 1; i < n; i *= 2) {
        acc -= arr[i];
        
shared_computation:
        /* Shared basic block */
        acc = (acc * 13) % 1000;
        
        if (i < n / 2) {
            continue;
        } else {
            /* Different path */
            for (int k = 0; k < 3; k++) {
                acc += k * arr[i];
            }
        }
    }
    
    return acc;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
    }
    
    /* Call functions with different slices to create various loop structures */
    int result = 0;
    
    /* Function A: Fully contained nested loops */
    result += function_a(arr, SIZE);
    
    /* Function B: Loops sharing basic blocks via goto */
    result += function_b(arr + 100, SIZE - 100);
    
    /* Function C: Loop with switch and break */
    result += function_c(arr + 200, SIZE - 200);
    
    /* Function D: Partially overlapping loops */
    result += function_d(arr + 300, SIZE - 300);
    
    /* Additional complex loop in main to ensure analysis */
    int final_check = 0;
    for (int i = 0; i < 100; i++) {
        /* Nested loop with multiple exits */
        for (int j = 0; j < 50; j++) {
            final_check += arr[i * 10 + j];
            if (final_check > 100000) {
                goto main_loop_exit;
            }
            
            /* Inner-inner loop */
            for (int k = 0; k < 5; k++) {
                final_check -= k;
                if (k == 3 && final_check < 0) {
                    break;
                }
            }
        }
        
        /* Another exit point */
        if (i == 75) {
            break;
        }
    }
    
main_loop_exit:
    result += final_check;
    
    /* Use volatile sink to prevent elimination */
    sink = result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

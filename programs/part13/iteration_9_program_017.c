#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 8
#define MIDDLE_SIZE 16

static volatile int sink = 0;

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
            
            /* Multiple exit points */
            if (sum > 1000000) {
                break;
            }
        }
        
        /* Loop-invariant code */
        int stride = 2;
        for (int j = 0; j < MIDDLE_SIZE && (i + j * stride) < n; j++) {
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
        result += arr[i] * 2;
        i++;
        
        /* Conditional goto to shared block */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        if (i % 5 == 0) {
            continue;
        }
        
        /* Another basic block in loop1 */
        result -= arr[i] / 3;
    }
    
    i = n / 2;
    
    /* Second loop that also jumps to shared block */
    while (i < n) {
        result += arr[i] * 3;
        i += 2;
        
        /* Different condition to goto shared block */
        if (result % 11 == 0) {
            goto shared_block;
        }
        
        if (i % 13 == 0) {
            break;
        }
        
        /* Another basic block in loop2 */
        result += i;
    }
    
    return result;

/* Shared basic block between the two loops */
shared_block:
    result = (result * 2) % 1000;
    
    /* Jump back to appropriate loop */
    if (i < n / 2) {
        goto loop1;
    } else {
        i++;
        if (i < n) {
            goto loop1;  /* This creates partial overlap */
        }
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int idx = 0; idx < n; idx++) {
        total += arr[idx];
        
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                if (total > 5000) {
                    state = 1;
                    /* This break is for the switch, not the loop */
                    break;
                }
                total += idx;
                break;
                
            case 1:
                if (arr[idx] % 2 == 0) {
                    state = 2;
                } else {
                    /* This break exits the entire loop */
                    goto loop_exit;
                }
                break;
                
            case 2:
                /* Nested loop inside case */
                for (int inner = 0; inner < 3; inner++) {
                    total += inner * arr[idx];
                    if (total > 10000) {
                        /* Break from nested loop only */
                        break;
                    }
                }
                
                /* Labeled break target */
                if (total < 0) {
                    goto negative_total;
                }
                break;
                
            default:
                /* Direct break from loop */
                if (idx > n / 2) {
                    return total;
                }
        }
        
        /* Multiple exit points */
        if (total > 20000) {
            break;
        }
        
        continue;
        
    negative_total:
        total = -total;
        if (idx < n - 1) {
            continue;
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Complex loop nest with partial overlap */
static int function_d(int* arr, int n) {
    int acc = 0;
    int i = 0;
    int j = 0;
    
    /* First loop structure */
start_loop1:
    for (; i < n / 3; i++) {
        acc += arr[i] * i;
        
        /* Inner loop that's fully contained */
        for (j = 0; j < 4; j++) {
            acc += j;
            if (acc % 17 == 0) {
                /* Jump to shared code */
                goto shared_computation;
            }
        }
        
        if (i % 7 == 0) {
            /* Jump to second loop structure */
            i = n / 3;
            goto start_loop2;
        }
    }
    
    i = n / 3;
    
start_loop2:
    /* Second loop that shares some blocks with first */
    while (i < 2 * n / 3) {
        acc -= arr[i];
        i += 2;
        
shared_computation:
        /* Shared basic block */
        acc = (acc * 3) % 100;
        
        if (i < n / 3) {
            /* Go back to first loop */
            goto start_loop1;
        }
        
        /* Different computation path */
        for (int k = 0; k < 2; k++) {
            acc += k * arr[i % n];
        }
    }
    
    return acc;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + 7) % 100;
    }
    
    /* Call all functions to create various loop structures */
    int result_a = function_a(arr, SIZE);
    int result_b = function_b(arr, SIZE);
    int result_c = function_c(arr, SIZE);
    int result_d = function_d(arr, SIZE);
    
    /* Combine results to prevent elimination */
    int final_result = result_a + result_b + result_c + result_d;
    
    /* Use volatile sink and printf to ensure code isn't eliminated */
    sink = final_result;
    printf("Result: %d\n", final_result % 1000);
    
    return final_result % 256;
}

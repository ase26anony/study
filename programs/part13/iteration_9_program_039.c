#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 32

/* Function A: Triple-nested loops with fully contained sub-loops */
static int function_a(int* arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += INNER_SIZE) {
        /* First inner loop - fully contained in outer loop */
        for (int j = i; j < i + INNER_SIZE && j < n; j++) {
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < 4; k++) {
                sum += arr[j] * (k + 1);
                /* Multiple exit point */
                if (sum > 1000000) {
                    break;
                }
            }
            /* Another conditional break */
            if (arr[j] < 0) {
                break;
            }
        }
        
        /* Second inner loop - also fully contained in outer loop */
        int limit = i + INNER_SIZE;
        if (limit > n) limit = n;
        for (int j = i; j < limit; j += 2) {
            sum -= arr[j];
            /* Loop-invariant code with strength reduction potential */
            int stride = 3;
            sum += arr[j * stride % n];
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
        result += arr[i];
        i++;
        
        /* Conditional goto to shared code */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        /* Continue with loop1 specific code */
        result ^= 0x55;
        continue;
        
shared_block:
        /* Shared basic block between two loops */
        result *= 2;
        if (result > 1000) {
            result /= 3;
        }
        /* Jump back to appropriate loop */
        goto loop1_continue;
    }
    
    i = n / 2;
    
    /* Second loop that also uses the shared block */
loop2:
    while (i < n) {
        result -= arr[i];
        i++;
        
        /* Different condition to goto shared block */
        if (result % 11 == 0) {
            goto shared_block;
        }
        
        /* Loop2 specific code */
        result ^= 0xAA;
        continue;
        
loop1_continue:
        /* Continuation point for loop1 */
        continue;
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                total += arr[i];
                if (total > 500) {
                    state = 1;
                }
                break;
            case 1:
                total -= arr[i];
                if (total < 0) {
                    state = 2;
                }
                /* This break exits the switch, not the loop */
                break;
            case 2:
                total *= arr[i] | 1; /* Ensure non-zero */
                /* This label and goto create additional control flow */
                if (total > 10000) {
                    goto exit_loop;
                }
                state = 0;
                break;
            default:
                /* Unreachable, but creates more basic blocks */
                total = 0;
                break;
        }
        
        /* Additional loop body code */
        if (i % 3 == 0) {
            total ^= i;
        }
        
        continue;
        
exit_loop:
        /* Break out of the loop from within switch */
        break;
    }
    
    return total;
}

/* Function D: Partially overlapping loops using conditional blocks */
static int function_d(int* arr, int n) {
    int acc = 0;
    int i = 0;
    
    /* First loop */
    while (i < n) {
        acc += arr[i];
        
        /* Conditional block that might be shared */
        if (acc % 2 == 0) {
            /* This block could be part of both loops' flow */
            acc += 100;
            i += 2;
        } else {
            i++;
        }
        
        if (i >= n / 2) {
            /* Transition point - partial overlap ends */
            break;
        }
    }
    
    /* Second loop that partially overlaps in control flow */
    int j = n / 3;
    while (j < n) {
        acc -= arr[j];
        
        /* Same conditional block structure as first loop */
        if (acc % 2 == 0) {
            acc += 100;
            j += 2;
        } else {
            j++;
        }
        
        /* Different termination condition */
        if (acc > 5000 || acc < -5000) {
            break;
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
    int result = 0;
    
    /* Force loop analysis by using different array slices */
    result += function_a(arr, SIZE);
    result += function_b(arr + SIZE/4, SIZE/2);
    result += function_c(arr + SIZE/2, SIZE/4);
    result += function_d(arr, SIZE);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

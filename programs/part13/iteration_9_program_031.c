#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i += 4) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < 8; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < 4; k++) {
                sum += arr[i + j + k];
                
                /* Multiple exit point */
                if (sum > LIMIT) {
                    break;
                }
            }
            
            /* Loop-invariant code */
            int stride = 2;
            if (j % stride == 0) {
                sum += arr[i * stride];
            }
            
            /* Another conditional break */
            if (sum < -LIMIT) {
                break;
            }
        }
        
        /* Early exit from outer loop */
        if (i > n / 2 && sum > LIMIT / 2) {
            break;
        }
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will share blocks with second loop via goto */
    for (i = 0; i < n; i += 2) {
        result += arr[i];
        
        /* Conditional goto to shared block */
        if (arr[i] % 3 == 0) {
            goto shared_block;
        }
        
        /* Normal loop continuation */
        result -= arr[i] / 2;
        continue;
        
    shared_block:
        /* Shared basic block between two loops */
        result *= 2;
        if (result > 10000) {
            result = 10000;
        }
        /* Continue back to first loop */
        continue;
    }
    
    /* Second loop - partially overlaps with first via shared_block */
    for (j = n - 1; j >= 0; j -= 3) {
        result += arr[j] * 3;
        
        /* Same goto target as first loop */
        if (arr[j] % 4 == 0) {
            goto shared_block;
        }
        
        /* Different computation path */
        result /= (arr[j] % 7) + 1;
    }
    
    return result;
}

/* Function C: Loop with switch and complex break */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    /* Loop with switch inside */
    for (int idx = 0; idx < n; idx++) {
        total += arr[idx];
        
        /* Switch with break to label outside loop */
        switch (state) {
            case 0:
                if (arr[idx] % 2 == 0) {
                    state = 1;
                }
                total += 1;
                break;
            case 1:
                if (total > 500) {
                    /* This break goes to switch, not loop */
                    break;
                }
                total += arr[idx] * 2;
                if (total > 1000) {
                    /* Exit loop entirely via goto */
                    goto loop_exit;
                }
                break;
            case 2:
                total -= arr[idx];
                /* Complex control flow */
                if (idx % 5 == 0) {
                    continue;  /* Skip to next iteration */
                }
                break;
            default:
                /* Direct break from loop via switch */
                goto loop_exit;
        }
        
        /* Loop-invariant computation */
        int invariant = n / 4;
        total += invariant;
        
        /* Another conditional exit */
        if (total < -500) {
            break;
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First adjacent loop */
    int start = 0;
    while (start < n) {
        acc1 += arr[start];
        start += 2;
        
        /* Shared exit condition */
        if (acc1 > 2000) {
            goto common_exit;
        }
    }
    
    /* Second adjacent loop - shares common_exit */
    int end = n - 1;
    while (end >= 0) {
        acc2 += arr[end];
        end -= 3;
        
        /* Same exit condition */
        if (acc2 > 2000) {
            goto common_exit;
        }
        
        /* Different computation to create different blocks */
        if (end % 4 == 0) {
            acc2 *= 2;
        }
    }
    
common_exit:
    return acc1 + acc2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + 7) % 100;
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, SIZE / 2);
    
    /* Function B: Overlapping loops with goto */
    result += function_b(arr + SIZE / 4, SIZE / 4);
    
    /* Function C: Loop with switch and complex break */
    result += function_c(arr + SIZE / 3, SIZE / 3);
    
    /* Function D: Adjacent loops with shared exit */
    result += function_d(arr, SIZE);
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result);
    
    /* Additional complex loop in main to increase coverage */
    volatile int sink = 0;
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with multiple exits */
        for (int inner = 0; inner < 20; inner++) {
            sink += arr[outer * 20 + inner];
            
            /* Nested switch inside inner loop */
            switch (inner % 3) {
                case 0:
                    sink += outer;
                    if (sink > 1000) {
                        /* Break from inner loop */
                        goto inner_done;
                    }
                    break;
                case 1:
                    sink -= outer * 2;
                    /* Continue to next iteration */
                    continue;
                case 2:
                    sink *= 2;
                    break;
            }
            
            /* Another conditional */
            if (sink < -500) {
                break;
            }
        }
    inner_done:
        /* Empty but creates a basic block */
        ;
    }
    
    return result > 0 ? 0 : 1;
}

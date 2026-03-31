#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n / 4; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < 8; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < 4; k++) {
                sum += arr[i * 16 + j * 2 + k];
                
                /* Multiple exit point */
                if (sum > LIMIT) {
                    break;
                }
            }
            
            /* Loop-invariant code */
            int stride = 2;  /* Loop invariant */
            if (j % stride == 0) {
                sum += arr[i * 16 + j];
            }
        }
        
        /* Another exit point from outer loop */
        if (sum > LIMIT * 2) {
            break;
        }
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop - will share blocks with second loop via goto */
    while (i < n / 2) {
        result += arr[i];
        
        /* Conditional goto to shared block */
        if (arr[i] % 3 == 0) {
            goto shared_block;
        }
        
        i++;
        continue;
        
    shared_block:
        result += arr[i] * 2;
        i++;
    }
    
    /* Second loop - overlaps with first via shared_block */
    int j = n / 2;
    while (j < n) {
        result -= arr[j];
        
        /* Same shared block as first loop */
        if (arr[j] % 5 == 0) {
            goto shared_block2;
        }
        
        j++;
        continue;
        
    shared_block2:
        /* This creates partial overlap - same block as in first loop's goto target */
        result += arr[j] * 3;
        j++;
    }
    
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int total = 0;
    int idx = 0;
    
outer_loop:
    while (idx < n) {
        /* Complex switch inside loop */
        switch (arr[idx] % 4) {
            case 0:
                total += arr[idx];
                idx++;
                break;
            case 1:
                total -= arr[idx];
                idx += 2;
                /* This break exits the switch, not the loop */
                break;
            case 2:
                total *= 2;
                /* This goto breaks out of the entire loop structure */
                if (total > 10000) {
                    goto loop_exit;
                }
                idx++;
                break;
            case 3:
                /* Nested loop inside case */
                for (int m = 0; m < 3; m++) {
                    total += m * arr[idx];
                    if (total < 0) {
                        /* Break from inner for loop only */
                        break;
                    }
                }
                idx++;
                break;
        }
        
        /* Multiple exit points */
        if (total > 5000) {
            break;
        }
        
        if (idx > n / 2 && total < 0) {
            goto outer_loop;  /* Continue outer loop */
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with potential overlap */
static int function_d(int *arr, int n) {
    int acc = 0;
    
    /* First adjacent loop */
    for (int x = 0; x < n / 3; x++) {
        acc += arr[x];
        
        /* Early exit creates multiple blocks */
        if (acc > 2000) {
            break;
        }
    }
    
    /* Second adjacent loop - shares no blocks but follows immediately */
    for (int y = n / 3; y < 2 * n / 3; y++) {
        acc -= arr[y];
        
        /* Different exit condition */
        if (acc < -1000) {
            break;
        }
    }
    
    /* Third loop that jumps back to previous blocks */
    int z = 2 * n / 3;
    while (z < n) {
        acc += arr[z] * 2;
        
        /* Jump to middle of first loop's conceptual space */
        if (arr[z] % 7 == 0) {
            /* This creates control flow that might confuse loop analysis */
            z += 2;
            continue;
        }
        
        z++;
    }
    
    return acc;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent optimization */
    volatile int size = SIZE;
    int *array = (int *)malloc(size * sizeof(int));
    
    if (!array) {
        return 1;
    }
    
    /* Initialize array with values */
    for (int i = 0; i < size; i++) {
        array[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    int result_a = function_a(array, size);
    int result_b = function_b(array, size);
    int result_c = function_c(array, size);
    int result_d = function_d(array, size);
    
    /* Combine results to prevent elimination */
    int final_result = result_a + result_b + result_c + result_d;
    
    /* Use printf to ensure code isn't eliminated */
    printf("Result: %d\n", final_result);
    
    free(array);
    
    return final_result != 0 ? 0 : 1;
}

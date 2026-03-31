#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    int stride = 3;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += stride) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < stride; j++) {
            if (i + j >= n) break;
            sum += arr[i + j];
        }
        
        /* Second inner loop - also fully contained */
        int k = 0;
        while (k < stride) {
            if (i + k >= n) break;
            sum -= arr[i + k] / 2;
            k++;
        }
        
        /* Multiple exit point */
        if (sum > LIMIT) break;
    }
    return sum;
}

/* Function B: Overlapping loops with goto creating shared basic blocks */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    while (i < n / 2) {
        result += arr[i] * 2;
        i++;
        if (result < 0) {
            goto common_block;  /* Jump to shared block */
        }
    }
    
    i = n / 2;
    
    /* Second loop - partially overlaps with first via common_block */
    while (i < n) {
        result -= arr[i];
        i++;
        
common_block:
        /* Shared basic block between both loops */
        result = (result & 0xFF) + 1;
        
        if (i % 7 == 0) break;  /* Multiple exit */
    }
    
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
outer_loop:
    for (int i = 0; i < n; i++) {
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                total += arr[i];
                if (total > 500) {
                    state = 1;
                    break;  /* This breaks the switch, not the loop */
                }
                break;
            case 1:
                total -= arr[i] / 2;
                if (i % 3 == 0) {
                    goto exit_loop;  /* Breaks out of the entire loop */
                }
                break;
            case 2:
                total *= 2;
                break;
        }
        
        /* Loop-invariant code */
        int invariant = n * 2;
        if (total > invariant) {
            state = 2;
        }
        
        /* Another exit point */
        if (total < -LIMIT) break;
    }
    
    if (state < 2 && n > 10) {
        state++;
        goto outer_loop;  /* Creates loop with multiple entries */
    }

exit_loop:
    return total;
}

/* Function D: Adjacent loops with potential overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n; i += 2) {
        acc1 += arr[i];
        if (acc1 > 1000) {
            /* Early exit creates additional basic block */
            break;
        }
    }
    
    /* Second adjacent loop - shares some control flow patterns */
    int j = 1;
    while (j < n) {
        acc2 += arr[j];
        j += 2;
        
        /* Conditional that might create overlapping CFG regions */
        if (acc2 > acc1 && j < n/2) {
            /* Nested loop inside */
            for (int k = 0; k < 3; k++) {
                acc2 += k;
            }
        }
    }
    
    return acc1 + acc2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call functions with different slices to create varied loop structures */
    int result = 0;
    
    /* Function A: Fully contained nested loops */
    result += function_a(arr, SIZE);
    
    /* Function B: Overlapping loops with goto */
    result += function_b(arr + 100, SIZE - 100);
    
    /* Function C: Loop with complex switch and breaks */
    result += function_c(arr + 200, SIZE - 200);
    
    /* Function D: Adjacent loops */
    result += function_d(arr + 300, SIZE - 300);
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}

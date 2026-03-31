#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested loops with fully contained sub-loops */
static int func_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            sum += arr[j];
            
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < j; k++) {
                sum -= arr[k];
                if (sum > LIMIT) break;  /* Multiple exit point */
            }
            
            if (sum < -LIMIT) break;  /* Another exit point */
        }
        
        /* Second inner loop - also fully contained in outer */
        int stride = 2;  /* Loop-invariant */
        for (int j = 0; j < n; j += stride) {
            sum += arr[i * stride + j % n];
            if (sum > LIMIT * 2) break;
        }
    }
    
    return sum;
}

/* Function B: Two loops that share a common basic block via goto */
static int func_b(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* First loop with partial overlap */
loop1_start:
    while (i < n / 2) {
        sum += arr[i];
        i++;
        
        if (sum % 7 == 0) {
            goto common_block;  /* Jump to shared block */
        }
        
        if (i > n / 4) break;
    }
    
    i = n / 2;
    
    /* Second loop that overlaps with first via common_block */
    while (i < n) {
        sum -= arr[i];
        i++;
        
        if (sum % 5 == 0) {
            goto common_block;  /* Same shared block */
        }
        
        if (i > 3 * n / 4) break;
    }
    
    goto done;
    
common_block:  /* Shared basic block between the two loops */
    sum *= 2;
    if (i < n) {
        goto loop1_start;  /* Creates irreducible flow */
    }
    
done:
    return sum;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int func_c(int *arr, int n) {
    int sum = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        sum += arr[i];
        
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                if (sum > 500) state = 1;
                sum += i;
                break;
            case 1:
                if (sum < -500) state = 2;
                sum -= i * 2;
                break;
            case 2:
                /* This break exits the for loop, not just the switch */
                if (sum > 1000) {
                    goto loop_exit;
                }
                sum /= 2;
                break;
            default:
                sum = 0;
                break;
        }
        
        /* Another exit point */
        if (i > n / 3 && sum < -LIMIT) break;
    }
    
loop_exit:
    return sum;
}

/* Function D: Adjacent loops with potential overlap analysis */
static int func_d(int *arr, int n) {
    int sum = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n; i += 2) {
        sum += arr[i];
        if (sum > LIMIT) {
            /* Early exit creates separate basic block */
            break;
        }
    }
    
    /* Second adjacent loop - shares no blocks with first */
    for (int i = 1; i < n; i += 2) {
        sum -= arr[i];
        /* Multiple basic blocks within loop */
        if (sum < -LIMIT) {
            sum = 0;
        } else {
            sum += i;
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i - SIZE/2;  /* Mix of positive and negative */
    }
    
    /* Call functions with different slices to create varied loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += func_a(arr, SIZE / 4);
    
    /* Function B: Overlapping loops via goto */
    result += func_b(arr + SIZE / 4, SIZE / 4);
    
    /* Function C: Loop with switch and complex break */
    result += func_c(arr + SIZE / 2, SIZE / 4);
    
    /* Function D: Adjacent loops */
    result += func_d(arr + 3 * SIZE / 4, SIZE / 4);
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

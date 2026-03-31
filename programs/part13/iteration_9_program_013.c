#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    int stride = 3; /* Loop-invariant variable */
    
    /* Outer loop - fully contains inner loops */
    for (int i = 1; i < n - 1; i++) {
        /* First inner loop - fully contained */
        for (int j = 1; j < i && j < 10; j++) {
            sum += arr[i * stride + j];
            if (sum > LIMIT) break; /* Multiple exit point */
        }
        
        /* Second inner loop - also fully contained */
        int k = 0;
        while (k < 5) {
            sum -= arr[i - k];
            k++;
            if (sum < -LIMIT) break; /* Another exit point */
        }
    }
    return sum;
}

/* Function B: Loops with overlapping basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop - shares common block with second loop */
loop1:
    for (; i < n/2; i++) {
        result += arr[i] * 2;
        if (result > LIMIT * 2) {
            goto common_block; /* Jump to shared block */
        }
    }
    
    i = n/2;
    
    /* Second loop - overlaps with first via common_block */
    while (i < n) {
        result -= arr[i];
        i++;
        if (i % 7 == 0) {
            goto common_block; /* Jump to shared block */
        }
    }
    
    goto done;
    
common_block: /* Shared basic block between the two loops */
    result /= 2;
    if (i < n) {
        goto loop1; /* Creates irreducible flow */
    }
    
done:
    return result;
}

/* Function C: Loop with switch and break to outside */
static int function_c(int *arr, int n) {
    int total = 0;
    int mode = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break to outside loop */
        switch (mode) {
            case 0:
                if (total > LIMIT) {
                    mode = 1;
                }
                break;
            case 1:
                if (arr[i] % 2 == 0) {
                    total *= 2;
                }
                /* This break exits the switch, not the loop */
                break;
            case 2:
                /* This will exit the entire loop when triggered */
                goto loop_exit;
            default:
                break;
        }
        
        /* Conditional break creates another exit point */
        if (total > LIMIT * 3) {
            break;
        }
        
        /* Nested loop inside */
        for (int j = 0; j < 3 && (i + j) < n; j++) {
            total += arr[i + j] / 2;
        }
    }
    
    return total;

loop_exit:
    return total * 2;
}

/* Function D: Complex partial overlap scenario */
static int function_d(int *arr, int n) {
    int acc = 0;
    int i = 0, j = 0;
    
    /* Two loops that partially overlap */
    
    /* Loop X */
x_loop:
    while (i < n/3) {
        acc += arr[i] * i;
        i++;
        
        /* Shared computation block */
        if (acc % 5 == 0) {
            goto shared_computation;
        }
    }
    
    /* Loop Y - starts where X might have left off */
    j = n/3;
y_loop:
    do {
        acc -= arr[j];
        j--;
        
shared_computation:
        /* This block is shared between both loops */
        acc = (acc < 0) ? -acc : acc;
        
        if (j > n/4 && i < n/2) {
            /* Creates partial overlap scenario */
            goto x_loop;
        }
    } while (j > 0 && i < n);
    
    return acc;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100; /* Prevent overflow in calculations */
    }
    
    volatile int sink = 0; /* Prevent dead code elimination */
    
    /* Call all functions to create various loop structures */
    int result_a = function_a(arr, SIZE / 2);
    int result_b = function_b(arr + SIZE / 4, SIZE / 2);
    int result_c = function_c(arr + SIZE / 3, SIZE / 3);
    int result_d = function_d(arr, SIZE);
    
    /* Combine results to ensure all code is used */
    int final_result = result_a + result_b + result_c + result_d;
    
    /* Use printf to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return final_result % 100;
}

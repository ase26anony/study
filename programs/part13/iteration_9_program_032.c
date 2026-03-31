#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += 4) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < 8; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < 4; k++) {
                sum += arr[i + j + k];
                if (sum > LIMIT) {
                    /* Multiple exit point */
                    break;
                }
            }
            /* Loop-invariant calculation */
            int stride = 2;
            sum += arr[j * stride];
        }
        
        /* Another inner loop at same level as middle loop */
        for (int m = 0; m < 3; m++) {
            sum -= arr[i + m];
            if (sum < -LIMIT) {
                break;
            }
        }
    }
    return sum;
}

/* Function B: Loops with partial overlap via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    while (i < n / 2) {
        result += arr[i];
        i++;
        
        if (result % 7 == 0) {
            goto common_block;  /* Jump to shared basic block */
        }
    }
    
    i = n / 2;
    
    /* Second loop - partially overlaps with first via common_block */
    while (i < n) {
        result -= arr[i];
        i++;
        
        if (result % 5 == 0) {
            goto common_block;  /* Same shared block */
        }
        
        /* Different path for second loop */
        if (i > n * 3 / 4) {
            result *= 2;
        }
    }
    
    return result;

common_block:  /* Shared basic block between both loops */
    result = (result * 3) / 2;
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int total = 0;
    int i = 0;
    
outer_loop:
    while (i < n) {
        total += arr[i];
        
        /* Switch inside loop with break to label */
        switch (i % 4) {
            case 0:
                total += 1;
                break;
            case 1:
                total += 2;
                /* This break only exits switch, not loop */
                break;
            case 2:
                total += 3;
                /* Complex control flow */
                if (total > 500) {
                    i++;
                    goto outer_loop;  /* Continue loop */
                }
                break;
            case 3:
                total += 4;
                /* This could trigger the break-to-label analysis */
                if (total < -100) {
                    break;  /* Normal switch break */
                }
                /* Fall through */
        }
        
        /* Another loop inside the outer loop */
        for (int j = 0; j < 3; j++) {
            total += arr[i + j];
            if (total > 1000) {
                goto exit_early;  /* Break out of nested structure */
            }
        }
        
        i++;
    }
    
    return total;

exit_early:
    return total * 2;
}

/* Function D: Irreducible control flow with overlapping loops */
static int function_d(int *arr, int n) {
    int acc = 0;
    int i = 0, j = 0;
    
loop1:
    for (; i < n / 3; i++) {
        acc += arr[i];
        
        if (acc % 11 == 0) {
            j = 0;
            goto loop2;  /* Jump to another loop */
        }
    }
    
    i = n / 3;
    
loop2:
    for (; j < n / 4; j++) {
        acc -= arr[j];
        
        if (acc % 13 == 0) {
            goto loop1;  /* Jump back */
        }
        
        /* Shared computation block */
        int temp = arr[i] + arr[j];
        acc += temp / 2;
    }
    
    return acc;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call functions with different slices to create varied loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, SIZE);
    
    /* Function B: Partially overlapping loops via goto */
    result += function_b(arr + 100, SIZE - 100);
    
    /* Function C: Loop with switch and complex breaks */
    result += function_c(arr + 200, SIZE - 200);
    
    /* Function D: Irreducible control flow */
    result += function_d(arr + 300, SIZE - 300);
    
    /* Additional complex loop in main */
    volatile int sink = 0;
    for (int outer = 0; outer < 10; outer++) {
        /* Sibling loops that might share blocks */
        for (int inner1 = 0; inner1 < 5; inner1++) {
            sink += arr[outer * 10 + inner1];
            if (sink > 500) {
                goto shared_computation;
            }
        }
        
        for (int inner2 = 5; inner2 < 10; inner2++) {
            sink -= arr[outer * 10 + inner2];
            if (sink < -500) {
                goto shared_computation;
            }
        }
        
        continue;
        
    shared_computation:  /* Block shared by both inner loops */
        sink = sink / 2;
        result += sink;
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}

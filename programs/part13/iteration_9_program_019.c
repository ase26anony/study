#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < j; k++) {
                sum += arr[i * 16 + j * 8 + k];
            }
            /* Multiple exit point */
            if (sum > LIMIT) break;
        }
        /* Another exit point */
        if (sum > LIMIT * 2) break;
    }
    return sum;
}

/* Function B: Overlapping loops with shared basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - partially overlaps with second */
loop1_start:
    for (; i < n/2; i++) {
        result += arr[i * 3];
        if (result % 7 == 0) {
            goto shared_block;  /* Jump to shared basic block */
        }
        if (i > n/4) break;
    }
    
    /* Second loop - shares block with first */
    i = n/4;
loop2_start:
    for (; i < n; i++) {
        result -= arr[i * 2];
        if (result < 0) {
            goto shared_block;  /* Same shared block */
        }
        if (i > n * 3/4) break;
    }
    
    goto finish;
    
shared_block:  /* Shared basic block between both loops */
    result *= 2;
    if (i < n/2) {
        i++;
        goto loop1_start;
    } else {
        i++;
        goto loop2_start;
    }
    
finish:
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                total += arr[i];
                if (total > 500) state = 1;
                break;
            case 1:
                total -= arr[i];
                if (total < 0) state = 2;
                break;
            case 2:
                total *= arr[i];
                if (i % 3 == 0) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* Exit loop from within switch */
                if (i > n/2) goto loop_exit;
                break;
            default:
                /* Another exit point */
                if (total > LIMIT) return total;
        }
        
        /* Loop-invariant calculation with strength reduction potential */
        int stride = 4;  /* Loop invariant */
        total += arr[i * stride] % 5;
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First adjacent loop */
    int m = n/3;
    for (int i = 0; i < m; i++) {
        acc1 += arr[i];
        /* Conditional jump to overlap region */
        if (acc1 % 11 == 0 && i > m/2) {
            goto overlap_region;
        }
    }
    
    /* Second adjacent loop - partially overlaps */
    for (int i = m/2; i < n; i++) {
        acc2 -= arr[i];
        if (acc2 < -100) {
            goto overlap_region;
        }
    }
    
    return acc1 + acc2;
    
overlap_region:  /* Region that both loops can reach */
    int temp = acc1 * acc2;
    if (temp > 0) {
        return temp;
    } else {
        /* Jump back to different loops based on condition */
        if (acc1 > acc2) {
            m++;
            goto overlap_region;  /* Self-loop creates irreducible flow */
        }
        return acc1 - acc2;
    }
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, 32);
    
    /* Function B: Overlapping loops with goto */
    result += function_b(arr, 64);
    
    /* Function C: Loop with complex switch */
    result += function_c(arr, 128);
    
    /* Function D: Adjacent loops with partial overlap */
    result += function_d(arr, 256);
    
    /* Use volatile to prevent elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't dead */
    printf("Result: %d\n", result);
    
    return result % 256;
}

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += 4) {
        /* First inner loop - fully contained in outer */
        for (int j = i; j < i + 3 && j < n; j++) {
            sum += arr[j] * 2;
        }
        
        /* Second inner loop - also fully contained */
        int k = i;
        while (k < i + 2 && k < n) {
            sum -= arr[k];
            k++;
        }
        
        /* Multiple exit points */
        if (sum > LIMIT) {
            break;
        }
    }
    return sum;
}

/* Function B: Overlapping loops with goto creating shared basic blocks */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    while (i < n / 2) {
        result += arr[i] * 3;
        i++;
        
        /* Shared basic block via goto */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        /* Loop-invariant code */
        int stride = 2;  /* Loop invariant */
        if (i * stride < n) {
            result -= arr[i * stride];
        }
        
        continue;
        
    shared_block:
        /* This block is shared between both loops */
        result ^= 0xFF;
        if (result < 0) result = -result;
    }
    
    /* Second loop that shares the same basic block */
    int j = n / 2;
    while (j < n) {
        result += arr[j] * 5;
        j += 2;
        
        if (result % 11 == 0) {
            goto shared_block;  /* Same shared block */
        }
        
        /* Different computation to create distinct basic blocks */
        result |= 0x0F;
    }
    
    return result;
}

/* Function C: Loop with switch and break to outer scope */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int idx = 0; idx < n; idx++) {
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                total += arr[idx];
                if (total > 500) state = 1;
                break;
            case 1:
                total -= arr[idx];
                if (total < -100) state = 2;
                break;
            case 2:
                total *= 2;
                /* This break exits the switch, not the loop */
                break;
            case 3:
                /* This break exits the entire loop via label */
                goto loop_exit;
            default:
                total ^= arr[idx];
        }
        
        /* Multiple exit points */
        if (idx > n / 3 && total < -LIMIT) {
            break;
        }
        
        /* Nested while inside for */
        int temp = idx;
        while (temp > 0 && temp > idx - 5) {
            total += temp;
            temp--;
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First loop */
    for (int i = 0; i < n; i++) {
        acc1 += arr[i];
        
        /* Conditional that creates multiple basic blocks */
        if (i % 3 == 0) {
            acc1 *= 2;
        } else if (i % 3 == 1) {
            acc1 /= 2;
        }
        
        /* Early exit */
        if (acc1 > 10000) {
            break;
        }
    }
    
    /* Second loop - partially overlaps in control flow */
    int j = n / 4;
    do {
        acc2 += arr[j] * arr[j];
        j++;
        
        /* Same pattern as first loop but not identical */
        if (j % 4 == 0) {
            acc2 <<= 1;
        } else if (j % 4 == 2) {
            acc2 >>= 1;
        }
        
        /* Shared computation pattern but different blocks */
        if (acc2 > 5000 && j < n * 3 / 4) {
            /* This creates control flow that might overlap
               with first loop's analysis but isn't contained */
            acc2 = acc2 % 256;
        }
    } while (j < n);
    
    return acc1 + acc2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully contained nested loops */
    result += function_a(arr, SIZE);
    
    /* Function B: Overlapping loops with goto */
    result += function_b(arr + SIZE/4, SIZE/2);
    
    /* Function C: Loop with switch and complex control flow */
    result += function_c(arr + SIZE/2, SIZE/4);
    
    /* Function D: Adjacent loops with partial overlap */
    result += function_d(arr, SIZE);
    
    /* Use volatile to prevent elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

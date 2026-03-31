#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int func_a(int* arr, int n) {
    int sum = 0;
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += 4) {
        /* First inner loop - fully contained in outer */
        for (int j = i; j < i + 3 && j < n; j++) {
            /* Second inner loop - fully contained in first inner */
            for (int k = j; k < j + 2 && k < n; k++) {
                sum += arr[k] * (k - j + 1);
                /* Multiple exit point */
                if (sum > LIMIT) break;
            }
            /* Loop-invariant code */
            int stride = 2;
            if (j + stride < n) {
                sum += arr[j * stride % n];
            }
        }
        /* Another conditional exit */
        if (sum > LIMIT * 2) break;
    }
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int func_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
loop1:
    while (i < n / 2) {
        result += arr[i] * i;
        i++;
        /* Shared basic block via goto */
        if (i % 3 == 0) goto shared_block;
        continue;
        
shared_block:
        /* This block is shared between both loops */
        result -= arr[i % n];
        if (result < 0) result = 0;
        /* Continue to next iteration */
    }
    
    /* Second loop that shares the 'shared_block' */
    int j = n / 2;
loop2:
    while (j < n) {
        result += arr[j] * 2;
        j++;
        if (j % 4 == 0) goto shared_block;
        /* Different computation to create distinct blocks */
        result += j % 5;
    }
    
    return result;
}

/* Function C: Loop with switch and break to outer scope */
static int func_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int idx = 0; idx < n; idx++) {
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                total += arr[idx];
                if (total % 7 == 0) state = 1;
                break;
            case 1:
                total -= arr[idx] * 2;
                if (total < 0) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* Fall through */
            case 2:
                total *= (arr[idx] % 10 + 1);
                /* This label is used for goto from switch */
                if (total > 1000) {
                    goto exit_loop;
                }
                state = (state + 1) % 3;
                break;
            default:
                /* Multiple exit points */
                if (idx > n / 2) goto exit_loop;
        }
        
        /* Additional loop-invariant computation */
        static const int multiplier = 3;
        total += multiplier * (idx % 8);
    }
    
exit_loop:
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int func_d(int* arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First loop */
    for (int i = 0; i < n; i++) {
        acc1 += arr[i];
        /* Conditional that could create shared block with next loop */
        if (acc1 > 500) {
            acc1 /= 2;
        }
    }
    
    /* Second loop - partially overlaps due to similar structure */
    for (int j = 0; j < n; j++) {
        acc2 += arr[j] * j;
        /* Similar conditional block */
        if (acc2 > 500) {
            acc2 /= 2;
        }
        /* Different computation to ensure not identical */
        acc2 += (j % 3);
    }
    
    return acc1 + acc2;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += func_a(arr, SIZE / 2);
    
    /* Function B: Overlapping loops with goto */
    result += func_b(arr + SIZE / 2, SIZE / 2);
    
    /* Function C: Loop with switch and complex breaks */
    result += func_c(arr, SIZE / 3);
    
    /* Function D: Adjacent partially overlapping loops */
    result += func_d(arr + SIZE / 3, SIZE / 3);
    
    /* Use volatile to prevent elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

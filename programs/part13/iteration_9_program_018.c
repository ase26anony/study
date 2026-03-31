#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 32
#define MID_SIZE 64

static volatile int sink = 0;

/* Function A: Triple-nested loops with fully contained sub-loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += MID_SIZE) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < MID_SIZE && (i + j) < n; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[i + j] * k;
                if (sum > 1000000) break;  /* Multiple exit point */
            }
            /* Loop-invariant code */
            int stride = 4;  /* Loop invariant */
            if (j % stride == 0) {
                sum += arr[i + j] * stride;
            }
        }
        
        /* Another inner loop at same level - partially overlapping blocks */
        int acc = 0;
        for (int j = MID_SIZE/2; j < MID_SIZE && (i + j) < n; j++) {
            acc += arr[i + j];
            if (acc > 5000) break;
        }
        sum += acc;
    }
    
    return sum;
}

/* Function B: Two loops that share common basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with complex control flow */
loop1_start:
    while (i < n/2) {
        result += arr[i] * 2;
        i++;
        
        /* Conditional goto to shared block */
        if (arr[i] % 3 == 0) {
            goto shared_block;
        }
        
        if (result > 10000) {
            break;
        }
    }
    
    /* Reset for second loop */
    i = n/2;
    
    /* Second loop that shares blocks with first */
    while (i < n) {
        result -= arr[i];
        i++;
        
shared_block:
        /* Shared basic block between both loops */
        result ^= 0x55;
        
        if (i % 7 == 0) {
            continue;
        }
        
        if (result < -5000) {
            goto loop_end;
        }
    }
    
loop_end:
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                if (arr[i] > 100) {
                    state = 1;
                }
                break;
            case 1:
                if (total > 5000) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* Fall through */
            case 2:
                if (i % 10 == 0) {
                    /* This goto creates irreducible flow */
                    goto switch_exit;
                }
                break;
            default:
                /* Direct break from loop inside switch */
                if (total > 10000) {
                    goto loop_exit;
                }
        }
        
        /* Loop-invariant computation */
        int invariant = n / 4;
        total += arr[i] % invariant;
        
        continue;
        
    switch_exit:
        total += 1000;
        if (i > n/2) {
            break;  /* This breaks the for loop */
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Nested loops with partial overlap using if-else chains */
static int function_d(int *arr, int n) {
    int val = 0;
    int i = 0, j = 0;
    
    /* First loop structure */
    while (i < n) {
        val += arr[i];
        i += 2;
        
        /* Inner while that partially overlaps */
        j = 0;
        while (j < 10 && (i + j) < n) {
            val -= arr[i + j];
            j++;
            
            /* Multiple exit points */
            if (val < -1000) {
                goto partial_overlap;
            }
        }
        
        if (i > n/3) {
            break;
        }
    }
    
    /* Second loop that shares some blocks */
    i = n/3;
partial_overlap:
    do {
        val *= 2;
        i++;
        
        /* Another inner loop at different nesting level */
        for (int k = 0; k < 5; k++) {
            val += k;
            if (val > 50000) {
                goto done;
            }
        }
        
        if (i >= n) {
            break;
        }
    } while (val < 20000);
    
done:
    return val;
}

int main(int argc, char **argv) {
    /* Use command line arg to make size non-constant */
    int size = SIZE;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size <= 0) size = SIZE;
    }
    
    /* Initialize array with sequential values */
    int *arr = (int*)malloc(size * sizeof(int));
    if (!arr) return -1;
    
    for (int i = 0; i < size; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    result += function_a(arr, size);
    sink = result;  /* Volatile sink to prevent elimination */
    
    result += function_b(arr, size);
    sink = result;
    
    result += function_c(arr, size);
    sink = result;
    
    result += function_d(arr, size);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    free(arr);
    return result & 0xFF;  /* Return non-constant result */
}

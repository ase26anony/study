#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 16
#define MIDDLE_SIZE 8

static volatile int sink;

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < MIDDLE_SIZE; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[(i * MIDDLE_SIZE + j) * INNER_SIZE + k];
            }
            
            /* Multiple exit points */
            if (sum > 1000000) {
                break;
            }
        }
        
        /* Another conditional break */
        if (i > n / 2 && sum < 500000) {
            break;
        }
    }
    
    return sum;
}

/* Function B: Loops with partial overlap using goto */
static int function_b(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* First loop */
loop1:
    while (i < n / 2) {
        sum += arr[i];
        i++;
        
        /* Conditional goto to shared block */
        if (sum % 7 == 0) {
            goto shared_block;
        }
        
        if (i % 3 == 0) {
            continue;
        }
        
        sum += i;
    }
    
    i = n / 2;
    
    /* Second loop - partially overlaps with first via shared_block */
    while (i < n) {
        sum -= arr[i];
        i++;
        
        /* Another entry to shared block */
        if (sum % 5 == 0) {
            goto shared_block;
        }
        
        if (i % 4 == 0) {
            continue;
        }
        
        sum -= i * 2;
    }
    
    goto end;
    
shared_block:
    /* Shared basic block between the two loops */
    sum *= 2;
    
    /* Complex control flow back to appropriate loop */
    if (i < n / 2) {
        goto loop1;
    } else {
        /* Continue in second loop */
        if (i % 2 == 0) {
            sum += 100;
        }
    }
    
end:
    return sum;
}

/* Function C: Loop with switch and break to outside */
static int function_c(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    while (i < n) {
        /* Loop-invariant code */
        int stride = 3;
        
        switch (i % 4) {
            case 0:
                sum += arr[i * stride];
                break;
            case 1:
                sum += arr[i * stride + 1];
                /* This break exits the switch, not the loop */
                break;
            case 2:
                sum += arr[i * stride + 2];
                /* Conditional break that exits the entire loop */
                if (sum > 50000) {
                    goto loop_exit;
                }
                break;
            case 3:
                sum += arr[i * stride + 3];
                /* Another loop exit point */
                if (i > n / 3) {
                    i++;
                    break;
                }
                break;
        }
        
        /* Multiple basic blocks within loop */
        if (sum % 11 == 0) {
            sum /= 2;
        } else {
            sum *= 2;
        }
        
        i++;
        
        /* Yet another exit point */
        if (i > 50 && sum < -10000) {
            break;
        }
    }
    
loop_exit:
    return sum;
}

/* Function D: Complex nested loops with mixed structures */
static int function_d(int *arr, int n) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i += 4) {
        int chunk_sum = 0;
        
        /* Inner loop A */
        for (int j = 0; j < 4; j++) {
            if (i + j >= n) {
                goto next_outer;
            }
            
            chunk_sum += arr[i + j];
            
            /* Early exit from inner loop */
            if (chunk_sum > 1000) {
                break;
            }
        }
        
        /* Another inner loop B - partially overlapping blocks with A */
        for (int j = 0; j < 3; j++) {
            if (i + j >= n) {
                goto next_outer;
            }
            
            chunk_sum -= arr[i + j] / 2;
            
            /* Shared computation that could be in its own basic block */
            if (chunk_sum < 0) {
                chunk_sum = 0;
            }
        }
        
        total += chunk_sum;
        
        /* Label for goto target */
        next_outer:
        
        /* Loop invariant computation */
        int scale = 2;
        total *= scale;
        
        /* Prevent overflows */
        if (total > 1000000) {
            total = 1000000;
        }
    }
    
    return total;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, SIZE / (MIDDLE_SIZE * INNER_SIZE));
    
    /* Function B: Partially overlapping loops with goto */
    result += function_b(arr + 256, 256);
    
    /* Function C: Loop with switch and external break */
    result += function_c(arr + 512, 256);
    
    /* Function D: Mixed loop structures */
    result += function_d(arr + 768, 256);
    
    /* Use volatile sink to prevent elimination */
    sink = result;
    
    /* Also print to ensure code isn't dead */
    printf("Result: %d\n", result);
    
    return result % 256;
}

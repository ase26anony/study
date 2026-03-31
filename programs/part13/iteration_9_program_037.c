#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int* arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < j; k++) {
                sum += arr[(i * j + k) % SIZE];
                
                /* Multiple exit points */
                if (sum > LIMIT * 2) {
                    break;
                }
            }
            
            /* Loop-invariant code */
            int stride = 3;  /* Loop invariant */
            if (j % stride == 0) {
                sum -= arr[j];
            }
            
            if (sum < -LIMIT) {
                break;
            }
        }
        
        /* Another inner loop at same level - partially overlapping */
        int temp = 0;
        while (temp < 5) {
            sum += arr[i] * temp;
            temp++;
            
            if (i > n / 2 && sum > LIMIT) {
                break;
            }
        }
    }
    
    return sum;
}

/* Function B: Loops with shared basic blocks via goto */
static int function_b(int* arr, int start, int end) {
    int result = 0;
    int i = start;
    
    /* First loop with goto to shared block */
loop1:
    while (i < end) {
        result += arr[i];
        i++;
        
        if (result % 7 == 0) {
            goto shared_block;  /* Jump to shared basic block */
        }
        
        if (i > end - 10) {
            break;
        }
    }
    
    i = start + 5;
    
    /* Second loop that also uses the shared block */
    while (i < end - 5) {
        result -= arr[i];
        i += 2;
        
        if (result % 11 == 0) {
            goto shared_block;  /* Same shared block */
        }
        
        if (i > end - 3) {
            break;
        }
    }
    
    goto done;
    
shared_block:  /* Shared basic block between the two loops */
    result *= 2;
    if (result > LIMIT) {
        return result / 2;
    }
    
done:
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int* arr, int n) {
    int acc = 0;
    int i = 0;
    
    /* Loop with complex switch inside */
    while (i < n) {
        acc += arr[i];
        
        switch (i % 4) {
            case 0:
                acc += 1;
                break;
            case 1:
                acc -= 1;
                /* This break only exits the switch */
                break;
            case 2:
                /* This will exit the while loop entirely */
                if (acc > LIMIT) {
                    goto loop_exit;
                }
                acc *= 2;
                break;
            case 3:
                /* Nested loop inside case */
                for (int j = 0; j < 3; j++) {
                    acc += j;
                    if (acc < -LIMIT) {
                        break;  /* Only exits the for loop */
                    }
                }
                break;
        }
        
        i++;
        
        /* Multiple exit conditions */
        if (acc > LIMIT * 3) {
            break;
        }
    }
    
loop_exit:
    return acc;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int* arr, int n) {
    int sum1 = 0, sum2 = 0;
    
    /* First loop */
    for (int i = 0; i < n; i += 2) {
        sum1 += arr[i];
        
        /* Conditional that creates extra basic blocks */
        if (sum1 % 5 == 0) {
            sum1 -= arr[i / 2];
        }
    }
    
    /* Second loop that shares some control flow pattern */
    for (int i = 1; i < n; i += 2) {
        sum2 += arr[i];
        
        /* Same pattern as first loop but not identical */
        if (sum2 % 5 == 0) {
            sum2 -= arr[i / 2];
        }
        
        /* Additional exit point */
        if (i > n / 2 && sum2 > sum1) {
            break;
        }
    }
    
    /* Common tail code - creates partial overlap in CFG */
    if (sum1 > sum2) {
        return sum1 - sum2;
    } else {
        return sum2 - sum1;
    }
}

int main(int argc, char** argv) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i - SIZE/2;  /* Mix positive and negative */
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    volatile int a_res = function_a(arr, 50);
    result += a_res;
    
    /* Function B: Loops with shared blocks via goto */
    volatile int b_res = function_b(arr, 100, 200);
    result += b_res;
    
    /* Function C: Loop with switch and complex breaks */
    volatile int c_res = function_c(arr, 150);
    result += c_res;
    
    /* Function D: Adjacent loops with similar structure */
    volatile int d_res = function_d(arr, 100);
    result += d_res;
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", result);
    
    /* Additional complex loop in main to increase coverage */
    int final_check = 0;
    
    /* Loop that combines multiple patterns */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner loop with multiple exits */
        int inner = 0;
        while (inner < 20) {
            final_check += arr[(outer * 20 + inner) % SIZE];
            inner++;
            
            if (final_check > LIMIT) {
                goto partial_overlap;
            }
        }
        
        continue;
        
    partial_overlap:
        /* This label creates overlapping loop regions */
        final_check /= 2;
        
        /* Another nested loop */
        for (int k = 0; k < 5; k++) {
            final_check += k;
            if (k == 3) {
                break;  /* Only exits the k loop */
            }
        }
    }
    
    result += final_check;
    
    return result % 256;  /* Prevent overflow in return */
}

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
                
                /* Multiple exit points */
                if (sum > LIMIT) {
                    break;  /* Exits innermost loop */
                }
            }
            
            /* Loop-invariant code */
            int stride = 2;  /* Loop invariant */
            if (j % stride == 0) {
                sum += arr[i * stride];
            }
            
            if (sum > LIMIT * 2) {
                break;  /* Exits middle loop */
            }
        }
        
        /* Another conditional exit */
        if (i > n / 2 && sum > LIMIT / 2) {
            break;
        }
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop - will share basic blocks with second loop */
loop1_start:
    while (i < n / 2) {
        result += arr[i];
        i++;
        
        /* Conditional jump to shared block */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        if (i > n / 4) {
            break;
        }
    }
    
    i = n / 2;
    
    /* Second loop - overlaps with first via shared_block */
    while (i < n) {
        result -= arr[i];
        i++;
        
        /* Another entry to shared block */
        if (result % 5 == 0) {
            goto shared_block;
        }
        
        if (i > 3 * n / 4) {
            break;
        }
    }
    
    goto done;

shared_block:
    /* Shared basic block between both loops */
    result *= 2;
    
    /* Complex control flow back to loops */
    if (result > 0) {
        goto loop1_start;
    }

done:
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int total = 0;
    int i = 0;
    
    /* Label for switch break target */
    outer_loop:
    while (i < n) {
        total += arr[i];
        
        /* Switch inside loop with break to outer label */
        switch (i % 4) {
            case 0:
                total += 1;
                break;  /* Normal switch break */
            case 1:
                total += 2;
                /* This could potentially create complex CFG */
                if (total > LIMIT) {
                    i++;
                    continue;  /* Continue loop */
                }
                break;
            case 2:
                total += 3;
                /* Conditional break from loop */
                if (total > LIMIT * 2) {
                    i++;
                    break;  /* Breaks switch, not loop */
                }
                break;
            case 3:
                total += 4;
                /* Multiple exit points */
                if (total % 13 == 0) {
                    goto exit_loop;  /* Exits loop entirely */
                }
                break;
        }
        
        /* Loop-invariant computation */
        int invariant = n / 2;
        total += arr[invariant];
        
        i++;
        
        /* Another conditional exit */
        if (i > n / 3 && total < 0) {
            break;
        }
        
        continue;
        
    exit_loop:
        /* Target for goto from switch */
        break;  /* Exits while loop */
    }
    
    /* Additional loop that might overlap with previous */
    for (int j = 0; j < n; j += 2) {
        total -= arr[j];
        
        /* Nested switch */
        switch (total % 3) {
            case 0:
                if (j > n / 2) {
                    goto outer_loop;  /* Jumps back to previous loop */
                }
                break;
            default:
                break;
        }
    }
    
    return total;
}

/* Function D: Complex irreducible control flow */
static int function_d(int *arr, int n) {
    int acc = 0;
    int i = 0, j = 0;
    
    /* Two loops that might be considered overlapping */
loop_x:
    for (; i < n; i++) {
        acc += arr[i];
        
        if (acc % 11 == 0) {
            j = 0;
            goto loop_y;
        }
        
        if (i > n / 2) {
            break;
        }
    }
    
    i = n / 3;
    
loop_y:
    while (j < n / 2) {
        acc -= arr[j];
        j++;
        
        if (acc % 13 == 0) {
            goto loop_x;
        }
        
        /* Shared computation block */
        int temp = acc * 2;
        if (temp > LIMIT) {
            goto shared_compute;
        }
    }
    
    goto finish;

shared_compute:
    /* Block shared between control paths */
    acc /= 2;
    
    /* Jump back to different loops based on condition */
    if (acc > 0) {
        goto loop_x;
    } else {
        goto loop_y;
    }

finish:
    return acc;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent dead code elimination */
    volatile int prevent_elimination = 0;
    
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Call all functions to create various loop structures */
    int result_a = function_a(arr, SIZE / 2);
    int result_b = function_b(arr + SIZE / 4, SIZE / 2);
    int result_c = function_c(arr + SIZE / 3, SIZE / 3);
    int result_d = function_d(arr, SIZE / 4);
    
    /* Combine results to ensure all code is used */
    int final_result = result_a + result_b + result_c + result_d;
    
    /* Use printf to prevent optimization */
    printf("Result: %d\n", final_result);
    
    /* Also return to main to ensure execution */
    return final_result % 256;
}

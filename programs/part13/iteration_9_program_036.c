#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            /* Innermost loop - fully contained in both */
            for (int k = 0; k < j; k++) {
                sum += arr[(i * j + k) % n];  /* Loop-invariant stride pattern */
            }
            
            /* Multiple exit points */
            if (sum > LIMIT) {
                break;  /* Exits j-loop */
            }
        }
        
        /* Another inner loop at same level */
        for (int j = n - 1; j > 0; j--) {
            sum -= arr[(i + j) % n];
            if (sum < -LIMIT) {
                break;
            }
        }
    }
    
    return sum;
}

/* Function B: Loops with overlapping blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with partial overlap */
loop1_start:
    while (i < n / 2) {
        result += arr[i * 2];  /* Loop-invariant stride */
        
        /* Conditional goto to shared block */
        if (arr[i] % 3 == 0) {
            goto shared_block;
        }
        
        result -= arr[i];
        i++;
        continue;
        
shared_block:
        /* Shared basic block between loops */
        result *= 2;
        i++;
        continue;
    }
    
    /* Reset for second loop */
    i = n / 2;
    
    /* Second loop that also uses shared_block */
loop2_start:
    while (i < n) {
        result += arr[i];
        
        /* Different condition to reach shared block */
        if (arr[i] % 5 == 0) {
            goto shared_block;
        }
        
        result /= (arr[i] + 1);
        i++;
    }
    
    return result;
}

/* Function C: Loop with switch and complex break */
static int function_c(int *arr, int n) {
    int acc = 0;
    int i = 0;
    
    while (i < n) {
        /* Switch with break to outside loop */
        switch (arr[i] % 4) {
            case 0:
                acc += arr[i];
                break;  /* Normal switch break */
            case 1:
                acc -= arr[i];
                break;
            case 2:
                acc *= 2;
                /* This break exits the while loop entirely */
                goto loop_exit;
            case 3:
                acc /= (arr[i] + 1);
                break;
        }
        
        /* Multiple exit points */
        if (acc > LIMIT * 2) {
            break;
        }
        
        i++;
        continue;
        
loop_exit:
        /* Target of goto from switch */
        break;  /* Exits while loop */
    }
    
    return acc;
}

/* Function D: Irreducible control flow with nested loops */
static int function_d(int *arr, int n) {
    int total = 0;
    int outer = 0;
    
    /* Outer loop */
    for (outer = 0; outer < n; outer += 3) {
        int inner = 0;
        
        /* Label for potential goto from inner loop */
        mid_point:
        
        /* Inner loop that can jump out */
        for (inner = 0; inner < outer; inner++) {
            total += arr[outer + inner];
            
            /* Conditional jump to label outside inner loop */
            if (total > LIMIT && inner > 5) {
                goto continue_outer;
            }
            
            /* Another conditional jump */
            if (arr[inner] % 7 == 0) {
                goto mid_point;
            }
        }
        
        continue_outer:
        
        /* Another loop at same level as outer */
        for (int k = 0; k < 5; k++) {
            total -= k;
            /* This creates overlapping but not contained blocks */
            if (k == 2) {
                goto mid_point;  /* Jump back */
            }
        }
    }
    
    return total;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call all functions to create various loop structures */
    int result_a = function_a(arr, SIZE / 4);
    int result_b = function_b(arr + SIZE / 4, SIZE / 4);
    int result_c = function_c(arr + SIZE / 2, SIZE / 4);
    int result_d = function_d(arr + 3 * SIZE / 4, SIZE / 4);
    
    /* Combine results to prevent elimination */
    int final_result = result_a + result_b + result_c + result_d;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}

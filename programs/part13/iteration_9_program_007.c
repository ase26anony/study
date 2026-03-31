#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define N 32

/* Function A: Triple-nested fully contained loops */
static int func_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < j; k++) {
                sum += arr[i * N + j + k];
            }
            
            /* Multiple exit points */
            if (sum > 1000000) {
                break;
            }
        }
        
        /* Loop-invariant code */
        int stride = 2;
        sum += arr[i * stride];
    }
    
    return sum;
}

/* Function B: Loops with overlapping blocks via goto */
static int func_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
loop1_start:
    if (i >= n) goto loop1_end;
    
    result += arr[i];
    i++;
    
    /* Shared basic block */
shared_block:
    result ^= 0x55;
    
    if (i % 3 == 0) goto loop1_start;
    
    /* Second loop that shares the shared_block */
    int j = 0;
loop2_start:
    if (j >= n/2) goto loop2_end;
    
    result -= arr[j];
    j++;
    
    /* Both loops can jump to shared_block */
    if (j % 2 == 0) goto shared_block;
    goto loop2_start;
    
loop1_end:
loop2_end:
    
    return result;
}

/* Function C: Loop with switch and break to exit loop */
static int func_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                if (total > 1000) state = 1;
                break;
            case 1:
                if (total > 2000) state = 2;
                /* This break exits the switch, not the loop */
                break;
            case 2:
                /* This will cause a break to loop_exit label */
                if (total > 3000) goto loop_exit;
                break;
            default:
                total *= 2;
        }
        
        /* Multiple basic blocks in loop body */
        if (i % 5 == 0) {
            total += 1;
        } else if (i % 7 == 0) {
            total -= 1;
        }
        
        continue;
        
    loop_exit:
        /* Break from loop via goto */
        break;
    }
    
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int func_d(int *arr, int n) {
    int acc = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n/2; i++) {
        acc += arr[i];
        
        /* Conditional that could jump to second loop's territory */
        if (acc < 0) {
            /* This creates potential overlap in control flow */
            acc = -acc;
        }
    }
    
    /* Common computation block */
    acc = (acc * 13) % 97;
    
    /* Second adjacent loop - shares the common block */
    for (int j = n/2; j < n; j++) {
        acc -= arr[j];
        
        /* Same computation as in first loop's conditional */
        if (acc < 0) {
            acc = -acc;
        }
    }
    
    return acc;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
    }
    
    /* Call all functions to create various loop structures */
    int sum = 0;
    
    /* Function A: Fully nested loops */
    sum += func_a(arr, 16);
    
    /* Function B: Overlapping loops via goto */
    sum += func_b(arr + 256, 64);
    
    /* Function C: Loop with switch and break */
    sum += func_c(arr + 512, 128);
    
    /* Function D: Adjacent loops with shared blocks */
    sum += func_d(arr + 768, 128);
    
    /* Use volatile to prevent dead code elimination */
    volatile int result = sum;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}

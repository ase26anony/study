#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += 4) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < 8; j++) {
            sum += arr[i] * j;
        }
        
        /* Second inner loop - also fully contained */
        for (int k = 0; k < 4; k++) {
            int val = arr[i + k];
            /* Multiple exit points */
            if (val > LIMIT) break;
            if (sum > 5000) break;
            sum += val * k;
        }
        
        /* Third level nesting */
        for (int x = 0; x < 2; x++) {
            for (int y = 0; y < 3; y++) {
                sum += arr[i + x + y] * (x + y);
                /* Loop-invariant code with strength reduction potential */
                int stride = 2;  /* Loop invariant */
                sum += arr[i * stride] % 7;
            }
        }
    }
    return sum;
}

/* Function B: Overlapping loops with goto creating shared basic blocks */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    while (i < n / 2) {
        result += arr[i] * 2;
        i++;
        
        /* Conditional break creating multiple basic blocks */
        if (result > 2000) {
            goto common_block;  /* Jump to shared block */
        }
        
        if (i % 7 == 0) {
            continue;
        }
        
        result += i;
    }
    
    i = n / 2;
    
    /* Second loop that shares basic blocks with first via goto */
    while (i < n) {
        result -= arr[i];
        i += 2;
        
        if (result < -1000) {
            goto common_block;  /* Same shared block */
        }
        
        /* Different computation path */
        result += i * 3;
    }
    
common_block:  /* Shared basic block between the two loops */
    result = result % 100;
    
    /* Another loop that partially overlaps with previous ones */
    for (int j = 0; j < n; j++) {
        if (j < n / 3) {
            result += arr[j];
        } else {
            result -= arr[j];
            /* Break to different location */
            if (result < -500) break;
        }
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int switch_in_loop(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                total += arr[i];
                if (arr[i] > 500) state = 1;
                break;
            case 1:
                total -= arr[i] * 2;
                if (arr[i] < 100) state = 2;
                break;
            case 2:
                total += arr[i] / 3;
                /* This break exits the SWITCH, not the loop */
                break;
            case 3:
                total *= 2;
                /* This goto breaks out of the loop entirely */
                if (total > 10000) goto loop_exit;
                break;
        }
        
        /* Multiple exit points from loop */
        if (total > 5000) {
            break;  /* Breaks the FOR loop */
        }
        
        if (i > n / 2 && total < 0) {
            break;
        }
        
        /* Loop-invariant computation */
        int invariant = n / 4;
        total += invariant % 5;
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with potential for hierarchy confusion */
static int adjacent_loops(int *arr, int n) {
    int acc = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n; i += 3) {
        acc += arr[i];
        /* Early exit creates additional basic block */
        if (acc > 3000) break;
    }
    
    /* Second adjacent loop - shares no blocks with first */
    for (int j = 1; j < n; j += 2) {
        acc -= arr[j];
        /* Nested loop inside */
        for (int k = 0; k < 2; k++) {
            acc += k;
            if (acc < -1000) goto shared_label;
        }
    }
    
shared_label:
    /* Third loop that could be considered overlapping with previous */
    int m = n / 2;
    while (m < n) {
        acc += arr[m] * arr[m - n/2];
        m++;
        
        /* Complex condition with multiple basic blocks */
        if (acc % 2 == 0) {
            acc /= 2;
        } else {
            acc *= 3;
        }
    }
    
    return acc;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + 7) % 1000;  /* Semi-random values */
    }
    
    /* Call functions with different loop structures */
    int result1 = triple_nested_loop(arr, SIZE);
    int result2 = overlapping_loops(arr + 100, SIZE - 100);
    int result3 = switch_in_loop(arr + 200, SIZE - 200);
    int result4 = adjacent_loops(arr + 300, SIZE - 300);
    
    /* Combine results to prevent elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}

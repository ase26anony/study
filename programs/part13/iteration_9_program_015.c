#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i++) {
        /* Middle loop - subset of outer loop */
        for (int j = 0; j < i; j++) {
            /* Innermost loop - subset of middle loop */
            for (int k = 0; k < j; k++) {
                sum += arr[(i * j + k) % SIZE];
                if (sum > LIMIT) {
                    /* Multiple exit point */
                    break;
                }
            }
            /* Loop-invariant calculation */
            int stride = j * 2;
            sum += arr[stride % SIZE];
        }
        /* Another inner loop at same nesting level */
        int count = 0;
        while (count < 5) {
            sum -= arr[i + count];
            count++;
            if (sum < -LIMIT) break;
        }
    }
    return sum;
}

/* Function B: Loops with overlapping basic blocks via goto */
static int function_b(int *arr, int start, int end) {
    int result = 0;
    int i = start;
    
    /* First loop with shared basic block */
loop1:
    for (; i < end / 2; i++) {
        result += arr[i] * 2;
        if (result > LIMIT * 2) {
            goto shared_block;  /* Jump to shared block */
        }
    }
    
    i = end / 2;
    
    /* Second loop that also uses the shared block */
    while (i < end) {
        result -= arr[i];
        i++;
        if (i % 7 == 0) {
            goto shared_block;  /* Same shared block */
        }
    }
    
    goto done;

/* Shared basic block used by both loops */
shared_block:
    result = result / 2;
    arr[i % SIZE] = result;
    if (i < end) {
        i++;
        goto loop1;  /* Creates irreducible flow */
    }

done:
    return result;
}

/* Function C: Loop with switch and break to outer scope */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break that exits the loop */
        switch (state) {
            case 0:
                if (total > LIMIT) {
                    state = 1;
                }
                break;
            case 1:
                if (total < -LIMIT) {
                    state = 2;
                }
                break;
            case 2:
                /* This break exits the switch, not the loop */
                break;
            case 3:
                /* Complex control flow: goto label outside loop */
                goto exit_loop;
        }
        
        /* Nested loop with early exit */
        for (int j = 0; j < 3; j++) {
            total += arr[(i + j) % SIZE];
            if (total > LIMIT * 3) {
                goto exit_loop;  /* Exits outer loop */
            }
        }
        
        /* Loop-invariant code */
        int multiplier = 3;
        total += arr[i] * multiplier;
    }
    
    return total;

exit_loop:
    /* Label outside the main loop */
    return total * 2;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    int i;
    
    /* First loop */
    for (i = 0; i < n; i += 2) {
        acc1 += arr[i];
        if (acc1 > LIMIT) {
            /* Jump to code shared with second loop */
            goto middle_section;
        }
    }
    
    i = 1;
    
    /* Second loop - partially overlaps with first */
    while (i < n) {
        acc2 -= arr[i];
        i += 2;
        if (acc2 < -LIMIT) {
            goto middle_section;
        }
    }
    
    goto finish;

/* Shared middle section - creates overlapping but not subset relationship */
middle_section:
    {
        int temp = acc1 + acc2;
        for (int k = 0; k < 3; k++) {
            temp += arr[k];
        }
        acc1 = temp;
        acc2 = temp / 2;
    }

finish:
    return acc1 + acc2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i - SIZE/2;  /* Mix of positive and negative */
    }
    
    /* Volatile to prevent optimization */
    volatile int total = 0;
    
    /* Call all functions to create various loop structures */
    total += function_a(arr, 50);
    total += function_b(arr, 0, 100);
    total += function_c(arr, 75);
    total += function_d(arr, 60);
    
    /* Additional complex loop in main */
    int outer_sum = 0;
    for (int x = 0; x < 20; x++) {
        /* Inner loop that's a proper subset */
        for (int y = 0; y < x; y++) {
            outer_sum += arr[x * y % SIZE];
            
            /* Switch inside inner loop */
            switch (x % 4) {
                case 0:
                    outer_sum += 1;
                    break;
                case 1:
                    if (outer_sum > 500) {
                        /* Early exit from inner loop */
                        goto next_outer;
                    }
                    break;
                case 2:
                    outer_sum -= 2;
                    break;
                case 3:
                    /* Continue to next outer iteration */
                    goto next_outer;
            }
        }
    next_outer:
        /* Label for goto targets */
        continue;
    }
    
    total += outer_sum;
    
    /* Use result to prevent elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}

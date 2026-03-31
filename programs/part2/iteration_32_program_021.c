/* hw-doloop-test.c - Test program for hardware loop bitmap intersection analysis */
#include <stdio.h>

volatile int global_counter = 0;

/* Function to prevent loop elimination and create distinct basic blocks */
__attribute__((noinline)) 
int use_value(int val) {
    volatile int sink = val;
    return sink;
}

/* 1. DISJOINT LOOPS - Two sequential loops with no block intersection */
__attribute__((noinline))
void disjoint_loops(void) {
    volatile int limit1 = 100;
    volatile int limit2 = 50;
    int sum = 0;
    
    /* First loop - completely separate blocks */
    for (int i = 0; i < limit1; i++) {
        sum += i * 2;
        /* Unique block to this loop */
        if (i & 1) {
            sum -= 1;
        }
    }
    
    /* Second loop - different iteration variable, different blocks */
    for (int j = 0; j < limit2; j++) {
        sum += j * 3;
        /* Different condition pattern */
        if (j % 3 == 0) {
            sum += 5;
        }
    }
    
    global_counter += use_value(sum);
}

/* 2. PERFECTLY NESTED LOOPS - Inner loop blocks are subset of outer loop */
__attribute__((noinline))
void perfectly_nested(void) {
    volatile int outer_limit = 20;
    volatile int inner_limit = 10;
    int product = 1;
    
    for (int i = 0; i < outer_limit; i++) {
        /* Outer loop preamble */
        product = (product * 2) % 1000;
        
        /* Inner loop - all blocks are within outer loop */
        for (int j = 0; j < inner_limit; j++) {
            product = (product + j) % 1000;
            /* Shared computation */
            if (j & 1) {
                product = (product * 3) % 1000;
            }
        }
        
        /* Outer loop postamble */
        product = (product - i) % 1000;
    }
    
    global_counter += use_value(product);
}

/* 3. PARTIALLY OVERLAPPING LOOPS - Share some blocks but not all */
__attribute__((noinline))
void partially_overlapping(void) {
    volatile int limit = 15;
    int acc1 = 0, acc2 = 0;
    
    /* Loop A - has unique blocks in else branch */
    for (int i = 0; i < limit; i++) {
        /* Shared preamble with Loop B */
        int temp = i * i;
        
        if (i & 1) {
            /* SHARED BLOCK - both loops execute this */
            acc1 += temp;
            /* Additional shared operation */
            temp = temp >> 1;
        } else {
            /* UNIQUE TO LOOP A */
            acc1 -= temp * 2;
            /* More unique operations */
            for (int k = 0; k < 2; k++) {
                acc1 += k;
            }
        }
        
        /* Shared postamble */
        acc1 = acc1 % 100;
    }
    
    /* Loop B - sequential but shares the if(i & 1) block structure */
    for (int j = 0; j < limit; j++) {
        /* Shared preamble (same as Loop A) */
        int temp = j * j;
        
        if (j & 1) {
            /* SHARED BLOCK - identical to Loop A's if block */
            acc2 += temp;
            /* Same shared operation */
            temp = temp >> 1;
        } else {
            /* UNIQUE TO LOOP B - different from Loop A's else */
            acc2 += temp / 2;
            /* Different unique operations */
            if (j % 3 == 0) {
                acc2 += 7;
            }
        }
        
        /* Shared postamble */
        acc2 = acc2 % 100;
    }
    
    global_counter += use_value(acc1 + acc2);
}

/* 4. COMPLEX NESTING WITH BREAK/CONTINUE - Creates more control flow variants */
__attribute__((noinline))
void complex_nesting(void) {
    volatile int limit = 25;
    int result = 0;
    
    /* Outer loop with early exit */
    for (int a = 0; a < limit; a++) {
        if (a > 20) {
            /* Unique break block */
            break;
        }
        
        /* Middle loop with continue */
        for (int b = 0; b < 5; b++) {
            if (b == 3) {
                /* Unique continue block */
                result += 100;
                continue;
            }
            
            /* Innermost loop - perfectly nested within middle */
            for (int c = 0; c < 3; c++) {
                result += a + b + c;
                /* Shared computation with other loops */
                if ((a + b + c) & 1) {
                    result *= 2;
                }
            }
        }
        
        /* Outer loop unique tail */
        result = result % 1000;
    }
    
    global_counter += use_value(result);
}

/* 5. MIXED LOOP TYPES IN SAME FUNCTION - Tests multiple relationships */
__attribute__((noinline))
void mixed_loops(void) {
    volatile int n = 10;
    int total = 0;
    
    /* First: Simple counted loop */
    for (int i = 0; i < n; i++) {
        total += i;
    }
    
    /* Second: While loop (different structure) */
    int j = 0;
    while (j < n) {
        total -= j;
        j++;
        if (j == n/2) {
            /* Unique block to this loop */
            total += 50;
        }
    }
    
    /* Third: Do-while loop */
    int k = 0;
    do {
        total *= 2;
        total = total % 100;
        k++;
    } while (k < n);
    
    global_counter += use_value(total);
}

int main(void) {
    /* Initialize to prevent constant propagation */
    volatile int seed = 1;
    global_counter = seed;
    
    /* Execute all loop patterns to trigger bitmap analysis */
    disjoint_loops();
    perfectly_nested();
    partially_overlapping();
    complex_nesting();
    mixed_loops();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", global_counter);
    
    return global_counter != 0 ? 0 : 1;
}

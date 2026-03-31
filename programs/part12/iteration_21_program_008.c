/* test_sel_sched_coverage.c
 * 
 * This program is designed to trigger GCC's selective scheduler
 * debugging output to cover the sel_print_insn function in sel-sched-dump.cc
 * Specifically targeting lines that switch dump output to stderr,
 * dump RTL instructions, and restore the original dump.
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define THRESHOLD 500

/* Function with tight, data-dependent loop that benefits from pipelining */
int compute_sum(int* a, int* b, int n) {
    int sum = 0;
    
    /* Main loop with data dependencies - forces scheduler to work hard */
    for (int i = 0; i < n; i++) {
        /* Data-dependent computation with carried dependency */
        int temp = a[i] * b[i];
        
        /* Conditional control flow - creates multiple basic blocks */
        if (temp > THRESHOLD) {
            sum += temp;
            
            /* Inline assembly with multiple operands - creates complex RTL */
            asm volatile (
                "addl %1, %0\n\t"
                "cmpl %2, %0\n\t"
                "setg %%al\n\t"
                "movzbl %%al, %0"
                : "+r"(sum)
                : "r"(temp), "r"(THRESHOLD * 2)
                : "cc", "al"
            );
        } else {
            sum += temp / 2;
            
            /* Another inline assembly with clobbers */
            asm volatile (
                "sarl $1, %0\n\t"
                "addl %1, %0"
                : "+r"(sum)
                : "r"(temp)
                : "cc"
            );
        }
        
        /* Additional computation to increase scheduling complexity */
        if (i % 8 == 0) {
            /* Nested loop with small iteration count */
            for (int j = 0; j < 4; j++) {
                sum += j * a[i];
            }
        }
    }
    
    return sum;
}

/* Another function with different loop structure */
int compute_product(int* a, int* b, int n) {
    int prod = 1;
    
    /* Loop with pointer arithmetic */
    int* pa = a;
    int* pb = b;
    int* end = a + n;
    
    while (pa < end) {
        int val = (*pa) * (*pb);
        
        /* Complex conditional with multiple branches */
        if (val > 0) {
            prod *= val;
            
            /* Inline assembly with memory operand */
            asm volatile (
                "imull %1, %0\n\t"
                "testl %0, %0\n\t"
                "jns 1f\n\t"
                "negl %0\n\t"
                "1:"
                : "+r"(prod)
                : "r"(val)
                : "cc"
            );
        } else if (val < 0) {
            prod *= -val;
        }
        
        pa++;
        pb++;
        
        /* Small unrolled section */
        if ((pa - a) % 16 == 0) {
            asm volatile ("nop" ::: "memory");
        }
    }
    
    return prod;
}

/* Function with switch statement for control flow variety */
int process_with_switch(int* arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        switch (val % 5) {
            case 0:
                result += val;
                asm volatile ("addl %1, %0" : "+r"(result) : "r"(val) : "cc");
                break;
            case 1:
                result -= val;
                break;
            case 2:
                result *= val;
                asm volatile ("imull %1, %0" : "+r"(result) : "r"(val) : "cc");
                break;
            case 3:
                result ^= val;
                break;
            case 4:
                result |= val;
                asm volatile ("orl %1, %0" : "+r"(result) : "r"(val) : "cc");
                break;
        }
    }
    
    return result;
}

int main() {
    /* Initialize arrays with predictable but non-constant values */
    int a[SIZE];
    int b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = (i * 3) % 1000;
        b[i] = (i * 7) % 1000;
    }
    
    /* Use __builtin_assume to provide optimization hints */
    if (SIZE > 0) {
        __builtin_assume(SIZE > 0);
    }
    
    /* Call functions to create scheduling regions */
    int sum1 = compute_sum(a, b, SIZE);
    int sum2 = compute_product(a, b, SIZE);
    int sum3 = process_with_switch(a, SIZE);
    
    /* Final computation to prevent dead code elimination */
    int final_result = sum1 + sum2 + sum3;
    
    /* Print result to ensure execution */
    printf("Result: %d\n", final_result);
    
    /* Additional volatile operations to prevent optimization */
    volatile int check = final_result;
    asm volatile ("" : : "r"(check) : "memory");
    
    return 0;
}

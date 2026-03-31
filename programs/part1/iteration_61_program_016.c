#include <stdio.h>
#include <stdlib.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Global pointer for side effects */
volatile int *global_ptr;

/* Function with parameter counter */
int loop_with_param(int counter) {
    int sum = 0;
    do {
        sum += counter;
        bar();
    } while (--counter > 0);
    return sum;
}

int main(void) {
    int total = 0;
    volatile int *ptr = &total;
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 100;
        do {
            total += counter;
            *ptr = total;  /* Simple side effect */
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            total += u_counter;
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register-qualified short */
    {
        register short s_counter = 25;
        do {
            total += s_counter;
            asm volatile("" : : "r"(s_counter)); /* Prevent optimization */
        } while (--s_counter > 0);
    }
    
    /* Pattern 4: char type in if statement context */
    {
        if (total > 0) {
            char c_counter = 10;
            do {
                total += c_counter;
                global_ptr = &total;
            } while (--c_counter > 0);
        }
    }
    
    /* Pattern 5: Counter starts at 1 (executes once) */
    {
        int once_counter = 1;
        do {
            total += 1000;
            bar();
        } while (--once_counter > 0);
    }
    
    /* Pattern 6: Function parameter counter */
    total += loop_with_param(20);
    
    /* Pattern 7: Embedded in complex control flow */
    {
        int counter = 15;
        int temp = 0;
        do {
            temp += counter;
            bar();
        } while (--counter > 0);
        total += temp;
        
        /* Follow-up statements affecting register allocation */
        int x = total * 2;
        total = x / 2;
    }
    
    /* Pattern 8: volatile counter (should NOT match pattern) */
    {
        volatile int v_counter = 5;
        do {
            total += 1;
        } while (--v_counter > 0);
    }
    
    /* NEGATIVE TEST: Post-increment (should NOT match) */
    {
        int counter = 5;
        do {
            total += 2;
        } while (counter++ < 10);
    }
    
    /* NEGATIVE TEST: Compare against non-zero (should NOT match) */
    {
        int counter = 10;
        do {
            total += 3;
        } while (--counter > 5);
    }
    
    /* Pattern 9: Compound assignment form */
    {
        int counter = 8;
        do {
            total += counter;
            bar();
        } while ((counter -= 1) != 0);
    }
    
    /* Pattern 10: Different integer type with pointer arithmetic */
    {
        unsigned char uc_counter = 12;
        int array[12] = {0};
        int *arr_ptr = array;
        do {
            *arr_ptr++ = uc_counter;
            total += uc_counter;
        } while (--uc_counter != 0);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    static int count = 0;
    count++;
}

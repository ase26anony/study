#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

int main(void) {
    int total = 0;
    volatile int* ptr = &total;  /* Use volatile pointer for side effects */
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 1;  /* Use volatile pointer access */
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short type with register qualifier */
    {
        register short s_counter = 25;
        do {
            total += 2;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 4: Char type with explicit decrement */
    {
        char c_counter = 10;
        do {
            total += 3;
        } while ((c_counter -= 1) != 0);
    }
    
    /* Pattern 5: Inside conditional context */
    if (total > 0) {
        int counter = 5;
        do {
            bar();
            total += 4;
        } while (--counter > 0);
    }
    
    /* Pattern 6: Counter as function parameter simulation */
    {
        int param_counter = 8;
        /* Simulate parameter passing by using in nested scope */
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 7: Boundary case - counter starts at 1 */
    {
        int counter = 1;
        do {
            total += 6;
        } while (--counter > 0);
    }
    
    /* Pattern 8: Should NOT match - post-increment */
    {
        int counter = 3;
        do {
            total += 7;
        } while (counter++ < 3);  /* Different pattern, should fail cmp_arg2 != const0_rtx */
    }
    
    /* Pattern 9: Should NOT match - compare against non-zero */
    {
        int counter = 4;
        do {
            total += 8;
        } while (--counter > 2);  /* Should fail cmp_arg2 != const0_rtx */
    }
    
    /* Pattern 10: Complex body with multiple statements */
    {
        int counter = 6;
        int local_sum = 0;
        do {
            local_sum += counter;
            total += local_sum;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 11: With volatile counter (likely won't match but tests edge) */
    {
        volatile int v_counter = 3;
        do {
            total += 9;
        } while (--v_counter > 0);
    }
    
    /* Pattern 12: Nested scope with different type */
    {
        {
            unsigned char uc_counter = 7;
            do {
                total += 10;
                bar();
            } while (--uc_counter != 0);
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Empty but non-const/non-pure */
}

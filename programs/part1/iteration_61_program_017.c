#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function with parameter counter */
void loop_with_param(int counter) {
    int sum = 0;
    do {
        sum += counter;
        bar();
    } while (--counter > 0);
    printf("Param loop sum: %d\n", sum);
}

int main() {
    int total = 0;
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 100;
        int local_sum = 0;
        do {
            local_sum += counter;
            bar();
        } while (--counter > 0);
        total += local_sum;
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        unsigned int u_sum = 0;
        do {
            u_sum += u_counter;
            bar();
        } while (--u_counter != 0);
        total += u_sum;
    }
    
    /* Pattern 3: Short type with register qualifier */
    {
        register short s_counter = 30;
        short s_sum = 0;
        do {
            s_sum += s_counter;
            bar();
        } while (--s_counter > 0);
        total += s_sum;
    }
    
    /* Pattern 4: Char type with explicit decrement */
    {
        char c_counter = 20;
        char c_sum = 0;
        do {
            c_sum += c_counter;
            bar();
        } while ((c_counter -= 1) != 0);
        total += c_sum;
    }
    
    /* Pattern 5: Counter starting at 1 (edge case) */
    {
        int counter = 1;
        int edge_sum = 0;
        do {
            edge_sum += counter;
            bar();
        } while (--counter > 0);
        total += edge_sum;
    }
    
    /* Pattern 6: Loop inside conditional */
    {
        int flag = 1;
        if (flag) {
            int counter = 40;
            int cond_sum = 0;
            do {
                cond_sum += counter;
                bar();
            } while (--counter > 0);
            total += cond_sum;
        }
    }
    
    /* Pattern 7: Loop with pointer in body */
    {
        int counter = 25;
        int ptr_sum = 0;
        int *ptr = &ptr_sum;
        do {
            *ptr += counter;
            bar();
        } while (--counter > 0);
        total += ptr_sum;
    }
    
    /* Pattern 8: Loop followed by other statements */
    {
        int counter = 35;
        int follow_sum = 0;
        do {
            follow_sum += counter;
            bar();
        } while (--counter > 0);
        /* Additional statement affecting register allocation */
        follow_sum *= 2;
        total += follow_sum;
    }
    
    /* Pattern 9: Volatile counter (should NOT match pattern) */
    {
        volatile int v_counter = 10;
        int vol_sum = 0;
        do {
            vol_sum += v_counter;
            bar();
        } while (--v_counter > 0);
        total += vol_sum;
    }
    
    /* Pattern 10: Post-increment (should NOT match pattern) */
    {
        int counter = 15;
        int post_sum = 0;
        do {
            post_sum += counter;
            bar();
        } while (counter++ < 14);  /* Different pattern - not decrement */
        total += post_sum;
    }
    
    /* Pattern 11: Compare against non-zero (should NOT match pattern) */
    {
        int counter = 60;
        int non_zero_sum = 0;
        do {
            non_zero_sum += counter;
            bar();
        } while (--counter > 5);  /* Compare against 5, not 0 */
        total += non_zero_sum;
    }
    
    /* Call function with parameter */
    loop_with_param(10);
    
    printf("Total result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    static int call_count = 0;
    call_count++;
}

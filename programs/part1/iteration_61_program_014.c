#include <stdio.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Function to accumulate results */
int accumulate(int value) {
    static int total = 0;
    total += value;
    return total;
}

/* Test function with various do-while patterns */
void test_loops(int param_counter) {
    int result = 0;
    
    /* Pattern 1: Basic signed int decrement (should match) */
    {
        int counter = 100;
        int sum = 0;
        do {
            sum += counter;
            bar();  /* External call prevents optimization */
        } while (--counter > 0);
        result += sum;
        printf("Pattern 1: %d\n", sum);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison (should match) */
    {
        unsigned int u_counter = 50;
        int prod = 1;
        do {
            prod *= 2;
            bar();
        } while (--u_counter != 0);
        result += prod;
        printf("Pattern 2: %d\n", prod);
    }
    
    /* Pattern 3: register-qualified short counter (should match) */
    {
        register short reg_counter = 25;
        int temp = 0;
        do {
            temp += reg_counter;
            bar();
        } while (--reg_counter > 0);
        result += temp;
        printf("Pattern 3: %d\n", temp);
    }
    
    /* Pattern 4: char counter with explicit decrement (should match) */
    {
        char char_counter = 10;
        int acc = 0;
        do {
            acc += char_counter;
            bar();
        } while ((char_counter -= 1) != 0);
        result += acc;
        printf("Pattern 4: %d\n", acc);
    }
    
    /* Pattern 5: Counter as function parameter (should match) */
    {
        int local_param = param_counter;
        if (local_param > 0) {
            int val = 0;
            do {
                val += local_param;
                bar();
            } while (--local_param > 0);
            result += val;
            printf("Pattern 5: %d\n", val);
        }
    }
    
    /* Pattern 6: Counter starting at 1 (edge case, should match) */
    {
        int single_counter = 1;
        int once = 0;
        do {
            once = 42;
            bar();
        } while (--single_counter > 0);
        result += once;
        printf("Pattern 6: %d\n", once);
    }
    
    /* Pattern 7: Nested in if statement (should match) */
    {
        int flag = 1;
        if (flag) {
            int if_counter = 15;
            int if_sum = 0;
            do {
                if_sum += if_counter;
                bar();
            } while (--if_counter > 0);
            result += if_sum;
            printf("Pattern 7: %d\n", if_sum);
        }
    }
    
    /* Pattern 8: Post-increment (should NOT match GEN_INT(-1) check) */
    {
        int post_counter = 10;
        int post_val = 0;
        do {
            post_val += post_counter;
            bar();
        } while (post_counter-- > 1);  /* Different pattern */
        result += post_val;
        printf("Pattern 8: %d\n", post_val);
    }
    
    /* Pattern 9: Compare against non-zero (should NOT match const0_rtx check) */
    {
        int non_zero_counter = 20;
        int non_zero_sum = 0;
        do {
            non_zero_sum += non_zero_counter;
            bar();
        } while (--non_zero_counter > 5);  /* Not comparing against 0 */
        result += non_zero_sum;
        printf("Pattern 9: %d\n", non_zero_sum);
    }
    
    /* Pattern 10: volatile counter (likely won't match but tests edge) */
    {
        volatile int vol_counter = 5;
        int vol_sum = 0;
        do {
            vol_sum += vol_counter;
            bar();
        } while (--vol_counter > 0);
        result += vol_sum;
        printf("Pattern 10: %d\n", vol_sum);
    }
    
    /* Pattern 11: Pointer-based loop with decrement */
    {
        int arr[10];
        int *ptr = &arr[9];
        int ptr_sum = 0;
        do {
            *ptr = 0;  /* Simple side effect */
            ptr_sum += (ptr - arr);
            bar();
        } while (ptr-- > arr);
        result += ptr_sum;
        printf("Pattern 11: %d\n", ptr_sum);
    }
    
    /* Final accumulation */
    accumulate(result);
}

/* Dummy implementation of bar() for linking */
void bar(void) {
    /* Empty but external linkage prevents optimization */
    static int dummy = 0;
    dummy++;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different parameters */
    test_loops(8);
    test_loops(3);
    test_loops(12);
    
    /* Simple verification loop (should also match pattern) */
    {
        int verify_counter = 7;
        int verify_sum = 0;
        do {
            verify_sum += verify_counter;
            bar();
        } while (--verify_counter > 0);
        total += verify_sum;
    }
    
    printf("Final total: %d\n", total);
    return 0;
}

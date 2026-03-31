#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int global_modifier = 0;
volatile int global_counter = 0;

/* Function with side effects to use in conditions */
int __attribute__((noinline)) side_effect_func(void) {
    return global_modifier++;
}

/* Test function 1: Simple modification of condition variable in then block */
void __attribute__((noinline)) test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 5;
    volatile int c = 0;
    
    /* Loop to create interesting block structure */
    for (int i = 0; i < 100; i++) {
        /* Condition where 'a' is tested */
        if (a > b) {
            /* CRITICAL: Modify 'a' which is part of the condition */
            a = b + i;  /* This should trigger modified_in_p check */
            c += a * b;
        } else {
            b = a + i;
        }
        
        /* Add some complexity to prevent dead code elimination */
        global_counter += (a & 1);
    }
    
    /* Use results to prevent optimization */
    printf("Test1: a=%d, b=%d, c=%d\n", a, b, c);
}

/* Test function 2: Compound condition with multiple modifications */
void __attribute__((noinline)) test_compound_condition(void) {
    volatile int x = 100;
    volatile int y = 50;
    volatile int z = 75;
    volatile int result = 0;
    
    for (int i = 0; i < 50; i++) {
        /* Complex condition with multiple variables */
        if (x > y && z < (x + y) && global_modifier < 100) {
            /* Modify 'x' which is used in the compound condition */
            x = y + z;  /* First modification */
            
            /* Also modify 'z' which is also in the condition */
            z = x - i;  /* Second modification */
            
            /* Add another instruction to ensure header has multiple insns */
            result += x * z;
            
            /* Call function that might affect global_modifier */
            global_modifier += side_effect_func() & 1;
        } else {
            y = x + z;
        }
        
        /* Vary the condition variables */
        if (i % 3 == 0) {
            x += 1;
        }
    }
    
    printf("Test2: x=%d, y=%d, z=%d, result=%d\n", x, y, z, result);
}

/* Test function 3: Condition with function call and modification */
void __attribute__((noinline)) test_func_call_condition(void) {
    static volatile int counter = 0;
    volatile int limit = 30;
    volatile int accumulator = 0;
    
    /* Use argc/argv or stdin to introduce runtime variability */
    for (int i = 0; i < 40; i++) {
        /* Condition using a variable that will be modified in then block */
        if (counter < limit && side_effect_func() > 0) {
            /* Modify 'counter' which is used in the condition */
            counter++;  /* This should trigger modified_in_p */
            
            /* Add more instructions to the header */
            accumulator += counter * 2;
            global_modifier = (global_modifier + 1) % 100;
            
            /* Another modification to a condition-related variable */
            if (i % 2 == 0) {
                limit += 1;  /* Also modifies 'limit' from condition */
            }
        } else {
            counter = counter > 0 ? counter - 1 : 0;
        }
        
        /* Loop with nested condition to create complex CFG */
        for (int j = 0; j < 2; j++) {
            if (accumulator > 100) {
                accumulator -= 50;
            }
        }
    }
    
    printf("Test3: counter=%d, limit=%d, accumulator=%d\n", 
           counter, limit, accumulator);
}

/* Test function 4: Multiple modifications in then block header */
void __attribute__((noinline)) test_multiple_mods_in_header(void) {
    volatile int p = 42;
    volatile int q = 17;
    volatile int r = 99;
    
    /* Unrolled loop to create larger basic blocks */
    for (int i = 0; i < 20; i += 2) {
        /* Condition with multiple variables */
        if (p != q && r > (p + q)) {
            /* Sequence of modifications - all in header */
            p = q + i;      /* Modifies 'p' from condition */
            q = r - p;      /* Modifies 'q' from condition */
            r = p * q;      /* Modifies 'r' from condition */
            
            /* More non-label, non-note instructions */
            global_counter += p;
            global_modifier = (global_modifier ^ q) & 0xFF;
        } else {
            int temp = p;
            p = q;
            q = temp;
        }
        
        /* Alternate path */
        if (i % 4 == 0) {
            r += side_effect_func();
        }
    }
    
    printf("Test4: p=%d, q=%d, r=%d\n", p, q, r);
}

/* Test function 5: Pointer aliasing could affect condition */
void __attribute__((noinline)) test_pointer_aliasing(void) {
    volatile int data[4] = {10, 20, 30, 40};
    volatile int *ptr1 = &data[0];
    volatile int *ptr2 = &data[1];
    volatile int index = 0;
    
    for (int i = 0; i < 25; i++) {
        /* Condition using array elements */
        if (data[0] > data[1] && *ptr1 < *ptr2) {
            /* Modify array element used in condition */
            data[0] = data[1] + i;  /* Modifies data[0] */
            
            /* Also modify through pointer */
            *ptr2 = data[0] - 5;    /* Modifies data[1] */
            
            /* Change index which might affect pointer deref */
            index = (index + 1) & 3;
            ptr1 = &data[index];
        } else {
            data[1] = data[0] + i;
        }
        
        /* Rotate pointers */
        if (i % 5 == 0) {
            ptr2 = &data[(i / 5) & 3];
        }
    }
    
    printf("Test5: data[0]=%d, data[1]=%d, *ptr1=%d, *ptr2=%d\n",
           data[0], data[1], *ptr1, *ptr2);
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to introduce runtime variability */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with some randomness */
    global_modifier = seed;
    global_counter = seed * 2;
    
    printf("Starting tests with seed=%d\n", seed);
    
    /* Run all test functions */
    test_simple_modification();
    test_compound_condition();
    test_func_call_condition();
    test_multiple_mods_in_header();
    test_pointer_aliasing();
    
    /* Final computation to ensure all code has observable effects */
    int final_result = global_counter + global_modifier;
    printf("Final: global_counter=%d, global_modifier=%d, total=%d\n",
           global_counter, global_modifier, final_result);
    
    return final_result != 0 ? 0 : 1;
}

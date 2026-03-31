/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex conditional logic */
int process_data(volatile int input) {
    int result = 0;
    
    if (input < 0) {
        result = -1;
    } else if (input == 0) {
        result = 0;
    } else if (input < 100) {
        result = 1;
        for (volatile int i = 0; i < input; ++i) {
            result += i % 3;
        }
    } else {
        result = 100;
        volatile int counter = input % 10;
        while (counter-- > 0) {
            result -= 2;
        }
    }
    
    return result;
}

/* Function 2: Nested loops */
int matrix_operation(volatile int size) {
    int sum = 0;
    
    for (volatile int i = 0; i < size; ++i) {
        for (volatile int j = 0; j < size; ++j) {
            sum += (i * j) % 7;
        }
    }
    
    /* Switch statement for additional complexity */
    switch (sum % 4) {
        case 0:
            sum += 10;
            break;
        case 1:
            sum += 20;
            break;
        case 2:
            sum += 30;
            break;
        default:
            sum += 40;
            break;
    }
    
    return sum;
}

/* Function 3: Recursive-like pattern with goto */
int complex_branching(volatile int x, volatile int y) {
    int value = 0;
    
    if (x > y) {
        value = x - y;
        if (value > 10) {
            goto large_difference;
        }
    } else if (x < y) {
        value = y - x;
        if (value > 5) {
            goto small_difference;
        }
    } else {
        value = 0;
        goto equal_case;
    }
    
    return value * 2;
    
large_difference:
    return value * 3;
small_difference:
    return value * 4;
equal_case:
    return -1;
}

/* Function 4: Mixed control flow */
void process_array(volatile int* arr, volatile int len) {
    volatile int i = 0;
    volatile int total = 0;
    
    do {
        if (arr[i] % 2 == 0) {
            total += arr[i] * 2;
        } else {
            total += arr[i];
        }
        
        /* Early exit condition */
        if (total > 1000) {
            break;
        }
        
        i++;
    } while (i < len);
    
    /* Nested switch */
    switch (len % 3) {
        case 0:
            total += 100;
            break;
        case 1: {
            volatile int temp = total;
            for (volatile int j = 0; j < 3; j++) {
                temp /= 2;
            }
            total = temp;
            break;
        }
        case 2:
            total *= 2;
            break;
    }
}

/* Main function with different execution paths */
int main(int argc, char* argv[]) {
    volatile int mode = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Different execution paths based on mode */
    switch (mode % 4) {
        case 0:
            printf("Result1: %d\n", process_data(50));
            printf("Result2: %d\n", matrix_operation(3));
            break;
        case 1:
            printf("Result3: %d\n", complex_branching(20, 5));
            {
                volatile int arr[5] = {1, 2, 3, 4, 5};
                process_array(arr, 5);
            }
            break;
        case 2:
            printf("Result1: %d\n", process_data(-5));
            printf("Result3: %d\n", complex_branching(5, 20));
            break;
        case 3:
            printf("Result2: %d\n", matrix_operation(4));
            {
                volatile int arr[3] = {10, 20, 30};
                process_array(arr, 3);
            }
            printf("Result1: %d\n", process_data(150));
            break;
    }
    
    /* Always execute some common code */
    volatile int common = process_data(25) + matrix_operation(2);
    
    return common % 100;
}

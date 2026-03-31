/* test_overlap.c - Complex program for gcov-tool overlap testing */
#include <stdio.h>
#include <stdlib.h>

/* Function 1: Complex function with loops and conditionals */
int func1(volatile int x, volatile int y) {
    int result = 0;
    
    /* Loop with volatile counter */
    for (volatile int i = 0; i < x; ++i) {
        result += i;
        
        /* Nested conditional */
        if (i % 2 == 0) {
            result *= 2;
        } else {
            result -= 1;
        }
    }
    
    /* Switch statement */
    switch (y) {
        case 1:
            result += 100;
            break;
        case 2:
            result += 200;
            break;
        case 3:
            result += 300;
            break;
        default:
            result += 50;
    }
    
    return result;
}

/* Function 2: Different complexity with recursion-like pattern */
int func2(volatile int n) {
    int total = 0;
    
    /* While loop */
    volatile int count = n;
    while (count > 0) {
        total += count;
        
        /* Multiple conditionals */
        if (count > 10) {
            total *= 2;
        } else if (count > 5) {
            total += 10;
        } else {
            total -= 1;
        }
        
        count--;
    }
    
    /* Do-while loop */
    volatile int j = 0;
    do {
        total += j * j;
        j++;
    } while (j < 3);
    
    return total;
}

/* Function 3: Matrix-like operations */
int func3(volatile int rows, volatile int cols) {
    int sum = 0;
    
    /* Nested loops */
    for (volatile int r = 0; r < rows; ++r) {
        for (volatile int c = 0; c < cols; ++c) {
            sum += r * c;
            
            /* Early exit condition */
            if (sum > 1000) {
                goto done;
            }
        }
    }
    
done:
    /* Ternary operator */
    return (sum > 500) ? sum / 2 : sum * 2;
}

/* Function 4: String/array operations */
void func4(volatile int size) {
    char buffer[100];
    
    /* Initialize array */
    for (volatile int i = 0; i < size && i < 100; ++i) {
        buffer[i] = (char)(i % 26 + 'A');
    }
    
    /* Process array */
    volatile int pos = 0;
    while (pos < size && pos < 100) {
        if (buffer[pos] == 'A' || buffer[pos] == 'E' || 
            buffer[pos] == 'I' || buffer[pos] == 'O' || 
            buffer[pos] == 'U') {
            buffer[pos] = '*';
        }
        pos++;
    }
}

/* Main function with varied execution paths */
int main(int argc, char *argv[]) {
    volatile int mode = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Different execution paths based on mode */
    switch (mode) {
        case 1:
            printf("Mode 1: %d\n", func1(5, 2));
            printf("Mode 1: %d\n", func2(8));
            break;
        case 2:
            printf("Mode 2: %d\n", func1(3, 1));
            printf("Mode 2: %d\n", func3(4, 5));
            func4(20);
            break;
        case 3:
            printf("Mode 3: %d\n", func2(12));
            printf("Mode 3: %d\n", func3(2, 10));
            func4(15);
            break;
        default:
            printf("Default: %d\n", func1(2, 3));
            printf("Default: %d\n", func2(4));
            printf("Default: %d\n", func3(3, 3));
            func4(10);
    }
    
    return 0;
}

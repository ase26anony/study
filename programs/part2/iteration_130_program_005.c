/* Complex type declarations with nested delimiters to exercise consume_balanced logic */

/* 1. Function pointers with complex signatures */
int (*(*complex_callback)(int (*)(float)))[10];

/* 2. Multi-dimensional arrays with nested initializers */
int matrix[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};

/* 3. Structure with nested anonymous structs, unions, and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
    struct {
        char *(*nested_func)(int, ...);
        double matrix[2][2];
    } inner;
};

/* 4. Type definitions with parentheses for grouping */
typedef int (*array_of_5_funcs[5])(char, ...);
typedef void (*(*signal_handler_t)(int, void (*)(int)))(void);

/* 5. GCC attributes with nested parentheses */
struct __attribute__((aligned(32), packed)) AlignedStruct {
    int data[8];
} __attribute__((deprecated("Use NewStruct instead")));

/* 6. Macro generating delimiter-heavy code */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])
MAKE_COMPLEX_TYPE(5);
MAKE_COMPLEX_TYPE(10);

/* 7. Complex expressions in sizeof */
size_t sizes[] = {
    sizeof(int[10][20]),
    sizeof(struct Outer),
    sizeof(int (*(*)[5])(void))
};

/* 8. Compound literals with nested braces */
int (*get_matrix(void))[4] {
    return (int[][4]){{1,2,3,4}, {5,6,7,8}};
}

/* 9. Function with complex parameter types */
void process_data(
    int (*(*callback)(int, ...))[10],
    char (*(*names)[5])[20],
    __attribute__((format(printf, 2, 3))) void (*logger)(const char *, ...)
);

/* 10. Nested switch-case with braces (extra delimiter nesting) */
int nested_switch_example(int x) {
    switch (x) {
        case 1: {
            int arr[][3] = {{1,2,3}, {4,5,6}};
            return arr[0][0];
        }
        case 2: {
            struct { int a; int b; } s = {.a = 1, .b = 2};
            return s.a + s.b;
        }
        default:
            return 0;
    }
}

/* Minimal main to ensure compilation */
int main(void) {
    /* Use sizeof on complex types */
    size_t s1 = sizeof(int (*(*)[5])(char, float));
    size_t s2 = sizeof(matrix);
    
    /* Declare and use complex types */
    array_of_5_funcs funcs = {0};
    struct Outer o = {0};
    
    /* Use compound literal */
    int (*ptr)[4] = (int[][4]){{0}};
    
    /* Call function with nested switch */
    return nested_switch_example(1);
}

/* Complex type declarations with deeply nested delimiters */
#include <stdarg.h>

/* 1. Function pointers with complex signatures */
int (*(*complex_callback)(int (*)(float)))[10];

/* 2. Multi-dimensional arrays with initializers */
int matrix[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};

/* 3. Structures with nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
};

/* 4. Type definitions with parentheses for grouping */
typedef int (*array_of_5_funcs[5])(char, ...);

/* 5. Nested arrays in function parameters */
void process_matrix(int (*mat)[sizeof(int[10][20])/sizeof(int[20])], 
                    int rows, 
                    int cols);

/* 6. Compound type with attributes */
struct __attribute__((aligned(32), packed)) AlignedStruct {
    int data[8];
    char __attribute__((aligned(16))) aligned_char;
};

/* 7. Complex function pointer with attributes */
int (__attribute__((noreturn)) *signal_handler)(int, 
    __attribute__((nonnull)) void (*)(int));

/* Macro to generate delimiter-heavy code */
#define MAKE_COMPLEX_TYPE(n) \
    int (*(*var##n)[n])(char (*)[n]) __attribute__((deprecated))

/* Instantiate macro-generated types */
MAKE_COMPLEX_TYPE(3);
MAKE_COMPLEX_TYPE(5);

/* Function with format attribute (nested parentheses in attribute) */
void log_message(const char *format, ...) 
    __attribute__((format(printf, 1, 2)));

/* Enum with complex initializer */
enum ComplexEnum {
    VAL1 = sizeof(int[2][3]),
    VAL2 = sizeof(struct { int x; double y; }),
    VAL3 = (int)((void (*)(void))0)
};

/* Global variable with complex type */
array_of_5_funcs global_funcs;

/* Function definitions */
void process_matrix(int (*mat)[10], int rows, int cols) {
    /* Use sizeof with complex array type */
    size_t size = sizeof(int[rows][cols]);
    (void)size;
}

void log_message(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

/* Main function - minimal but uses complex types */
int main(void) {
    /* Declare and use complex types */
    struct Outer outer_instance = {0};
    
    /* Use sizeof on complex array type */
    size_t matrix_size = sizeof(int[3][4][5]);
    
    /* Compound literal */
    int (*arr_ptr)[4] = &(int[3][4]){{0}};
    
    /* Cast expression with nested parentheses */
    int value = (int)((long)((void*)((char*)0 + 1)));
    
    /* Initialize function pointer array */
    for (int i = 0; i < 5; i++) {
        global_funcs[i] = NULL;
    }
    
    /* Use attribute-affected variable */
    var3 = NULL;
    
    return 0;
}

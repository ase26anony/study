/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function pointers with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Global function pointer variable using complex type */
int (*(*global_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3])[(4+1)];

/* 3. Nested structures with arrays and function pointers */
struct Inner {
    int (*calc)(int, int);
    char data[2][3];
};

struct Outer {
    struct Inner inner[2];
    int (*(*nested_fp)(struct Inner))[5];
    union {
        int x;
        double y;
        struct {
            char *p;
            int len;
        } str;
    } variant;
};

/* Global struct with complex initializer */
struct Outer global_struct = {
    .inner = {
        [0] = {
            .calc = NULL,
            .data = {{'a','b','c'},{'d','e','f'}}
        },
        [1] = {
            .calc = NULL,
            .data = {{'g','h','i'},{'j','k','l'}}
        }
    },
    .nested_fp = NULL,
    .variant = {
        .str = {
            .p = "test",
            .len = 4
        }
    }
};

/* 4. Combined constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

struct Config {
    int (*handlers[3])(void);
    int values[2][2];
} config = {
    .handlers = {NULL, NULL, NULL},
    .values = {{1,2},{3,4}}
};

/* 5. Function with complex parameter types */
void process_data(int (*table[])[5], struct { int x; int y; } point) {
    /* Function body - will be defined if needed */
}

/* Simple compatible functions for assignment */
int simple_func(void) { return 42; }
int func_taking_double(double d) { return (int)d; }
int func_returning_array_of_ints(char c) { return c; }

int main(void) {
    int result = 0;
    
    /* 1. Use complex function pointer type */
    fn_complex local_fp = NULL;
    
    /* 2. Access nested array elements from global struct */
    result += global_struct.inner[0].data[1][2];  /* 'f' = 102 */
    result += global_struct.inner[1].data[0][1];  /* 'h' = 104 */
    
    /* 3. Use array of pointers */
    result += ((int)ptr_to_array[0][0] + ptr_to_array[1][1]);  /* 1 + 4 = 5 */
    
    /* 4. Access config values */
    result += config.values[0][0] + config.values[1][1];  /* 1 + 4 = 5 */
    
    /* 5. Create local variable with complex type */
    int (*(*local_var)(int (*)(double)))(char) = NULL;
    
    /* 6. Complex array access with parenthesized expression */
    int idx = (1 + 1);
    result += matrix[idx][idx];  /* 0, but ensures syntax is parsed */
    
    /* 7. Nested structure with designated initializer */
    struct Outer local_outer = {
        .inner = {
            [0] = {
                .calc = simple_func,
                .data = {{1,2,3},{4,5,6}}
            }
        },
        .variant = { .x = 100 }
    };
    
    result += local_outer.variant.x;  /* 100 */
    
    /* Print final result (102 + 104 + 5 + 5 + 100 = 316) */
    printf("Result: %d\n", result);
    
    return 0;
}

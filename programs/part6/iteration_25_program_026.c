/* Complex type declarations to exercise gengtype parser's consume_balanced function */

/* 1. Complex function pointers with nested parentheses */
int simple_func(int x) { return x * 2; }

/* Pointer to function taking function pointer, returning pointer to function */
int (*(*complex_func_ptr)(int (*)(double)))(char);

/* Typedef chain building complex types */
typedef int (*fn1)(void);
typedef fn1 (*fn2)(int);
typedef fn2 (*fn3)(char*);
fn3 global_fn_chain;

/* 2. Multi-dimensional and complex array declarations */
/* Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* Array of pointers to arrays with parenthesized size expression */
char (*strings[(2+3)])[20];

/* 3D array with complex element type */
struct Point { int x, y, z; };
struct Point multi_array[2][3][4];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[3];
};

struct Middle {
    double d;
    struct Inner inner;
    int (*func_ptr)(int);
};

struct Nested {
    int a[2][3];
    struct Middle middle;
    union {
        long l;
        float f;
    } u;
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { 
        3.14, 
        { 
            "test", 
            {7, 8, 9} 
        }, 
        &simple_func 
    }, 
    { .l = 100L } 
};

/* Array with designated initializers and nested braces */
int designated_arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combined constructs in single declarations */
/* Variable with initializer using compound literal containing array */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4}, {5, 6} };

/* Function prototype with complex parameter */
void process_data(int (*table[])[5], struct { int x; char y[10]; } param);

/* Union containing array of function pointers */
union ComplexUnion {
    int (*(*func_array[3])(void))(int);
    struct {
        char *name;
        int (*handler)(char*);
    } meta;
};

/* 5. Additional global declarations */
/* Global variable with complex type */
int (*(*global_var)(void))[3];

/* Global struct with initializer containing function pointer */
struct Data { 
    int (*func)(int); 
    int matrix[2][2];
} global_data = { 
    &simple_func, 
    {{1, 2}, {3, 4}} 
};

/* Typedef for extremely complex type */
typedef int (*(*(*nightmare_type)[5])(int (*(*)(double))[3]))(char);

/* 6. Function using the complex types */
int helper_func(int x) { return x + 1; }
double helper_double(double x) { return x * 2.0; }

/* Main function to use the complex types and prevent dead code elimination */
int main(void) {
    /* Use global_nested */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
        }
    }
    
    /* Use designated_arr */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += designated_arr[i][j];
        }
    }
    
    /* Use ptr_to_array */
    sum += ptr_to_array[0][0] + ptr_to_array[1][1];
    
    /* Use global_data */
    if (global_data.func) {
        sum += global_data.func(10);
    }
    sum += global_data.matrix[0][0] + global_data.matrix[1][1];
    
    /* Use global_nested's function pointer */
    if (global_nested.middle.func_ptr) {
        sum += global_nested.middle.func_ptr(5);
    }
    
    /* Create and use local complex type */
    int (*(*local_complex)(int))[2];
    int local_array[2] = {10, 20};
    int (*array_ptr)[2] = &local_array;
    
    /* Use multi_array */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 4; k++) {
                multi_array[i][j][k].x = i + j + k;
                sum += multi_array[i][j][k].x;
            }
        }
    }
    
    /* Print result to create observable side effect */
    printf("Result: %d\n", sum);
    
    return 0;
}

/* Function definition for the complex prototype */
void process_data(int (*table[])[5], struct { int x; char y[10]; } param) {
    /* Implementation not critical for coverage */
    param.x = 0;
    table = table; /* Silence unused parameter warning */
}

/* Additional complex declarations */
/* Function returning pointer to array of function pointers */
int (*(*get_func_table(void))[5])(int) {
    static int (*table[5])(int) = {NULL};
    return &table;
}

/* Struct with anonymous union and struct */
struct Weird {
    struct {
        int type;
    };
    union {
        int i;
        float f;
    };
    int (*methods[2])(struct Weird*);
};

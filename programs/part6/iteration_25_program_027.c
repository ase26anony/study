/* test_complex_types.c - Complex type declarations to exercise gengtype parser */

/* 1. Complex function declarators with nested parentheses */
typedef int (*fn_simple)(void);
typedef fn_simple (*fn_returning_fn_ptr)(int);
typedef int (*(*fn_complex)(int (*)(double)))(char);

/* Global function pointer using complex typedef */
fn_returning_fn_ptr global_fn_ptr = 0;

/* Even more complex: pointer to function returning pointer to array of function pointers */
int (*(*(*global_mega_ptr)(void))[3])(int, int);

/* 2. Multi-dimensional and complex array declarations */
/* Array where size is parenthesized expression */
char (*string_array[(2+3)])[20];

/* Array of pointers to functions returning pointers to arrays */
int (*(*array_of_func_ptrs[5])(int))[10];

/* 3D array with parenthesized dimension */
int three_d_array[2][(3+1)][5];

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    fn_simple func_ptr;
};

/* Global struct with deeply nested initializer */
struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { "test", {{7,8}, {9,10}} }, 
    0 
};

/* Union with array and nested struct */
union ComplexUnion {
    int (*func_array[3])(void);
    struct {
        int x;
        struct Nested nested;
    } data;
};

/* Designated initializers with nested braces */
int designated_array[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combine all three delimiter types in single declarations */
/* Function prototype with complex parameter */
void process(int (*table[])[5], struct { int x; int y[2]; } param);

/* Variable with initializer using compound literal containing array */
int (*ptr_to_array)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

/* Even more complex: pointer to array of pointers to functions */
int (*(*complex_array_ptr)[3])(void) = 0;

/* 5. Additional global declarations */
struct Data {
    int (*func)(int);
    int matrix[2][(1+2)];
} global_data = { 0, {{1,2,3}, {4,5,6}} };

/* Function returning pointer to function with nested parentheses */
int (*(*get_callback(void))(int))(void) {
    return 0;
}

/* 6. Simple functions to assign to function pointers */
int simple_func(int x) { return x * 2; }
int another_func(void) { return 42; }
int func_taking_double(double d) { return (int)d; }
int func_returning_int(char c) { return (int)c; }

/* Main function to use the complex types */
int main(void) {
    /* 1. Use complex function pointer typedef */
    fn_complex local_complex = 0;
    
    /* 2. Access nested array from global struct */
    int sum = 0;
    sum += global_nested.a[0][0];
    sum += global_nested.a[1][2];
    sum += global_nested.inner.values[0][1];
    
    /* 3. Use designated array */
    sum += designated_array[0][2];
    sum += designated_array[1][0];
    
    /* 4. Access compound literal through pointer */
    if (ptr_to_array) {
        sum += ptr_to_array[0][1];
        sum += ptr_to_array[1][0];
    }
    
    /* 5. Use global_data */
    sum += global_data.matrix[0][1];
    sum += global_data.matrix[1][2];
    
    /* 6. Assign function pointers (even if not called) */
    global_data.func = simple_func;
    
    /* Create local instance of complex struct with initializer */
    struct Nested local_nested = {
        {{11,12,13}, {14,15,16}},
        { "local", {{17,18}, {19,20}} },
        another_func
    };
    
    sum += local_nested.a[0][1];
    sum += local_nested.inner.values[1][0];
    
    /* Use array of function pointers */
    int (*(*local_array[2])(int))[3];
    
    /* Complex declaration inside function */
    int (*(*func_local)(int (*(*)(void))[3]))(char) = 0;
    
    /* Nested initializer in function scope */
    int nested_init[2][2][2] = { { {1,2}, {3,4} }, { {5,6}, {7,8} } };
    sum += nested_init[0][1][0];
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}

/* Additional complex prototypes to ensure parser sees them */
typedef int Matrix[3][(2+2)];
Matrix* create_matrix(void);

struct UltimateType {
    int (*(*func_ptr_array[(1+2)])(Matrix*))[2];
    union {
        int x;
        struct Nested nested;
    } u;
};

/* One more global with all delimiters */
struct UltimateType global_ultimate = {
    {0, 0, 0},
    { .nested = { {{0}}, {{0}}, 0 } }
};

/* test_complex_types.c - Designed to exercise gengtype-parse.cc's consume_balanced function */

/* 1. Complex function declarators with nested parentheses */
typedef int (*func_ptr_simple)(void);
typedef func_ptr_simple (*func_ptr_returning_func_ptr)(int, double);
int (*(*global_func_ptr1)(int (*)(double)))(char);
int (*(*(*global_func_ptr2)(void))(int))[3];

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*multi_dim_array[2][3])(float);
struct { int (*table[4])[5]; } global_struct_with_array;

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int values[2][2];
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        int x;
        double y;
    } u;
};

/* Global variable with deeply nested initializer */
struct Nested global_nested = { 
    {{1,2,3}, {4,5,6}}, 
    { 
        "test", 
        {{7,8}, {9,10}}
    },
    { .y = 3.14 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1,2,3}, [1] = {4,5,6} };

/* 4. Combined constructs in single declarations */
int (*combined_ptr)[2] = (int[][2]){ {1,2}, {3,4}, {5,6} };

struct ComplexParam {
    int (*callback)(int, int);
};

void process(struct ComplexParam param1, int (*table[])[5]);

/* Function returning complex type */
struct Nested (*get_nested_array(void))[2] {
    static struct Nested arr[2][2] = {
        { {{1,2,3},{4,5,6}}, {"ptr1", {{11,12},{13,14}}}, {.x=1} },
        { {{7,8,9},{10,11,12}}, {"ptr2", {{15,16},{17,18}}}, {.y=2.71} }
    };
    return arr;
}

/* 5. More global declarations with all delimiters */
union UltraComplex {
    int (*(*func_field)(void))[3];
    struct {
        char (*strings[2])[10];
    } nested_struct;
} global_union = {
    .nested_struct = {
        .strings = { NULL, NULL }
    }
};

/* Simple function compatible with our function pointers */
int simple_func(int x, int y) {
    return x + y;
}

int func_taking_double(double d) {
    return (int)d;
}

int func_returning_int(void) {
    return 42;
}

/* Main function to use the complex types and prevent dead code elimination */
int main(void) {
    /* 1. Use complex function pointer types */
    func_ptr_returning_func_ptr local_func_ptr = NULL;
    
    /* 2. Access nested array elements */
    int sum = 0;
    sum += global_nested.a[0][0];      /* 1 */
    sum += global_nested.a[1][2];      /* 6 */
    sum += global_nested.inner.values[0][1]; /* 8 */
    sum += arr[0][2];                  /* 3 */
    sum += arr[1][0];                  /* 4 */
    
    /* 3. Use combined pointer */
    sum += combined_ptr[0][0];         /* 1 */
    sum += combined_ptr[2][1];         /* 6 */
    
    /* 4. Call function returning complex type */
    struct Nested (*nested_arr_ptr)[2] = get_nested_array();
    sum += nested_arr_ptr[0][0].a[0][0]; /* 1 */
    sum += nested_arr_ptr[0][1].inner.values[1][1]; /* 14 */
    
    /* 5. Assign to function pointer */
    struct ComplexParam cp;
    cp.callback = simple_func;
    sum += cp.callback(2, 3);          /* 5 */
    
    /* 6. Use global union */
    global_union.nested_struct.strings[0] = NULL;
    
    /* Print result to create observable side effect */
    printf("Sum of various elements: %d\n", sum);
    
    /* Additional complex local declaration */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    
    return sum > 0 ? 0 : 1;
}

/* Function prototype with complex parameter (declared earlier, defined here if needed) */
void process(struct ComplexParam param1, int (*table[])[5]) {
    /* Empty implementation - just for declaration */
    (void)param1;
    (void)table;
}

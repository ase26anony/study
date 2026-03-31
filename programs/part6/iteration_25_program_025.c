/* Complex type definitions to exercise gengtype parser's balanced delimiter handling */

/* 1. Complex function declarators with nested parentheses */
typedef int (*simple_func)(void);
typedef simple_func (*func_returning_func_ptr)(int);
typedef int (*(*complex_func_ptr)(int (*)(double)))(char);

/* 2. Multi-dimensional and complex array declarations */
int (*(*array_of_func_ptrs[5])(int))[10];
char (*strings[(2+3)])[20];
int (*matrix[3][4])(float, double);

/* 3. Nested aggregate initializers and type definitions */
struct Inner {
    char *p;
    int (*callback)(int, int);
};

struct Nested {
    int a[2][3];
    struct Inner inner;
    union {
        long x;
        double y;
    } data;
};

/* Global struct with initializer */
struct Nested global_nested = { 
    {{1, 2, 3}, {4, 5, 6}}, 
    { NULL, NULL },
    { .x = 42 }
};

/* Array with designated initializers */
int arr[2][3] = { [0] = {1, 2, 3}, [1] = {4, 5, 6} };

/* 4. Combine constructs in single declarations */
int (*ptr_to_array)[2] = (int[][2]){ {1, 2}, {3, 4} };

struct Config {
    int (*process)(int (*table[])[5], struct { int x; } param);
    char name[20];
};

/* 5. Global scope complex declarations */
int (*(*global_var)(void))[3];

struct Data { 
    int (*func)(int); 
    struct Nested nested;
} global_data = { 
    NULL, 
    { {{7, 8, 9}, {10, 11, 12}}, { NULL, NULL }, { .y = 3.14 } }
};

/* Function prototypes with complex parameters */
void handle_data(int (*(*callback)(int[][2]))[3], struct Config *cfg);
int (*register_callback(int (*func)(int)))(void);

/* Simple functions for assignment */
int simple_add(int a, int b) { return a + b; }
int process_int(int x) { return x * 2; }
int (*get_array(void))[3] { static int arr[2][3] = {{1,2,3},{4,5,6}}; return arr; }

/* Main function to use the complex types */
int main(void) {
    /* 1. Use complex typedef */
    func_returning_func_ptr frfp = NULL;
    
    /* 2. Access nested array from global struct */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            sum += global_nested.a[i][j];
        }
    }
    
    /* 3. Use array with designated initializers */
    sum += arr[0][1] + arr[1][2];
    
    /* 4. Use pointer to array with compound literal */
    sum += ptr_to_array[0][0] + ptr_to_array[1][1];
    
    /* 5. Assign function pointer in global struct */
    global_data.func = process_int;
    if (global_data.func) {
        sum += global_data.func(5);
    }
    
    /* 6. Access nested union in global struct */
    sum += (int)global_data.nested.data.y;
    
    /* 7. Complex local declaration mixing all delimiters */
    struct {
        int (*funcs[2])(int);
        struct Nested data;
    } local_var = {
        .funcs = { process_int, process_int },
        .data = { {{13,14,15},{16,17,18}}, {NULL, NULL}, {.x = 99} }
    };
    
    sum += local_var.data.a[0][0];
    
    /* 8. Even more complex declaration */
    int (*(*local_complex)(int (*)(double)))(char) = NULL;
    
    /* 9. Array of pointers to functions returning pointers to arrays */
    int (*(*func_array[2])(int))[3];
    func_array[0] = (int (*(*)(int))[3])get_array;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sum);
    
    return 0;
}

/* Additional complex declarations at file scope */
union UltraComplex {
    struct {
        int (*(*member1)(void))[5];
        char (*member2[10])(struct { int x; int y; });
    } s;
    void (*actions[3])(int (*)(int), char [][10]);
};

/* Template for complex C++ code (if compiled as C++) */
#ifdef __cplusplus
class ComplexClass {
public:
    virtual int (*(*virtual_method(int (*(*arg)(void))[3]))(void))[5] = 0;
    
    template<typename T>
    T (*template_method(T (*func)(T)))(T*, int) {
        return nullptr;
    }
};
#endif

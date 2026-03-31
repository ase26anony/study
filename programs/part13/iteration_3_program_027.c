/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void (*)(int, char), double);
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int flags:4;
            unsigned int mode:2;
            unsigned int :26;  /* Unnamed bit-field */
        } bits;
        unsigned int raw;
    } control;
    
    /* Multi-dimensional array */
    float matrix[3][4][2];
    
    /* Pointer to function returning pointer to array */
    char (*(*complex_func)(int))[10];
};

/* 2. Function pointers with deeply nested signatures */
typedef void (*(*SignalHandler)(int sig, void (*(*callback)(int))(void)))(int);

/* Function pointer with array parameter */
int (*process_matrix)(int rows, int cols, double (*matrix)[cols]);

/* 3. Multi-dimensional arrays and flexible array members */
struct DataPacket {
    int header;
    int payload_size;
    /* Flexible array member at end */
    unsigned char data[];
};

struct Tensor {
    int dimensions;
    /* Variable length array pointer */
    double (*elements)[][10];
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct DeviceRegisters {
    union {
        struct {
            unsigned int enable:1;
            unsigned int mode:3;
            unsigned int error:1;
            unsigned int reserved:27;
        };
        unsigned int reg32;
    } status;
    
    struct {
        unsigned char port:4;
        unsigned char :4;  /* Padding */
    } config;
};

/* 5. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*(*)(T, T))(T)
#define ARRAY_DECL(T, N) T (*name)[N]
#define NESTED_PTR(T) T (*(*(*)(void))(void))

/* Using the macros */
PTR_FUNC(int) global_func_ptr;
ARRAY_DECL(double, 5) matrix_ptr;

struct MacroStruct {
    NESTED_PTR(char) triple_ptr;
    PTR_FUNC(float) math_op;
};

/* 6. GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int x __attribute__((aligned(8)));
    char y;
} __attribute__((deprecated));

int __attribute__((const)) pure_function(int a, int b)
    __attribute__((warn_unused_result));

/* 7. Single declaration combining all bracket types (the ultimate test) */
struct UltimateType {
    /* Function returning function pointer with array parameter */
    void (*(*signal_handler)(int sig, 
        void (*(*get_callback)(char id))(int (*)(char[10]))))(int);
    
    /* Array of function pointers */
    int (*(*func_ptr_array[3])(int (*)(char[10]), 
        struct { int x; double y; }))[5];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:5;
            unsigned int b:3;
            unsigned int c:24;
        };
        unsigned int full;
    } flags;
    
    /* Multi-dimensional flexible array member */
    int flex_matrix[][2][3];
    
    /* Pointer to array of pointers to functions */
    char (*(*(*complex_array)[5])(int))[10];
} __attribute__((packed));

/* Additional complex cases */

/* Function with nested parameter types */
typedef int (*(*FactoryFunc)(int count, 
    struct { int id; char name[20]; } config))(void);

/* Union containing struct with function pointer array */
union Container {
    struct {
        int type;
        void (*operations[4])(union Container *);
    } meta;
    double value;
    long *ptr_array[2];
};

/* Typedef with attributes */
typedef volatile const int __attribute__((vector_size(16))) VectInt;

/* Struct with zero-length array (GCC extension) */
struct ZeroArray {
    int length;
    int items[0];  /* Zero-length array */
};

/* Nested parentheses in function declarations */
void (*(*register_callback(
    void (*(*factory)(int))(char), 
    int priority
))(int))(void);

/* Complex array of structs containing function pointers */
struct CallbackRecord {
    int id;
    char name[32];
    void (*(*get_handler)(int event))(void);
} callback_table[] = {
    {1, "start", 0},
    {2, "stop", 0},
    {3, "pause", 0}
};

/* Final test: Everything combined */
struct __attribute__((aligned(32))) MasterType {
    /* 1. Parentheses: function pointers */
    int (*(*master_func)(int (*(*)(char))(double)))(float);
    
    /* 2. Brackets: arrays */
    struct {
        int data[10];
        char *ptr_array[5][2];
    } nested_arrays;
    
    /* 3. Braces: anonymous struct/union */
    union {
        struct {
            unsigned long field1:16;
            unsigned long field2:16;
            unsigned long field3:16;
            unsigned long field4:16;
        };
        unsigned long long full;
    } bit_packed;
    
    /* Flexible array member with pointers */
    struct UltimateType *flex_list[];
};

#endif /* TEST_GENGTYPE_COVERAGE_H */

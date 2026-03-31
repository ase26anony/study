#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h)"))) string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int ival;
    float fval;
    char* GTY((tag("0"))) sval;
};

/* TYPE_STRUCT: Regular struct with various members */
struct GTY(()) complex_struct {
    /* Scalar members */
    int id;
    enum color color;
    double weight;
    
    /* Pointer member */
    struct complex_struct* GTY((chain_next("%h.next"))) next;
    
    /* Array member */
    int scores[10];
    
    /* String member */
    char* GTY((length("strlen(%h.name)"))) name;
    
    /* Union member */
    union basic_union data;
    
    /* Callback member */
    callback_func handler;
    
    /* Nested anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } point;
    
    /* Bit fields */
    unsigned int flags : 4;
    unsigned int status : 2;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_id;
    char* user_name;
};

/* TYPE_ARRAY: Various array types */
struct GTY(()) array_container {
    /* Fixed-size array of structs */
    struct complex_struct items[5];
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Array of pointers */
    struct complex_struct* GTY(()) ptr_array[8];
    
    /* Variable-length array (using length option) */
    int* GTY((length("%h.vla_len"))) variable_array;
    int vla_len;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_showcase {
    /* Pointer to struct */
    struct complex_struct* struct_ptr;
    
    /* Pointer to union */
    union basic_union* union_ptr;
    
    /* Pointer to scalar */
    int* int_ptr;
    
    /* Pointer to pointer */
    struct complex_struct** double_ptr;
    
    /* Void pointer */
    void* GTY((skip)) opaque_ptr;
    
    /* Function pointer */
    void (*func_ptr)(void);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to callback */
    callback_func* callback_ptr;
};

/* Chain structure for testing chain_next/chain_prev */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chain_node {
    int value;
    struct chain_node* next;
    struct chain_node* prev;
};

/* Structure with desc (discriminator) field */
struct GTY((desc("%1.type"))) discriminated_union {
    enum {
        TYPE_INT,
        TYPE_FLOAT,
        TYPE_STRING
    } type;
    
    union {
        int ival;
        float fval;
        char* sval;
    } data;
};

#endif /* TEST_TYPES_H */

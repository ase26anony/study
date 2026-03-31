#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
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
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h) + 1"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, void*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int i;
    float f;
    char* GTY((length("strlen(%h) + 1"))) str;
};

union GTY(()) tagged_union {
    enum { TAG_INT, TAG_FLOAT, TAG_STRING } tag;
    struct {
        int int_value;
    };
    struct {
        float float_value;
    };
    struct {
        char* GTY((length("strlen(%h) + 1"))) string_value;
    };
};

/* TYPE_STRUCT: Regular struct with various members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float weight;
    double score;
    enum color color;
    
    /* Nested anonymous struct */
    struct {
        int x;
        int y;
    } position;
    
    /* Bit fields */
    unsigned int flags : 4;
    unsigned int status : 2;
    
    /* Pointer member */
    struct basic_struct* GTY((skip)) next;
};

/* More complex struct with arrays and pointers */
struct GTY(()) complex_struct {
    /* TYPE_ARRAY: Fixed-size array */
    int numbers[10];
    
    /* Multi-dimensional array */
    float matrix[3][3];
    
    /* Array of structs */
    struct basic_struct items[5];
    
    /* Pointer to array */
    int* GTY(()) dynamic_array;
    
    /* String array */
    char* GTY((length("strlen(%h) + 1"))) strings[8];
    
    /* Union member */
    union basic_union data;
    
    /* Callback member */
    callback_func handler;
    
    /* Chain pointers for GC */
    struct complex_struct* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
    struct complex_struct* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) prev;
    
    /* Length field for variable array */
    size_t count;
    
    /* Variable length array at end */
    int GTY((length("%h.count"))) variable_array[];
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_id;
    void* user_data;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_struct {
    /* Simple pointers */
    int* int_ptr;
    float* float_ptr;
    char** string_ptr_ptr;
    
    /* Struct pointers */
    struct basic_struct* struct_ptr;
    struct complex_struct* complex_ptr;
    
    /* Union pointer */
    union basic_union* union_ptr;
    
    /* Function pointer */
    void (*func_ptr)(void);
    
    /* Void pointer */
    void* void_ptr;
    
    /* Pointer to pointer */
    struct pointer_struct** self_ptr_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to callback */
    callback_func* callback_ptr;
};

/* TYPE_ARRAY: Standalone array types */
typedef int GTY(()) int_array[20];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[8];
typedef callback_func GTY(()) callback_array[3];

/* Container struct that references everything */
struct GTY(()) master_container {
    /* All scalar types */
    scalar_int s_int;
    scalar_long s_long;
    scalar_float s_float;
    scalar_double s_double;
    color_t color;
    
    /* String types */
    string_ptr str;
    const_string_ptr cstr;
    
    /* Callback */
    callback_func cb;
    
    /* Struct types */
    struct basic_struct basic;
    struct complex_struct* complex;
    
    /* Union types */
    union basic_union union1;
    union tagged_union* tagged_union_ptr;
    
    /* User struct */
    struct user_struct user;
    
    /* Pointer types */
    struct pointer_struct pointers;
    
    /* Array types */
    int_array fixed_array;
    struct_array structs_array;
    
    /* Undefined type pointer */
    struct opaque_struct* opaque_ptr;
    
    /* Self-reference for graph */
    struct master_container* GTY((skip)) next_container;
};

#endif /* TEST_TYPES_H */

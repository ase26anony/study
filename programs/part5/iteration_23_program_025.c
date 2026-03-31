#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    struct basic_struct* GTY((chain_next)) next;
};

/* TYPE_STRUCT with bitfields and anonymous struct */
struct GTY(()) complex_struct {
    unsigned int GTY(()) flags : 8;
    unsigned int GTY(()) mode : 4;
    
    /* Anonymous struct member */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Nested struct */
    struct basic_struct GTY(()) nested;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) user_data;
    void* GTY(()) user_pointer;
};

/* TYPE_UNION: Basic union */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;
};

/* TYPE_UNION with tag (discriminated union) */
struct GTY(()) tagged_union_container {
    enum GTY(()) { INT_TYPE, DOUBLE_TYPE, STRING_TYPE } tag;
    union GTY(()) {
        int GTY(()) i;
        double GTY(()) d;
        char* GTY(()) s;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef int* GTY(()) int_ptr;
typedef void (*GTY(()) func_ptr)(int);  /* Function pointer */

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h)"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Callback function type */
typedef int (*GTY(()) callback_func)(int, void*);

/* Struct containing a callback */
struct GTY(()) callback_container {
    callback_func GTY(()) handler;
    void* GTY(()) user_data;
};

/* Chain of structures for testing chain_next/chain_prev */
struct GTY(()) chain_struct {
    int GTY(()) value;
    struct chain_struct* GTY((chain_next)) next;
    struct chain_struct* GTY((chain_prev)) prev;
};

/* Variable length array using length option */
struct GTY(()) var_len_struct {
    int GTY(()) count;
    int GTY((length("%0.count"))) data[];
};

#endif /* TEST_TYPES_H */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color_enum {
    RED,
    GREEN,
    BLUE
} color_t;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY((tag("0"))) scalar_int;
    char GTY((tag("1"))) scalar_char;
    float GTY((tag("2"))) scalar_float;
    double GTY((tag("3"))) scalar_double;
    color_t GTY((tag("4"))) enum_field;
    
    /* Bit-fields */
    unsigned int GTY((tag("5"))) bitfield1 : 4;
    unsigned int GTY((tag("6"))) bitfield2 : 8;
    
    /* Anonymous struct */
    struct GTY(()) {
        int inner_a;
        int inner_b;
    } anonymous;
};

/* TYPE_STRUCT: Nested struct with pointers */
struct GTY(()) complex_struct {
    struct basic_struct GTY((tag("0"))) nested;
    struct complex_struct* GTY((tag("1"))) next;
    struct complex_struct* GTY((tag("2"))) prev;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_data;
    void* GTY((skip)) user_pointer;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int GTY((tag("0"))) as_int;
    float GTY((tag("1"))) as_float;
    char* GTY((tag("2"))) as_string;
    struct basic_struct GTY((tag("3"))) as_struct;
};

/* TYPE_UNION: Tagged union within struct */
struct GTY(()) tagged_union_container {
    int GTY((tag("0"))) tag;
    union GTY((desc("tag"))) {
        int GTY((tag("0"))) case_int;
        float GTY((tag("1"))) case_float;
        struct basic_struct GTY((tag("2"))) case_struct;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr_t;
typedef union basic_union* GTY(()) union_ptr_t;
typedef void (*GTY(()) func_ptr_t)(int, char*);
typedef int* GTY(()) int_ptr_t;
typedef void* GTY(()) void_ptr_t;

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array_10[10];
typedef struct basic_struct GTY(()) struct_array_5[5];
typedef union basic_union GTY(()) union_array_3[3];
typedef int GTY(()) multi_dim_array[4][5][6];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h)"))) string_ptr_t;
typedef const char* GTY(()) const_string_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) callback_func_t)(int, void*);

/* Struct containing callback */
struct GTY(()) callback_container {
    callback_func_t GTY((tag("0"))) callback;
    void* GTY((tag("1"))) user_data;
};

/* Chain-linked structure using chain_next/chain_prev */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chain_struct {
    int id;
    struct chain_struct* GTY((skip)) next;
    struct chain_struct* GTY((skip)) prev;
};

/* Array with length field */
struct GTY(()) array_with_length {
    int GTY((tag("0"))) length;
    int GTY((length("%0"))) data[];
};

#endif /* TEST_TYPES_H */

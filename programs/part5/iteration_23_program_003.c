/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_undefined;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    float value;
    enum color color;
};

/* TYPE_STRUCT with nested anonymous struct */
struct GTY(()) complex_struct {
    int tag;
    union {
        int int_val;
        float float_val;
    } GTY((tag("tag"))) data;
    
    struct {
        int x;
        int y;
    } GTY(()) point;
    
    /* Bit-fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
};

/* TYPE_STRUCT with chain_next option */
struct GTY((chain_next("%h.next"))) linked_node {
    int value;
    struct linked_node *GTY((skip)) next;
};

/* TYPE_UNION: Basic union */
union GTY(()) basic_union {
    int int_val;
    float float_val;
    double double_val;
    char *GTY((length("%h.str_len"))) string_val;
    int str_len;
};

/* TYPE_UNION with tag */
union GTY((desc("%d.type"))) tagged_union {
    int type;
    struct {
        int x;
        int y;
    } GTY((tag("1"))) point;
    struct {
        float radius;
        float angle;
    } GTY((tag("2"))) polar;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_id;
    char *user_name;
    void *user_data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY(()) struct_ptr;
typedef union basic_union *GTY(()) union_ptr;
typedef int *GTY(()) int_ptr;
typedef void *GTY(()) void_ptr;
typedef void (*GTY(()) func_ptr)(int, char*);

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[3][3];
typedef char *GTY(()) string_array[8];

/* TYPE_STRING: String types */
typedef char *GTY((length("strlen(%h)+1"))) counted_string;
typedef const char *GTY(()) const_string;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) callback_func)(int, void*);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func handler;
    void *GTY((skip)) user_data;
    int (*GTY(()) another_callback)(float, double);
};

/* Complex type with all kinds of references */
struct GTY(()) master_struct {
    /* TYPE_STRUCT member */
    struct basic_struct nested_struct;
    
    /* TYPE_UNION member */
    union tagged_union data_union;
    
    /* TYPE_POINTER members */
    struct opaque_undefined *GTY((skip)) opaque_ptr;
    struct master_struct *GTY((chain_next("%h.next"))) next;
    struct master_struct **GTY((skip)) prev_ptr;
    
    /* TYPE_ARRAY members */
    int numbers[20];
    struct basic_struct objects[5];
    char *strings[10];
    
    /* TYPE_SCALAR members */
    enum color primary_color;
    int counter;
    float weight;
    double precision;
    
    /* TYPE_STRING member */
    char *GTY((length("strlen(%h.name)+1"))) name;
    
    /* TYPE_CALLBACK member */
    callback_func on_event;
    
    /* TYPE_USER_STRUCT pointer */
    struct user_struct *GTY((user)) user_info;
    
    /* Anonymous union */
    union {
        int as_int;
        float as_float;
        struct {
            short x;
            short y;
        } as_point;
    } GTY(()) variant;
};

/* Global variable declarations */
extern struct master_struct GTY(()) global_master;
extern struct linked_node *GTY(()) global_list;
extern union basic_union GTY(()) global_union_array[4];
extern callback_func GTY(()) global_callbacks[3];

#endif /* TEST_TYPES_H */

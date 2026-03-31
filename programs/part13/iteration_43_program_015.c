/* Test file to cover all TYPE_* cases in gengtype-state.cc */

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING - string type */
typedef const char *string_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char *, void *);

/* TYPE_POINTER - pointer types */
typedef int* int_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_ARRAY - array types */
typedef int fixed_array[10];
typedef char *ptr_array[5];

/* TYPE_UNION - union type */
union my_union {
    int a;
    void *b;
    float c;
};

/* TYPE_STRUCT - plain C struct (no GTY marker) */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT - GTY user-defined struct */
struct GTY((user)) user_struct {
    void *data;
    int id;
    callback_type callback;
};

/* TYPE_LANG_STRUCT - language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    union my_union lang_union;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER field */
    struct opaque_struct *opaque_ptr_field;
    
    /* TYPE_ARRAY field */
    int int_array[20];
    
    /* TYPE_STRING field */
    const char *name;
    
    /* TYPE_UNION field */
    union my_union data_union;
    
    /* TYPE_CALLBACK field */
    callback_type handler;
    
    /* Nested TYPE_STRUCT */
    struct inner_struct {
        int inner_field;
        float inner_float;
    } inner;
    
    /* Pointer to TYPE_USER_STRUCT */
    struct user_struct *user_data;
    
    /* Flexible array member (TYPE_ARRAY) */
    char flexible_array[];
};

/* Another GTY struct with various type combinations */
struct GTY(()) another_gty_struct {
    /* TYPE_SCALAR fields */
    scalar_int count;
    enum my_enum state;
    
    /* TYPE_POINTER to array */
    int (*matrix_ptr)[10];
    
    /* TYPE_ARRAY of pointers */
    void *ptr_list[8];
    
    /* TYPE_STRING array */
    const char *messages[5];
    
    /* TYPE_CALLBACK array */
    callback_type callbacks[3];
    
    /* Nested TYPE_UNION */
    union {
        int as_int;
        float as_float;
        void *as_ptr;
    } variant;
};

/* Union with GTY marker */
union GTY(()) gty_union {
    struct complex_nested *nested_ptr;
    struct another_gty_struct *gty_struct_ptr;
    callback_type func_ptr;
};

/* Typedef for a complex pointer type */
typedef union gty_union *(*complex_callback)(struct lang_specific *, int);

/* Struct containing all major types */
struct GTY(()) all_types_container {
    /* TYPE_SCALAR */
    int scalar_int_field;
    enum my_enum enum_field;
    
    /* TYPE_STRING */
    const char *string_field;
    
    /* TYPE_POINTER */
    void *void_ptr_field;
    struct plain_struct *struct_ptr_field;
    
    /* TYPE_ARRAY */
    int fixed_size_array[15];
    struct another_gty_struct *ptr_array[10];
    
    /* TYPE_UNION */
    union my_union union_field;
    
    /* TYPE_CALLBACK */
    callback_type callback_field;
    complex_callback complex_callback_field;
    
    /* TYPE_STRUCT (nested) */
    struct {
        int x;
        int y;
    } point;
    
    /* TYPE_USER_STRUCT pointer */
    struct user_struct *user_struct_ptr;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_specific *lang_struct_ptr;
    
    /* Flexible array of strings */
    const char *string_array[];
};

/* Forward declaration chain */
struct forward_declared;
struct another_forward;

/* Complete the forward declarations */
struct forward_declared {
    struct another_forward *next;
    int value;
};

struct another_forward {
    struct forward_declared *prev;
    const char *name;
};

/* Enumeration type definition */
typedef enum {
    STATE_INIT,
    STATE_RUNNING,
    STATE_STOPPED,
    STATE_ERROR
} process_state_t;

/* Struct with bitfields (still TYPE_SCALAR) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int value : 16;
};

/* Array of function pointers */
typedef void (*action_array[5])(void);

/* Struct with array of function pointers */
struct GTY(()) actions_container {
    action_array actions;
    int current_action;
};

/* Void pointer typedef */
typedef void *generic_handle;

/* Const pointer typedef */
typedef const int *const_int_ptr;

/* Double pointer */
typedef struct all_types_container **container_ptr_ptr;

/* Empty struct (edge case) */
struct GTY(()) empty_struct {
    /* No fields */
};

/* Union with only scalar types */
union scalar_union {
    int i;
    float f;
    double d;
    long l;
};

/* Final comprehensive type that references many others */
struct GTY(()) master_type {
    struct all_types_container *container;
    union gty_union data;
    struct lang_specific lang_data;
    struct user_struct *users[5];
    process_state_t state;
    struct bitfield_struct flags;
    struct actions_container actions;
    struct empty_struct empty;
    union scalar_union scalar_data;
    container_ptr_ptr nested_ptr;
};

// Creating a pointer to int type
struct type *int_type = malloc(sizeof(struct type));
int_type->kind = TYPE_SCALAR;

struct type *int_ptr = malloc(sizeof(struct type));
int_ptr->kind = TYPE_POINTER;
int_ptr->u.pointer_to = int_type;

// Creating an array of floats
struct type *float_type = malloc(sizeof(struct type));
float_type->kind = TYPE_SCALAR;

struct type *float_array = malloc(sizeof(struct type));
float_array->kind = TYPE_ARRAY;
float_array->u.element_type = float_type;

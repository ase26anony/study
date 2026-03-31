// Creating an integer pointer type
struct type *int_type = malloc(sizeof(struct type));
int_type->kind = TYPE_SCALAR;

struct type *int_ptr_type = malloc(sizeof(struct type));
int_ptr_type->kind = TYPE_POINTER;
int_ptr_type->u.pointer_to = int_type;

// Creating an array of integers
struct type *int_array_type = malloc(sizeof(struct type));
int_array_type->kind = TYPE_ARRAY;
int_array_type->u.element_type = int_type;

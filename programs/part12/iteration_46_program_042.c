// Create a pointer to int type
struct type *int_type = malloc(sizeof(struct type));
int_type->kind = TYPE_SCALAR;

struct type *int_ptr_type = malloc(sizeof(struct type));
int_ptr_type->kind = TYPE_POINTER;
int_ptr_type->u.pointer_to = int_type;

// Create an array of pointers to int
struct type *array_type = malloc(sizeof(struct type));
array_type->kind = TYPE_ARRAY;
array_type->u.element_type = int_ptr_type;

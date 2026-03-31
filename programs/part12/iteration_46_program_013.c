// Creating a pointer to int type
struct type *int_type = malloc(sizeof(struct type));
int_type->kind = TYPE_SCALAR;

struct type *int_ptr_type = malloc(sizeof(struct type));
int_ptr_type->kind = TYPE_POINTER;
int_ptr_type->u.pointer_to = int_type;

// Creating an array of strings
struct type *string_type = malloc(sizeof(struct type));
string_type->kind = TYPE_STRING;

struct type *string_array_type = malloc(sizeof(struct type));
string_array_type->kind = TYPE_ARRAY;
string_array_type->u.element_type = string_type;

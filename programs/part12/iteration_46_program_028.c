// Creating a pointer to int
struct type *int_type = create_type(TYPE_SCALAR);
struct type *int_ptr = create_type(TYPE_POINTER);
int_ptr->u.pointer_to = int_type;

// Creating an array of floats
struct type *float_type = create_type(TYPE_SCALAR);
struct type *float_array = create_type(TYPE_ARRAY);
float_array->u.element_type = float_type;

// Linking types in a list (e.g., function parameters)
struct type *param1 = create_type(TYPE_SCALAR);
struct type *param2 = create_type(TYPE_SCALAR);
param1->next = param2;

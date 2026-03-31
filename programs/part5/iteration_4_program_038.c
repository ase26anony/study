#include <stdatomic.h>

atomic_int flag = 0;

if (atomic_load(&flag)) {
    test_invalid_address_of_bitfield();
    test_incomplete_sizeof();
}

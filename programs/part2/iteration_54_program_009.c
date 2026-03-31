#define CHECK(x) ((x) > 0)

if (CHECK(x)) {
    x = 10;  // If CHECK(x) expands to (x > 0) and is evaluated again, behavior changes
}

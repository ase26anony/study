int a, b, c;
bool wasPositive = (a > 0);

if (wasPositive) {
    b = 10;
    a = 5;  // Now it's clear we're intentionally updating a
} else {
    b = 20;
}

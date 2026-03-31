int a = 1, b, c;
if (a > 0) {        // true, enters then block
    b = 10;         // b becomes 10
    a = 5;          // a becomes 5 (still > 0, but condition isn't rechecked)
} else {
    b = 20;         // skipped
}
// At this point: a = 5, b = 10

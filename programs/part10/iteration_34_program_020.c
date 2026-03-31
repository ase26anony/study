float a = 0.0f / 0.0f;  // NaN
float b = 5.0f;

if (a > b) { /* false - regular comparison with NaN */ }
if (isgreater(a, b)) { /* false - but no FP exception */ }
if (isunordered(a, b)) { /* true - detects the NaN */ }

Device (e.g., GPU)
├── Team 0
│   ├── Thread 0: processes j = 0..k with SIMD vectorization on i-loop
│   └── Thread 1: processes j = k+1..m with SIMD vectorization on i-loop
├── Team 1
│   ├── Thread 0: processes j = m+1..n with SIMD vectorization
│   └── Thread 1: processes j = n+1..p with SIMD vectorization
└── ...

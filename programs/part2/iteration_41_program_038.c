Device (e.g., GPU)
├── Multiple Teams (distributed across compute units)
│   ├── Team 0: processes j = 0..k
│   ├── Team 1: processes j = k+1..m
│   └── ...
└── Within each team:
    └── SIMD lanes process multiple i iterations concurrently

Device (GPU/Accelerator)
├── Team 0
│   ├── Threads execute j = 0..(N/num_teams)
│   └── Each thread uses SIMD for inner loop (16 i iterations at once)
├── Team 1
│   ├── Threads execute j = (N/num_teams)..(2N/num_teams)
│   └── Each thread uses SIMD for inner loop
└── ...

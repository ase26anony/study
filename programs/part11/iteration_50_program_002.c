GPU Execution Hierarchy:
└── Gang 1 (thread block)
    ├── Worker 1
    │   ├── Vector lane 1
    │   ├── Vector lane 2
    │   └── ...
    ├── Worker 2
    └── ...
└── Gang 2
    └── ...

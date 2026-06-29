import subprocess
import sys
import os

# Path to the directory containing mpc.lf
LF_PROJECT_DIR = os.path.join(os.path.dirname(__file__), 'mpc-lf-main')

print("=== Step 1: Compiling Lingua Franca Code ===")
compile_process = subprocess.run(
    ['lfc', 'mpc.lf'],
    cwd=LF_PROJECT_DIR,        # ← run lfc from the mpc-lf-main/ directory
    capture_output=True,
    text=True
)

if compile_process.returncode != 0:
    print("Compilation Failed! Error log:")
    print(compile_process.stderr)
    sys.exit(1)

print("Success! Binary generated.\n")
print("=== Step 2: Executing Simulation ===")

try:
    subprocess.run(
        ['./bin/mpc'],
        cwd=LF_PROJECT_DIR     # ← run binary from the same directory
    )
    print("\n=== Simulation Complete ===")
except FileNotFoundError:
    print("Error: The executable './bin/mpc' was not found. Check compilation.")
    sys.exit(1)
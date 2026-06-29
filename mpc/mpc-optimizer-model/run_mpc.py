"""
Compile and run the Lingua Franca MPC controller.

Usage:
    python run_mpc.py
"""
import subprocess
import sys
import os

LF_PROJECT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'mpc-lf-main')

print("=== Step 1: Compiling Lingua Franca Code ===")
print(f"    Source: {os.path.join(LF_PROJECT_DIR, 'mpc.lf')}")
print(f"    Solver: {os.path.join(LF_PROJECT_DIR, 'mpc_solver.c')}")

compile_process = subprocess.run(
    ['lfc', 'mpc.lf'],
    cwd=LF_PROJECT_DIR,
    capture_output=True,
    text=True
)

if compile_process.returncode != 0:
    print("\nCompilation Failed! Error log:")
    print(compile_process.stderr)
    sys.exit(1)

print("Success! Binary generated.\n")
print("=== Step 2: Executing Robot Simulation ===\n")

try:
    subprocess.run(
        ['./bin/mpc'],
        cwd=LF_PROJECT_DIR
    )
    print("\n=== Simulation Complete ===")
except FileNotFoundError:
    print("Error: The executable './bin/mpc' was not found. Check compilation.")
    sys.exit(1)

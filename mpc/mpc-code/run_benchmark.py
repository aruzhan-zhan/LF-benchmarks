import subprocess
import sys

print("=== Step 1: Compiling Lingua Franca Code ===")
# This runs 'lfc mpc.lf' in the background
compile_process = subprocess.run(['lfc', 'mpc.lf'], capture_output=True, text=True)

if compile_process.returncode != 0:
    print("Compilation Failed! Error log:")
    print(compile_process.stderr)
    sys.exit(1)
    
print("Success! Binary generated.\n")
print("=== Step 2: Executing Robot Simulation ===")

# This runs './bin/mpc' and streams the output directly to your console
try:
    # By not capturing the output, Python lets the LF executable print natively to the terminal
    subprocess.run(['./bin/mpc'])
    print("\n=== Simulation Complete ===")
except FileNotFoundError:
    print("Error: The executable './bin/mpc' was not found. Check compilation.")
    sys.exit(1)

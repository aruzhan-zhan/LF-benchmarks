import subprocess
import matplotlib.pyplot as plt

print("Step 1: Compiling Lingua Franca Code")
# This runs 'lfc mpc.lf' in the background
compile_process = subprocess.run(['lfc', 'mpc.lf'], capture_output=True, text=True)

if compile_process.returncode != 0:
    print("Compilation Failed! Error log:")
    print(compile_process.stderr)
    exit(1)
print("Success! Binary generated.")

print("\nStep 2: Executing Robot Simulation")
# This runs './bin/mpc' and captures all the printed text
run_process = subprocess.run(['./bin/mpc'], capture_output=True, text=True)
output = run_process.stdout

print("Simulation finished. Analyzing data...")

# Prepare lists to hold our data points
times = []
positions = []
velocities = []
motor_commands = []

# Step 3: Parse the Output
for line in output.split('\n'):
    if "[actuate]" in line:
        try:
            # We split the line by the '|' character and extract the numbers
            parts = line.split('|')
            
            # Example: "[actuate] tick= 1500 " -> extracts 1500
            tick = float(parts[0].split('=')[1].strip())
            pos  = float(parts[1].split('=')[1].strip())
            vel  = float(parts[2].split('=')[1].strip())
            u    = float(parts[3].split('=')[1].strip())
            
            # Convert ticks (milliseconds) to standard seconds for the graph
            times.append(tick / 1000.0)
            positions.append(pos)
            velocities.append(vel)
            motor_commands.append(u)
        except Exception as e:
            continue

print("\nStep 4: Generating Performance Graph")
# Create a large image with 3 stacked graphs
plt.figure(figsize=(10, 8))

# Graph 1: Position
plt.subplot(3, 1, 1)
plt.title("Robot Model Predictive Control (Lingua Franca)")
plt.plot(times, positions, label='Robot Position', color='blue', linewidth=2)
# Draw the goals the human requested with the remote control
plt.axhline(y=1.0, color='red', linestyle='--', alpha=0.5, label='Target: 1.0 (at 1s)')
plt.axhline(y=0.5, color='green', linestyle='--', alpha=0.5, label='Target: 0.5 (at 3s)')
plt.ylabel('Position (m)')
plt.legend()
plt.grid(True, alpha=0.3)

# Graph 2: Velocity
plt.subplot(3, 1, 2)
plt.plot(times, velocities, label='Robot Velocity', color='orange', linewidth=2)
plt.ylabel('Velocity (m/s)')
plt.legend()
plt.grid(True, alpha=0.3)

# Graph 3: Motor Voltage (u)
plt.subplot(3, 1, 3)
# We use 'step' instead of 'plot' because voltage is applied instantly, not gradually
plt.step(times, motor_commands, label='Motor Voltage (u)', color='purple', linewidth=2, where='post')
plt.xlabel('Time (seconds)')
plt.ylabel('Voltage (v)')
plt.legend()
plt.grid(True, alpha=0.3)

# Save the image cleanly
plt.tight_layout()
plt.savefig('robot_performance.png', dpi=300)
print("Done! Check your folder for 'robot_performance.png'.")
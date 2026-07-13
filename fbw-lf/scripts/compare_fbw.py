#!/usr/bin/env python3

import argparse
import csv
import math
import re
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--c-log", required=True)
parser.add_argument("--lf-log", required=True)
parser.add_argument("--out", default="fbw_comparison.csv")
parser.add_argument("--tolerance", type=float, default=1e-4)
args = parser.parse_args()

c_pattern = re.compile(r"^\s*(-?\d+)\s+([-+0-9.eE]+)\s*$")
lf_pattern = re.compile(r"^\s*LF_DATA\s+(-?\d+)\s+([-+0-9.eE]+)\s*$")


def read_values(filename, pattern):
    values = {}

    with open(filename, "r", encoding="utf-8", errors="replace") as file:
        for line in file:
            match = pattern.match(line)
            if match:
                counter = int(match.group(1))
                speed = float(match.group(2))
                values[counter] = speed

    return values


c_values = read_values(args.c_log, c_pattern)
lf_values = read_values(args.lf_log, lf_pattern)

if not c_values:
    print("ERROR: No C records were found.")
    sys.exit(2)

if not lf_values:
    print("ERROR: No LF_DATA records were found.")
    sys.exit(2)

c_counters = set(c_values)
lf_counters = set(lf_values)

common = sorted(c_counters & lf_counters)
c_only = sorted(c_counters - lf_counters)
lf_only = sorted(lf_counters - c_counters)

rows = []
errors = []
mismatches = []

for counter in common:
    c_speed = c_values[counter]
    lf_speed = lf_values[counter]
    error = abs(c_speed - lf_speed)
    passed = error <= args.tolerance

    rows.append([counter, c_speed, lf_speed, error, passed])
    errors.append(error)

    if not passed:
        mismatches.append((counter, c_speed, lf_speed, error))

with open(args.out, "w", newline="", encoding="utf-8") as file:
    writer = csv.writer(file)
    writer.writerow([
        "counter",
        "c_speed",
        "lf_speed",
        "absolute_error",
        "within_tolerance"
    ])
    writer.writerows(rows)

max_error = max(errors) if errors else float("nan")
mean_error = sum(errors) / len(errors) if errors else float("nan")
rmse = (
    math.sqrt(sum(error ** 2 for error in errors) / len(errors))
    if errors else float("nan")
)

print("=== Fly-By-Wire C vs LF Comparison ===")
print(f"C records:             {len(c_values)}")
print(f"LF records:            {len(lf_values)}")
print(f"Common counters:       {len(common)}")
print(f"C-only counters:       {len(c_only)}")
print(f"LF-only counters:      {len(lf_only)}")
print(f"Tolerance:             {args.tolerance:.1e}")
print(f"Maximum abs. error:    {max_error:.9g}")
print(f"Mean abs. error:       {mean_error:.9g}")
print(f"RMSE:                  {rmse:.9g}")
print(f"Mismatched samples:    {len(mismatches)}")
print(f"Detailed CSV:          {args.out}")

if mismatches:
    print("\nFirst mismatches:")
    for counter, c_speed, lf_speed, error in mismatches[:10]:
        print(
            f"counter={counter}: "
            f"C={c_speed:.9f}, "
            f"LF={lf_speed:.9f}, "
            f"error={error:.9g}"
        )

if not mismatches and not c_only and not lf_only:
    print("\nRESULT: PASS — all counters and speeds match.")
    sys.exit(0)

if not mismatches:
    print("\nRESULT: PARTIAL PASS — matching values, but counters differ.")
    sys.exit(1)

print("\nRESULT: FAIL — numerical mismatches detected.")
sys.exit(1)

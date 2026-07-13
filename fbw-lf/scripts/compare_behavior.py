#!/usr/bin/env python3
from pathlib import Path
import sys

def parse_c(path):
    result = {}
    expected = {}
    for line in Path(path).read_text().splitlines():
        parts = line.split()
        if len(parts) == 6 and parts[0] == "C_BEHAVIOR":
            scenario = parts[1]
            result[scenario] = parts[2]
            expected[scenario] = parts[4]
    return result, expected

def parse_lf(path):
    result = {}
    for line in Path(path).read_text().splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[0] == "LF_BEHAVIOR":
            result[parts[1]] = parts[2]
    return result

if len(sys.argv) != 3:
    print("Usage: compare_behavior.py <c_output> <lf_output>")
    raise SystemExit(2)

c_result, expected = parse_c(sys.argv[1])
lf_result = parse_lf(sys.argv[2])

scenarios = list(expected)
failures = 0

print("scenario                 original_C       expected_TMR      LF               C_vs_expected")
print("-" * 98)

for scenario in scenarios:
    c = c_result.get(scenario, "MISSING")
    exp = expected.get(scenario, "MISSING")
    lf = lf_result.get(scenario, "MISSING")
    relation = "MATCH" if c == exp else "MISMATCH"
    lf_ok = lf == exp

    print(
        f"{scenario:24} {c:16} {exp:16} {lf:16} {relation}"
        + ("" if lf_ok else "  <-- LF FAIL")
    )

    if not lf_ok:
        failures += 1

print()
if failures:
    print(f"RESULT: FAIL — LF disagreed with expected TMR in {failures} scenario(s).")
    raise SystemExit(1)

original_mismatches = sum(
    1 for scenario in scenarios
    if c_result.get(scenario) != expected.get(scenario)
)

print("RESULT: LF PASS — LF matched symmetric TMR behavior in every scenario.")
print(
    f"Original C WES disagreed with symmetric TMR in "
    f"{original_mismatches} scenario(s)."
)

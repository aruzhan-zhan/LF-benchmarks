#!/usr/bin/env python3

from pathlib import Path
import re
import sys

LOG_FILE = "e2e_fault_output.txt"

path = Path(LOG_FILE)

if not path.exists():
    print(f"ERROR: {LOG_FILE} does not exist.")
    sys.exit(2)

text = path.read_text(encoding="utf-8", errors="replace")
lines = text.splitlines()


def matching_lines(pattern):
    regex = re.compile(pattern)
    return [line for line in lines if regex.search(line)]


checks = [
    (
        "FMAN sends STOP to PFC2 at T=5",
        r"\[FMAN\s*\]\s*T=5 s\s*\|\s*TEST: STOP sent to PFC2",
    ),
    (
        "PFC2 receives and applies STOP",
        r"\[PFC2\s*\]\s*T=5 s\s*\|\s*STOPPED",
    ),
    (
        "WES identifies missing/stopped PFC2",
        r"\[WES\s*\]\s*T=(5|6|7) s\s*\|\s*ERROR: PFC2 discordant",
    ),
    (
        "PFCDS receives PFC2 fault status",
        r"\[PFCDS\s*\]\s*T=(5|6|7) s\s*\|\s*ERROR: PFC2 faulty",
    ),
    (
        "FMAN sends CONT to PFC2 at T=8",
        r"\[FMAN\s*\]\s*T=8 s\s*\|\s*TEST: CONT sent to PFC2",
    ),
    (
        "PFC2 receives and applies CONT",
        r"\[PFC2\s*\]\s*T=8 s\s*\|\s*RESUMED",
    ),
    (
        "WES returns to OK immediately after CONT",
        r"\[WES\s*\]\s*T=8 s\s*\|\s*OK:",
    ),
    (
        "FMAN sends BIAS to PFC3 at T=640",
        r"\[FMAN\s*\]\s*T=640 s\s*\|\s*TEST: BIAS sent to PFC3",
    ),
    (
        "PFC3 applies BIAS to transmitted sample",
        r"\[PFC3\s*\]\s*T=640 s\s*\|\s*BIAS applied:",
    ),
    (
        "WES identifies biased PFC3",
        r"\[WES\s*\]\s*T=640 s\s*\|\s*ERROR: PFC3 discordant",
    ),
    (
        "PFCDS receives PFC3 fault status",
        r"\[PFCDS\s*\]\s*T=640 s\s*\|\s*ERROR: PFC3 faulty",
    ),
    (
        "WES recovers on the next sample",
        r"\[WES\s*\]\s*T=641 s\s*\|\s*OK:",
    ),
]

print("=== LF End-to-End Fault Validation ===\n")

failed = 0

for description, pattern in checks:
    matches = matching_lines(pattern)

    if matches:
        print(f"PASS: {description}")
        print(f"      {matches[0]}")
    else:
        print(f"FAIL: {description}")
        failed += 1

emergencies = matching_lines(r"\bEMERGENCY\b")

print()
if emergencies:
    print("FAIL: Unexpected EMERGENCY occurred:")
    for line in emergencies[:10]:
        print(f"      {line}")
    failed += 1
else:
    print("PASS: No unexpected EMERGENCY occurred")

print("\n=== Summary ===")
print(f"Checks passed: {len(checks) + 1 - failed}")
print(f"Checks failed: {failed}")

if failed == 0:
    print(
        "\nRESULT: PASS — STOP, CONT, BIAS, WES voting, "
        "PFCDS reporting, and recovery all worked end to end."
    )
    sys.exit(0)

print("\nRESULT: FAIL — one or more end-to-end checks failed.")
sys.exit(1)

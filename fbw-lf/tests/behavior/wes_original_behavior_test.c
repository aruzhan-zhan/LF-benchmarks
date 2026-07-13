#include <stdio.h>
#include <string.h>

typedef struct {
    int counter;
    double speed;
} sample_t;

/*
 * Exact decision structure from the original Wabri wes.c.
 * Labels use PFC numbers rather than process IDs.
 */
static const char* original_wes(sample_t p1, sample_t p2, sample_t p3) {
    if ((p1.counter == p2.counter) && (p1.speed == p2.speed)) {
        if ((p1.counter == p3.counter) && (p1.speed == p3.speed)) {
            return "OK";
        } else {
            return "ERROR_PFC3";
        }
    } else if (p1.counter == p3.counter) {
        return "ERROR_PFC2";
    } else if (p2.counter == p3.counter) {
        return "ERROR_PFC1";
    } else {
        return "EMERGENCY";
    }
}

/* Symmetric 2-of-3 voting used by the corrected LF implementation. */
static const char* expected_tmr(sample_t p1, sample_t p2, sample_t p3) {
    int match12 =
        p1.counter == p2.counter && p1.speed == p2.speed;
    int match13 =
        p1.counter == p3.counter && p1.speed == p3.speed;
    int match23 =
        p2.counter == p3.counter && p2.speed == p3.speed;

    if (match12 && match13) return "OK";
    if (match12) return "ERROR_PFC3";
    if (match13) return "ERROR_PFC2";
    if (match23) return "ERROR_PFC1";
    return "EMERGENCY";
}

typedef struct {
    const char* name;
    sample_t p1;
    sample_t p2;
    sample_t p3;
} scenario_t;

int main(void) {
    const scenario_t scenarios[] = {
        {
            "all_agree",
            {10, 5.0}, {10, 5.0}, {10, 5.0}
        },
        {
            "pfc3_bias",
            {11, 5.0}, {11, 5.0}, {11, 20.0}
        },
        {
            "pfc2_stale",
            {12, 5.0}, {11, 5.0}, {12, 5.0}
        },
        {
            "recovered",
            {13, 6.0}, {13, 6.0}, {13, 6.0}
        },
        {
            "pfc1_stale",
            {13, 6.0}, {14, 6.0}, {14, 6.0}
        },
        {
            "different_counters",
            {15, 7.0}, {14, 7.0}, {13, 7.0}
        },
        {
            "pfc1_bias",
            {16, 28.0}, {16, 7.0}, {16, 7.0}
        },
        {
            "pfc2_bias",
            {17, 7.0}, {17, 28.0}, {17, 7.0}
        },
        {
            "all_speeds_different",
            {18, 7.0}, {18, 8.0}, {18, 9.0}
        }
    };

    const int count = (int)(sizeof(scenarios) / sizeof(scenarios[0]));

    for (int i = 0; i < count; ++i) {
        const char* original =
            original_wes(scenarios[i].p1, scenarios[i].p2, scenarios[i].p3);
        const char* expected =
            expected_tmr(scenarios[i].p1, scenarios[i].p2, scenarios[i].p3);

        printf(
            "C_BEHAVIOR %s %s EXPECTED %s %s\n",
            scenarios[i].name,
            original,
            expected,
            strcmp(original, expected) == 0 ? "MATCH" : "MISMATCH"
        );
    }

    return 0;
}

#ifndef FBW_NAV_H
#define FBW_NAV_H

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Constants */
#define EARTH_RADIUS_KM  6371
#define DELTA_SEC        1
#define GPGLL_SENTENCE   "$GPGLL"

/* Fault types (replaces POSIX signals) */
#define FAULT_NONE 0
#define FAULT_STOP 1
#define FAULT_CONT 2
#define FAULT_INT  3
#define FAULT_BIAS 4

/* WES voting result */
#define STATUS_OK          0
#define STATUS_ERROR_PFC1  1
#define STATUS_ERROR_PFC2  2
#define STATUS_ERROR_PFC3  3
#define STATUS_EMERGENCY   4
#define STATUS_EXIT        5

/* Parsed GPGLL fix */
typedef struct {
    double latitude;
    char   lat_dir;
    double longitude;
    char   lon_dir;
    int    fix_time;
    int    valid;
} gll_fix_t;

/* Speed output sent from PFC to Transducer */
typedef struct {
    int    counter;
    double speed;
    int    valid;
} speed_msg_t;

/* Convert degrees to radians */
static inline double degrees_to_radians(double deg) {
    return deg * M_PI / 180.0;
}

/* Parse a GPGLL sentence from a raw NMEA line */
static int parse_gpgll(const char* line, gll_fix_t* fix) {
    if (strncmp(line, GPGLL_SENTENCE, strlen(GPGLL_SENTENCE)) != 0) {
        fix->valid = 0;
        return 0;
    }
    char lat_dir, lon_dir;
    double lat, lon;
    int time_val;
    char rest[32];
    int n = sscanf(line, "$GPGLL,%lf,%c,%lf,%c,%d,%31s",
                   &lat, &lat_dir, &lon, &lon_dir, &time_val, rest);
    if (n < 5) {
        fix->valid = 0;
        return 0;
    }
    fix->latitude  = lat;
    fix->lat_dir   = lat_dir;
    fix->longitude = lon;
    fix->lon_dir   = lon_dir;
    fix->fix_time  = time_val;
    fix->valid     = 1;
    return 1;
}

/* Haversine distance between two GPS fixes (in km) */
static double haversine_distance(const gll_fix_t* start, const gll_fix_t* end) {
    double d_lat = degrees_to_radians(end->latitude - start->latitude);
    double d_lon = degrees_to_radians(end->longitude - start->longitude);
    double s_lat = degrees_to_radians(start->latitude);
    double e_lat = degrees_to_radians(end->latitude);
    double ha = pow(sin(d_lat / 2.0), 2) +
                pow(sin(d_lon / 2.0), 2) * cos(s_lat) * cos(e_lat);
    double hc = 2.0 * atan2(sqrt(ha), sqrt(1.0 - ha));
    return EARTH_RADIUS_KM * hc;
}

/* Compute instantaneous speed from two consecutive fixes */
static double compute_speed(double prev_speed,
                            const gll_fix_t* prev_fix,
                            const gll_fix_t* curr_fix) {
    double dist = haversine_distance(prev_fix, curr_fix);
    return prev_speed + dist / DELTA_SEC;
}

/* Apply SIGUSR1 bias: left-shift speed by 2 bits */
static double apply_bias(double speed) {
    return (double)((int)round(speed) << 2);
}

#endif /* FBW_NAV_H */

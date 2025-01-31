/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6 */

#ifndef PB_ESPTELEMETRY_PROTO_FILES_TELEMETRY_PB_H_INCLUDED
#define PB_ESPTELEMETRY_PROTO_FILES_TELEMETRY_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _ESPTelemetry_data { 
    float currentADC;
    float voltsEVADC;
    uint32_t ambientTempADC;
    float backupVoltADC;
    float pitchMPU;
    float rollMPU;
    float capacity;
    float voltage;
    float current;
    float remaining;
    uint32_t state;
    float cell01;
    float cell02;
    float cell03;
    float cell04;
    float cell05;
    float cell06;
    float cell07;
    float cell08;
    float cell09;
    float cell10;
    float cell11;
    float cell12;
    float cell13;
    uint32_t temperature1;
    uint32_t temperature2;
    uint32_t temperature3;
    uint32_t temperature4;
    uint32_t temperature5;
    uint32_t temperature6;
} ESPTelemetry_data;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define ESPTelemetry_data_init_default           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define ESPTelemetry_data_init_zero              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define ESPTelemetry_data_currentADC_tag         1
#define ESPTelemetry_data_voltsEVADC_tag         2
#define ESPTelemetry_data_ambientTempADC_tag     3
#define ESPTelemetry_data_backupVoltADC_tag      4
#define ESPTelemetry_data_pitchMPU_tag           5
#define ESPTelemetry_data_rollMPU_tag            6
#define ESPTelemetry_data_capacity_tag           7
#define ESPTelemetry_data_voltage_tag            8
#define ESPTelemetry_data_current_tag            9
#define ESPTelemetry_data_remaining_tag          10
#define ESPTelemetry_data_state_tag              11
#define ESPTelemetry_data_cell01_tag             16
#define ESPTelemetry_data_cell02_tag             17
#define ESPTelemetry_data_cell03_tag             18
#define ESPTelemetry_data_cell04_tag             19
#define ESPTelemetry_data_cell05_tag             20
#define ESPTelemetry_data_cell06_tag             21
#define ESPTelemetry_data_cell07_tag             22
#define ESPTelemetry_data_cell08_tag             23
#define ESPTelemetry_data_cell09_tag             24
#define ESPTelemetry_data_cell10_tag             25
#define ESPTelemetry_data_cell11_tag             26
#define ESPTelemetry_data_cell12_tag             27
#define ESPTelemetry_data_cell13_tag             28
#define ESPTelemetry_data_temperature1_tag       29
#define ESPTelemetry_data_temperature2_tag       30
#define ESPTelemetry_data_temperature3_tag       31
#define ESPTelemetry_data_temperature4_tag       32
#define ESPTelemetry_data_temperature5_tag       33
#define ESPTelemetry_data_temperature6_tag       34

/* Struct field encoding specification for nanopb */
#define ESPTelemetry_data_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, FLOAT,    currentADC,        1) \
X(a, STATIC,   SINGULAR, FLOAT,    voltsEVADC,        2) \
X(a, STATIC,   SINGULAR, UINT32,   ambientTempADC,    3) \
X(a, STATIC,   SINGULAR, FLOAT,    backupVoltADC,     4) \
X(a, STATIC,   SINGULAR, FLOAT,    pitchMPU,          5) \
X(a, STATIC,   SINGULAR, FLOAT,    rollMPU,           6) \
X(a, STATIC,   SINGULAR, FLOAT,    capacity,          7) \
X(a, STATIC,   SINGULAR, FLOAT,    voltage,           8) \
X(a, STATIC,   SINGULAR, FLOAT,    current,           9) \
X(a, STATIC,   SINGULAR, FLOAT,    remaining,        10) \
X(a, STATIC,   SINGULAR, UINT32,   state,            11) \
X(a, STATIC,   SINGULAR, FLOAT,    cell01,           16) \
X(a, STATIC,   SINGULAR, FLOAT,    cell02,           17) \
X(a, STATIC,   SINGULAR, FLOAT,    cell03,           18) \
X(a, STATIC,   SINGULAR, FLOAT,    cell04,           19) \
X(a, STATIC,   SINGULAR, FLOAT,    cell05,           20) \
X(a, STATIC,   SINGULAR, FLOAT,    cell06,           21) \
X(a, STATIC,   SINGULAR, FLOAT,    cell07,           22) \
X(a, STATIC,   SINGULAR, FLOAT,    cell08,           23) \
X(a, STATIC,   SINGULAR, FLOAT,    cell09,           24) \
X(a, STATIC,   SINGULAR, FLOAT,    cell10,           25) \
X(a, STATIC,   SINGULAR, FLOAT,    cell11,           26) \
X(a, STATIC,   SINGULAR, FLOAT,    cell12,           27) \
X(a, STATIC,   SINGULAR, FLOAT,    cell13,           28) \
X(a, STATIC,   SINGULAR, UINT32,   temperature1,     29) \
X(a, STATIC,   SINGULAR, UINT32,   temperature2,     30) \
X(a, STATIC,   SINGULAR, UINT32,   temperature3,     31) \
X(a, STATIC,   SINGULAR, UINT32,   temperature4,     32) \
X(a, STATIC,   SINGULAR, UINT32,   temperature5,     33) \
X(a, STATIC,   SINGULAR, UINT32,   temperature6,     34)
#define ESPTelemetry_data_CALLBACK NULL
#define ESPTelemetry_data_DEFAULT NULL

extern const pb_msgdesc_t ESPTelemetry_data_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define ESPTelemetry_data_fields &ESPTelemetry_data_msg

/* Maximum encoded size of messages (where known) */
#define ESPTelemetry_data_size                   177

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

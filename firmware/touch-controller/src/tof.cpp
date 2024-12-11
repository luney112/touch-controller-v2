// #include <Arduino.h>
// #include <Wire.h>
// #include <vl53l7cx_class.h>

// #define LPN_PIN 12
// #define I2C_RST_PIN 14
// #define PWREN_PIN 13
// #define INT_PIN 19

// void measure(void);
// void print_result(VL53L7CX_ResultsData *Result);

// // Component.
// VL53L7CX sensor_vl53l7cx_top(&Wire, LPN_PIN, I2C_RST_PIN);

// bool EnableAmbient = false;
// bool EnableSignal = false;
// uint8_t res = VL53L7CX_RESOLUTION_4X4;
// char report[256];
// volatile int interruptCount = 0;
// uint8_t i;

// /* Setup ---------------------------------------------------------------------*/

// void setup() {

//   VL53L7CX_DetectionThresholds thresholds[VL53L7CX_NB_THRESHOLDS];

//   // Enable PWREN pin if present
//   if (PWREN_PIN >= 0) {
//     pinMode(PWREN_PIN, OUTPUT);
//     digitalWrite(PWREN_PIN, HIGH);
//     delay(10);
//   }

//   // Initialize serial for output.
//   Serial.begin(460800);

//   // Initialize I2C bus.
//   Wire.begin();

//   // Set interrupt pin
//   pinMode(INT_PIN, INPUT_PULLUP);
//   attachInterrupt(INT_PIN, measure, FALLING);

//   // Configure VL53L7CX component.
//   sensor_vl53l7cx_top.begin();

//   sensor_vl53l7cx_top.init_sensor();

//   // Disable thresholds detection.
//   sensor_vl53l7cx_top.vl53l7cx_set_detection_thresholds_enable(0U);

//   // Set all values to 0.
//   memset(&thresholds, 0, sizeof(thresholds));

//   // Configure thresholds on each active zone
//   for (i = 0; i < res; i++) {
//     thresholds[i].zone_num = i;
//     thresholds[i].measurement = VL53L7CX_DISTANCE_MM;
//     thresholds[i].type = VL53L7CX_IN_WINDOW;
//     thresholds[i].mathematic_operation = VL53L7CX_OPERATION_NONE;
//     thresholds[i].param_low_thresh = 80;
//     thresholds[i].param_high_thresh = 120;
//   }

//   // Last threshold must be clearly indicated.
//   thresholds[i].zone_num |= VL53L7CX_LAST_THRESHOLD;

//   // Send array of thresholds to the sensor.
//   sensor_vl53l7cx_top.vl53l7cx_set_detection_thresholds(thresholds);

//   // Enable thresholds detection.
//   sensor_vl53l7cx_top.vl53l7cx_set_detection_thresholds_enable(1U);

//   // Increase poll freq
//   sensor_vl53l7cx_top.vl53l7cx_set_ranging_frequency_hz(30);

//   sensor_vl53l7cx_top.vl53l7cx_set_ranging_mode(VL53L7CX_RANGING_MODE_CONTINUOUS);

//   // Start Measurements.
//   sensor_vl53l7cx_top.vl53l7cx_start_ranging();
// }

// void loop() {
//   VL53L7CX_ResultsData Results;
//   uint8_t NewDataReady = 0;
//   uint8_t status;

//   do {
//     status = sensor_vl53l7cx_top.vl53l7cx_check_data_ready(&NewDataReady);
//   } while (!NewDataReady);

//   if ((!status) && (NewDataReady != 0) && interruptCount) {
//     interruptCount = 0;
//     status = sensor_vl53l7cx_top.vl53l7cx_get_ranging_data(&Results);
//     print_result(&Results);
//   }
// }

// void print_result(VL53L7CX_ResultsData *Result) {
//   int8_t i, j, k, l;
//   uint8_t zones_per_line;
//   uint8_t number_of_zones = res;

//   zones_per_line = (number_of_zones == 16) ? 4 : 8;

//   snprintf(report, sizeof(report), "%c[2H", 27); /* 27 is ESC command */
//   Serial.print(report);
//   Serial.print("53L7A1 Threshold Detection demo application\n");
//   Serial.print("-------------------------------------------\n\n");
//   Serial.print("Cell Format :\n\n");

//   for (l = 0; l < VL53L7CX_NB_TARGET_PER_ZONE; l++) {
//     snprintf(report, sizeof(report), " \033[38;5;10m%20s\033[0m : %20s\n", "Distance [mm]", "Status");
//     Serial.print(report);

//     if (EnableAmbient || EnableSignal) {
//       snprintf(report, sizeof(report), " %20s : %20s\n", "Signal [kcps/spad]", "Ambient [kcps/spad]");
//       Serial.print(report);
//     }
//   }

//   Serial.print("\n\n");

//   for (j = 0; j < number_of_zones; j += zones_per_line) {
//     for (i = 0; i < zones_per_line; i++)
//       Serial.print(" -----------------");
//     Serial.print("\n");

//     for (i = 0; i < zones_per_line; i++)
//       Serial.print("|                 ");
//     Serial.print("|\n");

//     for (l = 0; l < VL53L7CX_NB_TARGET_PER_ZONE; l++) {
//       // Print distance and status.
//       for (k = (zones_per_line - 1); k >= 0; k--) {
//         if (Result->nb_target_detected[j + k] > 0) {
//           snprintf(report, sizeof(report), "| \033[38;5;10m%5ld\033[0m  :  %5ld ",
//                    (long)Result->distance_mm[(VL53L7CX_NB_TARGET_PER_ZONE * (j + k)) + l],
//                    (long)Result->target_status[(VL53L7CX_NB_TARGET_PER_ZONE * (j + k)) + l]);
//           Serial.print(report);
//         } else {
//           snprintf(report, sizeof(report), "| %5s  :  %5s ", "X", "X");
//           Serial.print(report);
//         }
//       }
//       Serial.print("|\n");

//       if (EnableAmbient || EnableSignal) {
//         // Print Signal and Ambient.
//         for (k = (zones_per_line - 1); k >= 0; k--) {
//           if (Result->nb_target_detected[j + k] > 0) {
//             if (EnableSignal) {
//               snprintf(report, sizeof(report),
//                        "| %5ld  :  ", (long)Result->signal_per_spad[(VL53L7CX_NB_TARGET_PER_ZONE * (j + k)) + l]);
//               Serial.print(report);
//             } else {
//               snprintf(report, sizeof(report), "| %5s  :  ", "X");
//               Serial.print(report);
//             }
//             if (EnableAmbient) {
//               snprintf(report, sizeof(report), "%5ld ", (long)Result->ambient_per_spad[j + k]);
//               Serial.print(report);
//             } else {
//               snprintf(report, sizeof(report), "%5s ", "X");
//               Serial.print(report);
//             }
//           } else {
//             snprintf(report, sizeof(report), "| %5s  :  %5s ", "X", "X");
//             Serial.print(report);
//           }
//         }
//         Serial.print("|\n");
//       }
//     }
//   }
//   for (i = 0; i < zones_per_line; i++)
//     Serial.print(" -----------------");
//   Serial.print("\n");
// }

// void measure(void) {
//   interruptCount = 1;
// }

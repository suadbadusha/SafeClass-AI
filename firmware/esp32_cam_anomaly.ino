/**
 * SafeClass AI – ESP32-CAM firmware (4-label model)
 * Labels expected exactly:
 *  - "Normal classroom"
 *  - "Distracted classroom"
 *  - "Empty classroom"
 *  - "Unusual behaviour"
 *
 * Requirements:
 *  - Edge Impulse Arduino library (with your exported model)
 *  - ESP32 board support installed in Arduino IDE
 *  - Board: AI Thinker ESP32-CAM
 *
 * Notes:
 *  - Do NOT stream or store frames. Only on-device inference.
 *  - Adjust thresholds and timing logic below as you gather more data.
 */

 #include "esp_camera.h"
 #include <WiFi.h>
 
 // Edge Impulse includes (install the Arduino library exported from Edge Impulse)
 #include <edge_impulse/edge-impulse-sdk/classifier/ei_run_classifier.h>
 
 // Camera pins for AI Thinker ESP32-CAM
 #define PWDN_GPIO_NUM     32
 #define RESET_GPIO_NUM    -1
 #define XCLK_GPIO_NUM      0
 #define SIOD_GPIO_NUM     26
 #define SIOC_GPIO_NUM     27
 #define Y9_GPIO_NUM       35
 #define Y8_GPIO_NUM       34
 #define Y7_GPIO_NUM       39
 #define Y6_GPIO_NUM       36
 #define Y5_GPIO_NUM       21
 #define Y4_GPIO_NUM       19
 #define Y3_GPIO_NUM       18
 #define Y2_GPIO_NUM        5
 #define VSYNC_GPIO_NUM    25
 #define HREF_GPIO_NUM     23
 #define PCLK_GPIO_NUM     22
 
 // Output pins (optional)
 #define BUZZER_PIN         4
 #define STATUS_LED_PIN    33
 
 // WiFi (optional, used here for sending alerts later)
 const char* ssid     = "Suad";
 const char* password = "12345678";
 
 // Thresholds (tune later with validation data)
 const float UNUSUAL_THRESHOLD = 0.70f;   // label "Unusual behaviour" threshold -> immediate anomaly
 const float DISTRACTED_THRESHOLD = 0.65f; // "Distracted classroom" -> warning
 const float EMPTY_THRESHOLD = 0.70f;     // "Empty classroom" -> warning (use with schedule)
 
 // Edge Impulse input size constants come from the model headers
 static const int EI_INPUT_WIDTH  = EI_CLASSIFIER_INPUT_WIDTH;
 static const int EI_INPUT_HEIGHT = EI_CLASSIFIER_INPUT_HEIGHT;
 static const int EI_INPUT_SIZE   = EI_INPUT_WIDTH * EI_INPUT_HEIGHT;
 
 // Image buffer to hold grayscale pixels for EI
 static signed char ei_image_buffer[EI_INPUT_SIZE];
 
 // helper for EI signal
 int ei_get_frame_data(size_t offset, size_t length, float *out_ptr) {
   for (size_t i = 0; i < length; i++) {
     out_ptr[i] = (ei_image_buffer[offset + i]) / 255.0f;
   }
   return 0;
 }
 
 void setup() {
   Serial.begin(115200);
   delay(2000);
 
   pinMode(BUZZER_PIN, OUTPUT);
   pinMode(STATUS_LED_PIN, OUTPUT);
   digitalWrite(BUZZER_PIN, LOW);
   digitalWrite(STATUS_LED_PIN, LOW);
 
   Serial.println("SafeClass AI (4-label) starting...");
 
   // Camera init
   camera_config_t config;
   config.ledc_channel = LEDC_CHANNEL_0;
   config.ledc_timer   = LEDC_TIMER_0;
   config.pin_d0       = Y2_GPIO_NUM;
   config.pin_d1       = Y3_GPIO_NUM;
   config.pin_d2       = Y4_GPIO_NUM;
   config.pin_d3       = Y5_GPIO_NUM;
   config.pin_d4       = Y6_GPIO_NUM;
   config.pin_d5       = Y7_GPIO_NUM;
   config.pin_d6       = Y8_GPIO_NUM;
   config.pin_d7       = Y9_GPIO_NUM;
   config.pin_xclk     = XCLK_GPIO_NUM;
   config.pin_pclk     = PCLK_GPIO_NUM;
   config.pin_vsync    = VSYNC_GPIO_NUM;
   config.pin_href     = HREF_GPIO_NUM;
   config.pin_sscb_sda = SIOD_GPIO_NUM;
   config.pin_sscb_scl = SIOC_GPIO_NUM;
   config.pin_pwdn     = PWDN_GPIO_NUM;
   config.pin_reset    = RESET_GPIO_NUM;
   config.xclk_freq_hz = 20000000;
   config.pixel_format = PIXFORMAT_GRAYSCALE; // easier input for EI
 
   // Use small frame size for performance; EI will expect a specific input resolution.
   config.frame_size   = FRAMESIZE_QQVGA; // 160x120
   config.jpeg_quality = 12;
   config.fb_count     = 1;
 
   esp_err_t err = esp_camera_init(&config);
   if (err != ESP_OK) {
     Serial.printf("Camera init failed: 0x%x\n", err);
     while (true) delay(1000);
   }
   Serial.println("Camera initialized.");
 
   // Optional Wi-Fi connect (comment out if not needed)
   WiFi.begin(ssid, password);
   Serial.print("Connecting to WiFi");
   unsigned long tstart = millis();
   while ((WiFi.status() != WL_CONNECTED) && (millis() - tstart < 8000)) {
     delay(300);
     Serial.print(".");
   }
   if (WiFi.status() == WL_CONNECTED) {
     Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
   } else {
     Serial.println("\nWiFi not connected. Continuing in offline mode.");
   }
 }
 
 void loop() {
   // 1) Capture & resize into ei_image_buffer
   if (!capture_resized_grayscale()) {
     Serial.println("Capture failed. Retrying...");
     delay(1000);
     return;
   }
 
   // 2) Prepare signal for EI
   ei::signal_t signal;
   signal.total_length = EI_INPUT_SIZE;
   signal.get_data = &ei_get_frame_data;
 
   // 3) Run classifier
   ei_impulse_result_t result = {0};
   EI_IMPULSE_ERROR r = run_classifier(&signal, &result, false);
   if (r != EI_IMPULSE_OK) {
     Serial.printf("run_classifier error: %d\n", r);
     delay(1000);
     return;
   }
 
   // 4) Print all prediction scores
   Serial.println("Prediction results:");
   for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
     Serial.printf("  %s : %.3f\n", result.classification[i].label, result.classification[i].value);
   }
 
   // 5) Find best label
   int best_idx = -1;
   float best_val = 0.0f;
   for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
     float v = result.classification[i].value;
     if (v > best_val) { best_val = v; best_idx = i; }
   }
 
   const char* label = result.classification[best_idx].label;
   Serial.printf("Top label: %s (%.3f)\n", label, best_val);
 
   // 6) Decision logic (tunable)
   if (strcmp(label, "Unusual behaviour") == 0 && best_val >= UNUSUAL_THRESHOLD) {
     // immediate anomaly: fighting, bullying, or unusual movement
     alert_anomaly(label, best_val);
   } else if (strcmp(label, "Distracted classroom") == 0 && best_val >= DISTRACTED_THRESHOLD) {
     // warning: teacher attention recommended
     warning_action(label, best_val);
   } else if (strcmp(label, "Empty classroom") == 0 && best_val >= EMPTY_THRESHOLD) {
     // empty classroom: treat as a warning for times when class should be occupied
     warning_action(label, best_val);
   } else {
     // Normal classroom or low-confidence predictions
     clear_alert();
   }
 
   Serial.println("-----------------------\n");
   delay(1500); // capture interval - tune to your needs
 }
 
 void alert_anomaly(const char* lbl, float conf) {
   Serial.printf(">>> ANOMALY: %s (%.2f) - immediate alert!\n", lbl, conf);
   digitalWrite(STATUS_LED_PIN, HIGH);
   tone(BUZZER_PIN, 2500, 600); // beep for 600ms
   // TODO: add Wi-Fi HTTP/MQTT alert code here (send JSON with label, conf, timestamp)
 }
 
 void warning_action(const char* lbl, float conf) {
   Serial.printf(">> WARNING: %s (%.2f) - suggest teacher check\n", lbl, conf);
   // shorter LED blink or soft beep
   digitalWrite(STATUS_LED_PIN, HIGH);
   tone(BUZZER_PIN, 1500, 200);
   delay(300);
   digitalWrite(STATUS_LED_PIN, LOW);
 }
 
 void clear_alert() {
   digitalWrite(STATUS_LED_PIN, LOW);
   // No buzzer
 }
 
 /**
  * Capture and resize to model input using nearest neighbour.
  * Assumes fb is grayscale one byte per pixel.
  */
 bool capture_resized_grayscale() {
   camera_fb_t *fb = esp_camera_fb_get();
   if (!fb) return false;
 
   int src_w = fb->width;
   int src_h = fb->height;
   uint8_t* src = fb->buf;
 
   // If the camera output matches EI input dimensions, copy directly
   if (src_w == EI_INPUT_WIDTH && src_h == EI_INPUT_HEIGHT) {
     for (int i = 0; i < EI_INPUT_SIZE; ++i) {
       ei_image_buffer[i] = (signed char)src[i];
     }
     esp_camera_fb_return(fb);
     return true;
   }
 
   // Otherwise, nearest-neighbour downscale
   for (int y = 0; y < EI_INPUT_HEIGHT; y++) {
     for (int x = 0; x < EI_INPUT_WIDTH; x++) {
       int sx = (x * src_w) / EI_INPUT_WIDTH;
       int sy = (y * src_h) / EI_INPUT_HEIGHT;
       uint8_t pix = src[sy * src_w + sx];
       ei_image_buffer[y * EI_INPUT_WIDTH + x] = (signed char)pix;
     }
   }
 
   esp_camera_fb_return(fb);
   return true;
 }
 

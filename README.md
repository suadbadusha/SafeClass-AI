# SafeClass AI — Behavioural Anomaly Detection (early-stage)

**A tiny-ML proof-of-concept that runs on the ESP32-CAM and classifies classroom scenes into:**
1. Normal classroom  
2. Distracted classroom  
3. Empty classroom  
4. Unusual behaviour

> Note: This is an early-stage hackathon project. I have just started this idea and I intend to expand it — collect more data, improve labels, reduce bias, and add a dashboard & multi-node deployment. This submission demonstrates a working on-device classifier and a prototype alerting mechanism.

## What is included
- `firmware/esp32_cam_anomaly.ino` — main Arduino sketch (see instructions)
- Documentation: `docs/` (Problem statement, system design, dataset, ethics, next steps)
- `edge_impulse/` — placeholder for label file & impulse notes
- `assets/` — sample frames and examples you used for training

## Quick start (flash device)
1. Train and export an **Arduino library** from Edge Impulse for your 4-label model.
2. In Arduino IDE:  
   - Install ESP32 board support.  
   - Add the Edge Impulse .zip library (Sketch → Include Library → Add .ZIP Library).  
   - Open `firmware/esp32_cam_anomaly.ino`.  
   - Select **AI Thinker ESP32-CAM** board and correct upload settings.
3. Update Wi-Fi details (optional) in the sketch.
4. Upload to the device. Open Serial Monitor at 115200 baud and observe predictions.
5. Place the device to view a classroom/corridor. Test the four scenarios and tune thresholds.

## How this is intended to grow
- Add more labelled data for each class (diversity of lighting, ages, classroom layouts).
- Add time-of-day & schedule-awareness for `Empty classroom`.
- Add a small web dashboard or MQTT integration to collect events centrally.
- Add fine-grained action detection (non-contact vs contact physical aggression).
- Improve model with more training, augmentation, and possibly lightweight object detection.

## Ethics & privacy
- On-device inference only — no images are sent to cloud.
- No face recognition or identity tracking.
- School should inform students/parents and use system for safety not punishment.

# Project status & intentions

This project is an early prototype built for a hackathon. I started with the idea of privacy-preserving behaviour monitoring for schools and created a minimal working prototype:

- A 4-label image classification model, trained in Edge Impulse.
- On-device inference running on an ESP32-CAM with simple alert logic.

This is **small** and experimental. My goals after the hackathon:
- Expand dataset (more classes, more environments).
- Improve the model and reduce false positives.
- Add scheduled context (which classes should be occupied).
- Build a central dashboard and multi-camera deployment pipeline.
- Conduct user testing in a school environment with consent.


# How to improve & extend

1. **Data**  
   - Gather 1000+ labelled images per class across multiple schools, ages, lighting.  
   - Use balanced sampling to avoid class imbalance.  
   - Add augmentation (brightness, rotation, blur) in Edge Impulse.

2. **Labels & sublabels**  
   - Expand `Unusual behaviour` to sublabels: fighting vs running vs climbing.  
   - Add `Teacher on break` or `Maintenance` to reduce false positives.

3. **Model improvements**  
   - Use transfer learning with MobileNetV2 (Edge Impulse transfer learning).  
   - Try quantized int8 model for size optimization.

4. **Evaluation**  
   - Keep a test set from different schools and compute precision/recall per class.  
   - Tune thresholds to maximize precision on `Unusual behaviour` (reduce false alarms).

5. **Deployment**  
   - Add MQTT/HTTP event sending to a central server with minimal metadata only (label, timestamp, device id).  
   - Implement secure channel (TLS) if needed.

6. **Ethics & governance**  
   - Get written parental consent for cameras in classrooms.  
   - Ensure alerts are always reviewed by a human; do not automate punishments.

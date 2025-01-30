#!/usr/bin/python3

import cv2
from ultralytics import YOLO
import cvzone
from mqtt_pub import MQTTPublisher

class TrafficScenario:
    def __init__(self, label):
        self.label = label

    def annotate_frame(self, frame, detections):
        for detection in detections:
            x1, y1, x2, y2, label, confidence = detection  
            color = (0, 255, 0) if confidence >= 0.70 else (0, 0, 255)
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, 2)
            cvzone.putTextRect(frame, label, (x1, y1), scale=1, thickness=1)

class TrafficAnalyzer:
    def __init__(self, model_path, class_list_path, video_path, mqtt_broker_ip=None):
        self.model = YOLO(model_path)
        with open(class_list_path, "r") as file:
            self.class_list = file.read().splitlines()
        self.video_path = video_path
        self.scenarios = ["Low", "Medium", "High", "TrafficJam", "Accident"]

        self.mqtt_publisher = None
        if mqtt_broker_ip:
            self.mqtt_publisher = MQTTPublisher(mqtt_broker_ip)
            self.mqtt_publisher.connect()

    def process_frame(self, frame):
        results = self.model.predict(frame)
        detections = []
        if results[0].boxes.data is not None:
            data = results[0].boxes.data.cpu().numpy()
            print(f"Raw detection data: {data}")  
            for row in data:
                if len(row) == 6:
                    x1, y1, x2, y2, conf, class_id = row
                    class_id = int(class_id) 
                    print(f"class_id: {class_id}")  
                    if 0 <= class_id < len(self.class_list): 
                        label = self.class_list[class_id].capitalize() 
                        detections.append((int(x1), int(y1), int(x2), int(y2), label, conf))
                    else:
                        print(f"Invalid class_id: {class_id}")  
        return detections

    def determine_scenarios(self, detections):
        labels = [detection[4] for detection in detections]  
        print(f"Detected labels: {labels}")

        traffic_level = None
        accident_detected = any(label.lower() == "accident" for label in labels)  

        for level in ["Low", "Medium", "High", "TrafficJam"]:
            if level.lower() in [label.lower() for label in labels]:  
                traffic_level = level
                break

        print(f"Traffic level: {traffic_level}, Accident detected: {accident_detected}")
        self.publish_status(traffic_level, accident_detected)
        return traffic_level, accident_detected

    def publish_status(self, traffic_level, accident_detected):
        if accident_detected:
            message = "Accident"
        elif traffic_level:
            message = traffic_level
        else:
            message = "None"

        print(f"Publishing message: {message}")
        if self.mqtt_publisher:
            self.mqtt_publisher.publish("Traffic/RoadStatus", message)
        else:
            print("MQTT publisher not initialized. Cannot publish message.")

    def analyze_video(self):
        cap = cv2.VideoCapture(self.video_path)
        count = 0

        while True:
            ret, frame = cap.read()
            if not ret:
                cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                continue

            count += 1
            if count % 3 != 0:
                continue

            frame = cv2.resize(frame, (1020, 500))
            detections = self.process_frame(frame)
            traffic_level, accident_detected = self.determine_scenarios(detections)

            if traffic_level:
                TrafficScenario(traffic_level).annotate_frame(frame, detections)
            if accident_detected:
                TrafficScenario("Accident").annotate_frame(frame, detections)

            cv2.imshow("Video", frame)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

        cap.release()
        cv2.destroyAllWindows()

        if self.mqtt_publisher:
            self.mqtt_publisher.disconnect()

mqtt_broker_ip = "172.20.10.2"
analyzer = TrafficAnalyzer(
    model_path='/home/basmala/gproject/best.pt',
    class_list_path='/home/basmala/gproject/coco1.txt',
    video_path='/home/basmala/gproject/cr.mp4',
    mqtt_broker_ip=mqtt_broker_ip
)
analyzer.analyze_video()

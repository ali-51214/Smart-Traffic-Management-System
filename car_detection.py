import time
import cv2
import serial
from ultralytics import YOLO

# Load YOLO model
model = YOLO("yolov8n.pt")

# Open the video source (0 for webcam, or provide file path)
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open webcam.")
    exit()

# Configure LoRa module (Connected via USB-UART)
LORA_PORT = "COM3"  # Change as needed (e.g., "/dev/ttyUSB0")
BAUD_RATE = 115200

try:
    ser = serial.Serial(LORA_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Allow LoRa module to initialize
    if ser.is_open:
        print(f"LoRa module connected on {LORA_PORT}")
    else:
        print("Error: Serial port not open.")
        exit()
except Exception as e:
    print(f"Error: {e}")
    exit()

# === LORA Configuration Function ===
def configure_lora(serial_port):
    commands = [
        'AT+RESET',
        'AT+MODE=0',
        'AT+ADDRESS=11',
        'AT+NETWORKID=6',
        'AT+BAND=433000000'
    ]
    for cmd in commands:
        serial_port.write((cmd + '\r\n').encode())
        time.sleep(0.5)  # Wait for response
        while serial_port.in_waiting:
            response = serial_port.readline().decode().strip()
            print(f"{cmd} -> {response}")

# Call the config function
configure_lora(ser)

# Start timer for sending updates every 3 seconds
last_sent_time = time.time()

while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: Failed to capture frame")
        break

    frame_width = frame.shape[1]
    mid_x = frame_width // 2  # Middle point to divide lanes

    results = model(frame)  # Run YOLO detection
    car_count_a = 0  # Lane A (Left Half)
    car_count_b = 0  # Lane B (Right Half)

    if results and results[0].boxes is not None:
        for box in results[0].boxes:
            if int(box.cls) == 2:  # Class ID 2 is "car"
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                center_x = (x1 + x2) // 2

                if center_x < mid_x:
                    car_count_a += 1
                else:
                    car_count_b += 1

                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
                cv2.putText(frame, "Car", (x1, y1 - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)

    # Draw lane division line
    cv2.line(frame, (mid_x, 0), (mid_x, frame.shape[0]), (0, 0, 255), 2)

    # Show the live video feed
    cv2.imshow("Camera Feed", frame)

    # Send data every 3 seconds without slowing video
    if time.time() - last_sent_time >= 3:
        payload = f"LANE1:{car_count_a},LANE2:{car_count_b}"
        command = f"AT+SEND=10,{len(payload)},{payload}\r\n"
        ser.write(command.encode())
        ser.flush()
        print(f"Sent: {command.strip()}")
        print(f"Lane A Cars: {car_count_a} | Lane B Cars: {car_count_b}")
        last_sent_time = time.time()

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
ser.close()
cv2.destroyAllWindows()

# smart_otp_based_door_locking_system
Smart Door OTP Authentication System combining an ESP32 smart lock with a FastAPI and MongoDB backend to generate, email, and verify time-bound single-use OTPs for secure lock/unlock actions, featuring keypad input, LCD status display, buzzer alerts, and automatic OTP expiration.
Smart Door OTP Authentication System (ESP32 + FastAPI + MongoDB)

A secure IoT-based Smart Door Lock system that uses OTP-based verification over Wi-Fi. The system integrates:

ESP32-based electronic smart lock

Keypad input and LCD display

Email-based OTP delivery

FastAPI backend with MongoDB storage

Timeout-based OTP validation

Lock / Unlock operation with servo control

Buzzer-based acoustic feedback

The OTP is generated on the ESP32, sent to the backend, emailed to the user, and verified through keypad entry before executing servo lock/unlock actions.

# 🚀 System Architecture
User → ESP32 Keypad → Generate OTP
        ↓
Send OTP → FastAPI Backend → MongoDB
        ↓
Send OTP via Email
        ↓
User enters OTP on Keypad
        ↓
ESP32 → Backend Verify OTP
        ↓
If Valid → Lock / Unlock Door

✅ Core Features

OTP-based secure authentication

Email delivery to multiple recipients

2-minute OTP expiration control

Lock & Unlock operation modes

MongoDB persistence and audit trail

Built-in health & diagnostics API

Network connection diagnostics on ESP32

LCD status messages and keypad masking

Success / Error / Action buzzer feedback

Anti-replay protection (single-use OTP)

Full Arduino + Backend integration

🧩 Tech Stack
Backend

FastAPI

MongoDB

FastAPI-Mail

Pydantic

Uvicorn

Python dotenv

Hardware / Firmware

ESP32

Wi-Fi Client

LiquidCrystal I2C Display

4×4 Matrix Keypad

Servo Motor

Buzzer

ArduinoJson

HTTPClient

📂 Project Structure
smart-door-otp-system/
│
├── backend/
│   ├── main.py
│   ├── .env
│   └── requirements.txt
│
└── firmware/
    └── es_otp_door_lock.ino

⚙️ Backend Setup
1️⃣ Install Dependencies
pip install -r requirements.txt


Minimal requirements:

fastapi
uvicorn
fastapi-mail
pymongo
python-dotenv
pydantic

2️⃣ Configure Environment Variables (.env)
MAIL_USERNAME=your_email@gmail.com
MAIL_PASSWORD=your_app_password
MAIL_FROM=your_email@gmail.com
MAIL_SERVER=smtp.gmail.com
MAIL_PORT=587

MONGO_URL=mongodb://localhost:27017

RECIPIENTS=user1@gmail.com,user2@gmail.com


Gmail requires App Passwords if 2-factor authentication is enabled.

3️⃣ Start Backend Server
uvicorn main:app --host 0.0.0.0 --port 8000 --reload


Server will run at:

http://localhost:8000

🗄️ MongoDB Collections

Database Name:

smart_door


Collection:

otp


Stored Record Example:

{
  "otp": "5839",
  "mode": "unlock",
  "created_at": "...",
  "expire_at": "..."
}


Old OTPs are cleared before new creation.

🔌 API Endpoints
Send OTP
POST /send-otp


Request:

{
  "otp": "1234",
  "mode": "lock"
}


Responses:

success (OTP stored + email sent)

partial_success (email failure, DB success)

Verify OTP
POST /verify-otp


Request:

{
  "otp": "1234"
}


Success Response:

{
  "status": "success",
  "message": "OTP verified successfully",
  "mode": "unlock"
}

Health Check
GET /health

Root Endpoint
GET /


Provides service metadata.

🧰 ESP32 Firmware Configuration

Update Wi-Fi + Server IP:

const char* WIFI_SSID = "your_wifi";
const char* WIFI_PASS = "your_password";

String BASE_URL = "http://<backend-ip>:8000";


Ensure ESP32 and backend are on the same network.

🛠️ Hardware Components
Component	Purpose
ESP32 Dev Board	Controller
4×4 Matrix Keypad	OTP input
Arduino I2C LCD	Status display
Servo Motor	Lock / Unlock
Buzzer	Audio feedback
Power Supply	System power
🔌 Servo Operation
Mode	Action
lock	move to 90°
unlock	move to 0°
🎵 Buzzer Feedback Mapping
Event	Tone
Startup	Melody
Key Press	Short beep
Wi-Fi Connect	Success beep
OTP Sent	Success beep
OTP Verified	Success beep
Lock / Unlock	Triple tone
Failure / Error	Low tone
🧪 Operational Workflow

User presses:

#. # → Lock mode

* → Unlock mode

ESP32 generates OTP

OTP sent to backend + stored in Mongo

OTP emailed to recipients

User enters OTP via keypad

ESP32 verifies OTP with backend

If valid:

Servo actuates

Logs removed from DB

Buzzer acknowledgment

🛡️ Security Controls

Implemented safeguards:

OTP expires in 2 minutes

OTP is single-use

Previous OTPs auto-cleared

Backend validates timestamp

Request origin independent of user device

ESP32 never stores OTPs after use

🧯 Troubleshooting Guide
ESP32 cannot connect to backend

Check server IP reachability

Ensure both devices are on same network

Use machine IP, not localhost

Run:

ipconfig   # Windows
ifconfig   # Linux / Mac


Update:

String BASE_URL = "http://<system-ip>:8000";

Email not sending

Verify:

App password enabled

SMTP server correct

Port 587 (TLS)

OTP always invalid

Possible causes:

OTP expired

Incorrect keypad entry

Backend restarted mid-session

Check Mongo timestamps.

🧱 Future Enhancements

Mobile App OTP approval

SMS-based OTP delivery

Multi-user & role-based control

Entry audit trail dashboard

Cloud-synced remote monitoring

Camera event capture

Battery-fail safety mode

📜 License

This project is intended for research, academic, and prototype engineering use.

🙋 Need Support or Modifications?

Tell me what you want next:

Deployment guide for Render / Railway / EC2

OTA firmware update support

Logging + analytics module

Security hardening recommendations

UI panel for admin OTP approval

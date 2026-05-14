import serial  

import requests  

import json  

import re  

import datetime  

 

  

# Webhook URL to send data to Apps Script #  

WEBHOOK_URL = "https://script.google.com/macros/s/AKfycbynKXA9ns_m9UxLsnDN6LZCFx1cPWby0Vk8gqL2-h5OaKQ7Q96tk6SzvzdSdLN6gKy9Ug/exec" 

 

# For serial communication with Arduino  

COM_PORT = "COM8"  

BAUD_RATE = 9600  

 

# example -v  

# EVENT, ENTRY  

# ENTRY, EXIT 

 

 

event_pattern = re.compile(r"EVENT,(ENTRY|EXIT)") 

 

def send_to_google(action): 

    """Send event payload to Google Apps Script""" 

    data = { 

        "action": action, 

        "timestamp": str(datetime.datetime.now()), 

    } 

 

    print("Sending:", data) 

    try: 

        r = requests.post(WEBHOOK_URL, json=data) # Sending POST request with JSON payload 

        print("Response:", r.text) 

    except Exception as e: 

        print("ERROR sending to Google:", e) # Google error check 

 

 

def main(): 

    ser = serial.Serial(COM_PORT, BAUD_RATE) 

    print("Looooking for arduino...") 

 

    while True: 

        line = ser.readline().decode("utf-8", errors="ignore").strip() 

        print("Arduino:", line) 

 

	# Check if line matches the expected event format; ignore if not matched 

        match = event_pattern.search(line) 

        if not match: 

            continue 

 

        action = match.group(1) # Sending the action to google sheets 

        send_to_google(action) 

         

if __name__ == "__main__": 

    main() 

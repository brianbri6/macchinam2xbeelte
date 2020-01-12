/*
  Send a TCP message to Hologram
  By: Jim Lindblom
  SparkFun Electronics
  Date: October 23, 2018
  License: This code is public domain but you buy me a beer if you use this 
  and we meet someday (Beerware license).
  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14997

  This example demonstrates how to send TCP messages to a Hologram server

  Before beginning, you should have your shield connected on a MNO.
  See example 00 for help with that.

  Before uploading, set your HOLOGRAM_DEVICE_KEY. This string can be found
  in your device's Hologram dashboard.

  Once programmed, open the serial monitor, set the baud rate to 9600,
  and type a message to be sent via TCP to the Hologram message service.
  Make sure your serial monitor's end-of-line setting is set to "newline".
  
  Hardware Connections:
  Attach the SparkFun LTE Cat M1/NB-IoT Shield to your Arduino
  Power the shield with your Arduino -- ensure the PWR_SEL switch is in
    the "ARDUINO" position.
*/

//Click here to get the library: http://librarymanager/All#SparkFun_LTE_Shield_Arduino_Library
#include <SparkFun_LTE_Shield_Arduino_Library.h>

// Create a SoftwareSerial object to pass to the LTE_Shield library
#define lteSerial Serial1
// Create a LTE_Shield object to use throughout the sketch
LTE_Shield lte;

// Plug in your Hologram device key here:
String HOLOGRAM_DEVICE_KEY = "devicekey";

// These values should remain the same:
const char HOLOGRAM_URL[] = "cloudsocket.hologram.io";
const unsigned int HOLOGRAM_PORT = 9999;
const unsigned int HOLOGRAM_LISTEN_PORT = 4010;

int listeningSocket = -1;


// Callback to process data coming into the LTE module
void processSocketRead(int socket, String response) {
  // Print message received, and the IP address it was received from.
  // Also print socket received on.
  SerialUSB.println("Read: " + response);
  SerialUSB.println("Socket: " + String(socket));
  SerialUSB.print("Remote IP: ");
  SerialUSB.println(lte.lastRemoteIP());
}

// Callback to process when a socket closes
void processSocketClose(int socket) {
  // If the closed socket is the one we're listening on.
  // Set a flag to re-open the listening socket.
  if (socket == listeningSocket) {
    listeningSocket = -1;
  } else {
    // Otherwise print the closed socket
    SerialUSB.println("Socket " + String(socket) + " closed");
  }
}

void setup() {
  SerialUSB.begin(9600);
 //delay(4000);
  if ( lte.begin(lteSerial, 9600) ) {
    SerialUSB.println(F("LTE Shield connected!"));
  }
  
  
 // delay(6000); 
  lte.setSocketReadCallback(&processSocketRead);
  lte.setSocketCloseCallback(&processSocketClose);
  
  
  SerialUSB.println(F("Type a message. Send a Newline (\\n) to send it..."));
}






void loop() {
  static String message = "";

 // If a listening socket is not open. Set up a new one.
  if (listeningSocket < 0) {
    listenHologramMessage();
  }
  
  if (SerialUSB.available())
  {
    char c = SerialUSB.read();
    // Read a message until a \n (newline) is received
    if (c == '\n') {
      // Once we receive a newline. send the text.
      SerialUSB.println("Sending: " + String(message));
      // Call lte.sendSMS(String number, String message) to send an SMS
      // message.
      sendHologramMessage(message);
      message = ""; // Clear message string
    } else {
      message += c; // Add last character to message
    }
  }
  lte.poll();
}

void sendHologramMessage(String message)
{
  int socket = -1;
  String hologramMessage;

  // New lines are not handled well
  message.replace('\r', ' ');
  message.replace('\n', ' ');

  // Construct a JSON-encoded Hologram message string:
  hologramMessage = "{\"k\":\"" + HOLOGRAM_DEVICE_KEY + "\",\"d\":\"" +
    message + "\"}";
  
  
  // Open a socket
  socket = lte.socketOpen(LTE_SHIELD_TCP);
  // On success, socketOpen will return a value between 0-5. On fail -1.
  if (socket >= 0) {
    // Use the socket to connec to the Hologram server
    SerialUSB.println("Connecting to socket: " + String(socket));
    if (lte.socketConnect(socket, HOLOGRAM_URL, HOLOGRAM_PORT) == LTE_SHIELD_SUCCESS) {
      // Send our message to the server:
      SerialUSB.println("Sending: " + String(hologramMessage));
      if (lte.socketWrite(socket, hologramMessage) == LTE_SHIELD_SUCCESS)
      {
        // On succesful write, close the socket.
        if (lte.socketClose(socket) == LTE_SHIELD_SUCCESS) {
          SerialUSB.println("Socket " + String(socket) + " closed");
        }
      } else {
        SerialUSB.println(F("Failed to write"));
      }
    }
  }
}


void listenHologramMessage()
{
  int sock = -1;
  LTE_Shield_error_t err;

  // Open a new available socket
  listeningSocket = lte.socketOpen(LTE_SHIELD_TCP);
  // If a socket is available it should return a value between 0-5
  if (listeningSocket >= 0) {
    // Listen on the socket on the defined port
    err = lte.socketListen(listeningSocket, HOLOGRAM_LISTEN_PORT);
    if (err == LTE_SHIELD_ERROR_SUCCESS) {
      SerialUSB.print(F("Listening socket open: "));
      SerialUSB.println(listeningSocket);
    }
    else {
      SerialUSB.println("Unable to listen on socket");
    }
  }
  else {
    SerialUSB.println("Unable to open socket");
  }
}

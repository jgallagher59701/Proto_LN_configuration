#include <Arduino.h>

#include "blink.h"

#define DEBUG 1

#define MAX_INIT_MSG 64
#define ACK_MSG 0xAA
#define MAX_WAIT_MSG 2000 // in ms

#define ACK "ACK"
#define CONFIG_MSG "hello"    // Tell the configuration software the node is ready

bool buffer_equal(const char *buf, unsigned len, const char *expected) {
    if (len == strlen(expected))
      return memcmp(buf, expected, len) == 0;
    else 
      return false;
}

void send_msg(const char *msg) {
    Serial.println(msg);
    Serial.flush();
}

void send_int(int32_t value) {
    Serial.println(value);
    Serial.flush();
}

bool read_ack() {
    unsigned len = 0;
    char buffer[MAX_INIT_MSG] = {""};
    len = Serial.readBytes(buffer, MAX_INIT_MSG);
#if DEBUG
    if (buffer_equal(buffer, len, ACK)) {
      buffer[len] = '\0';
      char msg[128] = {0};
      snprintf(msg, 128, "Success! bytes read: %d, @%s@", len, buffer);
      send_msg(msg);
      return true;
    }
    else {
      buffer[len] = '\0';
      char msg[128] = {0};
      snprintf(msg, 128, "FAIL! bytes read: %d, @%s@", len, buffer);
      send_msg(msg);
      return false;
    }
#else
    return buffer_equal(buffer, len, ACK);
#endif
}

// We only know about a fixed set of parameters
enum parameter {
  unknown,
  interval,
  node,
  channel
};

// Constants that define the strings the configuration program uses for the parameters
#define INTERVAL "interval"
#define NODE "node"
#define CHANNEL "channel"

// Call this when we expect to read the name of a parameter
parameter read_parameter() {
    char buffer[MAX_INIT_MSG] = {""};
    unsigned len = Serial.readBytes(buffer, MAX_INIT_MSG);

#if DEBUG
    bool is_interval = buffer_equal(buffer, len, INTERVAL);
    buffer[len] = '\0';
    char msg[128] = {0};
    snprintf(msg, 128, "read_parameter bytes read: %d, @%s@, is interval: %s", len, buffer, is_interval ? "True": "False");
    send_msg(msg);
#endif

    parameter p = unknown;
    if (buffer_equal(buffer, len, INTERVAL)) p = interval;
    else if (buffer_equal(buffer, len, NODE)) p = node;
    else if (buffer_equal(buffer, len, CHANNEL)) p = channel;
    else p = unknown;

    switch (p) {
      case interval:
      case node:
      case channel:
        send_msg("ACK");
        return p;
      
      default:
      send_msg("FAIL");
        return p;
    }
}

// Call this when we expect to read an integer value for a parameter
int read_int_value() {
    char buffer[MAX_INIT_MSG] = {""};
    unsigned len = Serial.readBytes(buffer, MAX_INIT_MSG);

#if DEBUG
    char msg[128] = {0};
    snprintf(msg, 128, "read_int_value bytes read: %d, @%d@", len, *(int*)&buffer);
    send_msg(msg);
#endif

    int32_t value = 0xFFFF;

    if (len == 4) {
      value = *(int*)&buffer;
      send_msg("ACK");
      send_int(value);
    }
   
    return value;
}

void node_initialization() {
    Serial.begin(9600);

    // Wait for the serial line to settle for up to MAX_WAIT_MSG ms
    long time = millis();
    while (!Serial) {
      if (millis() - time > MAX_WAIT_MSG)
        break;
    }

    // Conduct initial handshake with programming computer. Send 'hello'
    // and wait for the computer to respond with ACK_MSG
    Serial.setTimeout(MAX_WAIT_MSG);
  
    send_msg(CONFIG_MSG);
    if (read_ack()) {
      parameter p = read_parameter();
      int32_t value = read_int_value();

      blink(LED_BUILTIN, 3, 0);
    }


    // Now assume the serial link is working and 
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // put your setup code here, to run once:
  node_initialization();
}

void loop() {
  // put your main code here, to run repeatedly:
  blink(LED_BUILTIN, 2, 0);
}
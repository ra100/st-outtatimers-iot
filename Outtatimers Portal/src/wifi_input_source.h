#pragma once

#include "input_manager.h"
#include "status_led.h"

#ifndef UNIT_TEST
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
// Mock classes for unit testing
class ESP8266WebServer
{
public:
  ESP8266WebServer(int port) {}
  void begin() {}
  void handleClient() {}
  void on(const char *path, std::function<void()> handler) {}
  void send(int code, const char *type, const char *content) {}
  bool hasArg(const char *name) { return false; }
  String arg(const char *name) { return ""; }
};
#endif

/**
 * @brief WiFi-based input source for remote control
 *
 * This input source creates a simple web server that accepts HTTP requests
 * to trigger portal commands. This demonstrates how the InputManager system
 * can be extended with new input sources without modifying existing code.
 *
 * @example
 * ```cpp
 * WiFiInputSource wifiInput(80);  // HTTP server on port 80
 * wifiInput.begin("MyPortal", "password123");
 * inputManager.addInputSource(&wifiInput);
 *
 * // Now you can control via HTTP:
 * // http://portal-ip/toggle
 * // http://portal-ip/malfunction
 * // http://portal-ip/fadeout
 * ```
 */
class WiFiInputSource : public IInputSource
{
public:
  /**
   * @brief Construct a new WiFiInputSource
   * @param port HTTP server port (default: 80)
   */
  explicit WiFiInputSource(int port = 80)
      : server_(port), eventQueueHead_(0), eventQueueTail_(0), isConnected_(false) {}

  /**
   * @brief Initialize WiFi and start web server
   * @param ssid WiFi network name
   * @param password WiFi password
   * @return true if initialization successful
   */
  bool begin(const char *ssid, const char *password)
  {
#ifndef UNIT_TEST
    // Initialize status LED
    StatusLED::begin();

    // Start WiFi connection
    WiFi.begin(ssid, password);
    StatusLED::update(PortalConfig::Hardware::WiFiStatus::CONNECTING, millis());

    // Wait for connection (with timeout)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
      delay(500);
      attempts++;
      // Update status LED during connection attempts
      StatusLED::update(PortalConfig::Hardware::WiFiStatus::CONNECTING, millis());
    }

    if (WiFi.status() != WL_CONNECTED)
    {
      StatusLED::update(PortalConfig::Hardware::WiFiStatus::DISCONNECTED, millis());
      return false;
    }

    isConnected_ = true;
    StatusLED::update(PortalConfig::Hardware::WiFiStatus::CONNECTED, millis());

    // Set up web server routes
    server_.on("/", [this]()
               { handleRoot(); });
    server_.on("/toggle", [this]()
               { handleCommand(InputManager::Command::TogglePortal); });
    server_.on("/malfunction", [this]()
               { handleCommand(InputManager::Command::TriggerMalfunction); });
    server_.on("/fadeout", [this]()
               { handleCommand(InputManager::Command::FadeOut); });
    server_.on("/status", [this]()
               { handleStatus(); });

    server_.begin();
#endif

    return true;
  }

  bool update(unsigned long currentTime) override
  {
#ifndef UNIT_TEST
    if (isConnected_)
    {
      server_.handleClient();

      // Check if there are any connected clients (stations)
      bool hasClients = false;
#ifdef ESP8266
      // ESP8266WiFi has WiFi.softAPgetStationNum() for AP mode
      // or we can check if any client has connected recently via the web server
      // For now, we'll use a simple heuristic: if we've handled any HTTP requests recently
      hasClients = (WiFi.softAPgetStationNum() > 0);
#endif

      // Update status LED based on connection and client status
      if (hasClients)
      {
        StatusLED::update(PortalConfig::Hardware::WiFiStatus::CONNECTED_WITH_CLIENTS, currentTime);
      }
      else
      {
        StatusLED::update(PortalConfig::Hardware::WiFiStatus::CONNECTED, currentTime);
      }
    }
#endif
    return hasEvents();
  }

  bool hasEvents() const override
  {
    return eventQueueHead_ != eventQueueTail_;
  }

  InputEvent getNextEvent() override
  {
    if (!hasEvents())
    {
      return {0, EventType::Released, 0, "none"};
    }

    InputEvent event = eventQueue_[eventQueueHead_];
    eventQueueHead_ = (eventQueueHead_ + 1) % MAX_EVENTS;
    return event;
  }

  const char *getSourceName() const override
  {
    return "WiFiInput";
  }

  /**
   * @brief Get the WiFi IP address
   * @return IP address as string, or "Not Connected" if not connected
   */
  const char *getIPAddress() const
  {
#ifndef UNIT_TEST
    if (isConnected_)
    {
      static String ipStr = WiFi.localIP().toString();
      return ipStr.c_str();
    }
#endif
    return "Not Connected";
  }

  /**
   * @brief Check if WiFi is connected
   * @return true if connected to WiFi
   */
  bool isConnected() const
  {
    return isConnected_;
  }

private:
  static constexpr int MAX_EVENTS = 8;

  ESP8266WebServer server_;
  InputEvent eventQueue_[MAX_EVENTS];
  int eventQueueHead_;
  int eventQueueTail_;
  bool isConnected_;

  /**
   * @brief Handle root web page request
   */
  void handleRoot()
  {
    const char *html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Portal Controller</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; background: #1a1a1a; color: #fff; }
        .container { max-width: 600px; margin: 0 auto; }
        .button {
            display: inline-block;
            padding: 15px 30px;
            margin: 10px;
            background: #4CAF50;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            font-size: 18px;
        }
        .button:hover { background: #45a049; }
        .malfunction { background: #f44336; }
        .malfunction:hover { background: #da190b; }
        .fadeout { background: #ff9800; }
        .fadeout:hover { background: #e68900; }
        h1 { color: #4CAF50; text-align: center; }
        .status { background: #333; padding: 20px; border-radius: 5px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌀 Portal Controller</h1>
        <div class="status">
            <h3>Available Commands:</h3>
            <p>Control your portal effect remotely via WiFi</p>
        </div>
        <div style="text-align: center;">
            <a href="/toggle" class="button">Toggle Portal</a><br>
            <a href="/malfunction" class="button malfunction">Trigger Malfunction</a><br>
            <a href="/fadeout" class="button fadeout">Fade Out</a><br>
            <a href="/status" class="button">Status</a>
        </div>
    </div>
</body>
</html>
        )";

    server_.send(200, "text/html", html);
  }

  /**
   * @brief Handle command requests
   * @param command Command to execute
   */
  void handleCommand(InputManager::Command command)
  {
    // Queue the event
    queueEvent({.inputId = static_cast<int>(command),
                .type = EventType::Pressed,
                .timestamp = millis(),
                .sourceName = "WiFi"});

    // Send response
    String response = "Command executed: ";
    response += InputManager::getCommandName(command);

    server_.send(200, "text/plain", response);
  }

  /**
   * @brief Handle status request
   */
  void handleStatus()
  {
    String status = "Portal Controller Status\n";
    status += "WiFi Connected: Yes\n";
    status += "IP Address: ";
    status += getIPAddress();
    status += "\n";
    status += "Available Commands:\n";
    status += "  /toggle - Toggle portal effect\n";
    status += "  /malfunction - Trigger malfunction\n";
    status += "  /fadeout - Fade out effect\n";

    server_.send(200, "text/plain", status);
  }

  /**
   * @brief Add event to queue
   * @param event Event to queue
   */
  void queueEvent(const InputEvent &event)
  {
    int nextTail = (eventQueueTail_ + 1) % MAX_EVENTS;
    if (nextTail != eventQueueHead_)
    { // Don't overflow
      eventQueue_[eventQueueTail_] = event;
      eventQueueTail_ = nextTail;
    }
  }
};
#pragma once

#include "input_manager.h"
#include "status_led.h"
#include "config_manager.h"

#ifndef UNIT_TEST
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
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
 * to trigger turbolift commands. This demonstrates how the InputManager system
 * can be extended with new input sources without modifying existing code.
 *
 * @example
 * ```cpp
 * WiFiInputSource wifiInput(80);  // HTTP server on port 80
 * wifiInput.begin("MyTurbolift", "password123");
 * inputManager.addInputSource(&wifiInput);
 *
 * // Now you can control via HTTP:
 * // http://turbolift-ip/toggle
 * // http://turbolift-ip/malfunction
 * // http://turbolift-ip/fadeout
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
      : server_(port), eventQueueHead_(0), eventQueueTail_(0), isConnected_(false),
        connectionStartTime_(0), inAPMode_(false), apServerStarted_(false) {}

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

    // Initialize LittleFS filesystem for serving web assets (modern replacement for SPIFFS with better wear-leveling)
    if (!LittleFS.begin())
    {
      Serial.println(F("LittleFS mount failed - check flash partitioning and available space"));
      StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED, millis());
      return false;
    }

    Serial.println(F("LittleFS mounted successfully"));

    // Start WiFi connection (non-blocking)
    WiFi.begin(ssid, password);
    connectionStartTime_ = millis();
    StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::CONNECTING_STA, millis());

    // Set up web server routes for both STA and AP modes
    setupWebServerRoutes();

    server_.begin();
#endif

    return true;
  }

  bool update(unsigned long currentTime) override
  {
#ifndef UNIT_TEST
    // Check WiFi connection status and handle fallback to AP mode
    if (!isConnected_ && !inAPMode_)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        // WiFi connected successfully
        isConnected_ = true;
        StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::STA_CONNECTED, currentTime);

        Serial.print("WiFi connected! Web interface available at: http://");
        Serial.println(WiFi.localIP().toString().c_str());
        Serial.println("WiFi commands available:");
        Serial.println("  http://[ip]/toggle - Toggle turbolift effect");
        Serial.println("  http://[ip]/malfunction - Trigger malfunction");
        Serial.println("  http://[ip]/fadeout - Fade out effect");
      }
      else
      {
        // Check for connection timeout
        if (currentTime - connectionStartTime_ > TurboliftConfig::WiFi::WIFI_TIMEOUT_MS)
        {
          // Connection timed out - switch to AP mode
          Serial.println("WiFi connection timeout - switching to AP mode");
          switchToAPMode();
        }
        else
        {
          // Still connecting - update status LED
          StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::CONNECTING_STA, currentTime);
        }
      }
    }
    else if (inAPMode_)
    {
      // In AP mode - handle web server
      if (!apServerStarted_)
      {
        startAPServer();
      }

      server_.handleClient();

      // Check if there are any connected clients
      int numClients = WiFi.softAPgetStationNum();
      bool hasClients = numClients > 0;

      // Update status LED based on AP mode and client status
      if (hasClients)
      {
        StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::AP_WITH_CLIENTS, currentTime);
      }
      else
      {
        StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::AP_MODE, currentTime);
      }
    }
    else
    {
      // WiFi is connected - handle web server
      server_.handleClient();

      // Check if there are any connected clients (stations)
      int numClients = WiFi.softAPgetStationNum();
      bool hasClients = numClients > 0;

      // Update status LED based on connection and client status
      if (hasClients)
      {
        StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::AP_WITH_CLIENTS, currentTime);
      }
      else if (WiFi.getMode() & WIFI_STA)
      {
        // STA mode
        StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::STA_CONNECTED, currentTime);
      }
      else
      {
        // AP mode without clients
        StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::AP_MODE, currentTime);
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
    else if (inAPMode_)
    {
      static String apStr = WiFi.softAPIP().toString();
      return apStr.c_str();
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

  /**
   * @brief Check if device is in AP mode
   * @return true if in AP mode
   */
  bool isInAPMode() const
  {
    return inAPMode_;
  }

  /**
   * @brief Set up web server routes for both STA and AP modes
   */
  void setupWebServerRoutes()
  {
    server_.on("/", [this]()
               { handleRoot(); });
    server_.on("/toggle", [this]()
               { handleCommand(InputManager::Command::ToggleTurbolift); });
    server_.on("/malfunction", [this]()
               { handleCommand(InputManager::Command::TriggerMalfunction); });
    server_.on("/fadeout", [this]()
               { handleCommand(InputManager::Command::FadeOut); });
    server_.on("/status", [this]()
               { handleStatus(); });
    server_.on("/config", [this]()
               { handleConfig(); });
    server_.on("/set_speed", [this]()
               { handleSetSpeed(); });
    server_.on("/set_brightness", [this]()
               { handleSetBrightness(); });
    server_.on("/set_hue", [this]()
               { handleSetHue(); });
    server_.on("/set_saturation", [this]()
               { handleSetSaturation(); });
    server_.on("/set_mode", [this]()
               { handleSetMode(); });
    server_.on("/set_effect_mode", [this]()
               { handleSetEffectMode(); });

    // Lift animation endpoints
    server_.on("/set_lift_submode", [this]()
               { handleLiftIntParam("submode", [](int v)
                                    { ConfigManager::setLiftSubmode(v); }, 0, 3); });
    server_.on("/set_lift_speed", [this]()
               { handleLiftIntParam("speed", [](int v)
                                    { ConfigManager::setLiftSpeed((uint8_t)v); }, 0, 100); });
    server_.on("/set_lift_width", [this]()
               { handleLiftIntParam("width", [](int v)
                                    { ConfigManager::setLiftWidth(v); }, 1, 100); });
    server_.on("/set_lift_spacing", [this]()
               { handleLiftIntParam("spacing", [](int v)
                                    { ConfigManager::setLiftSpacing(v); }, 0, 200); });
    server_.on("/set_lift_hue", [this]()
               { handleLiftIntParam("hue", [](int v)
                                    { ConfigManager::setLiftHue((uint8_t)v); }, 0, 255); });
    server_.on("/set_lift_saturation", [this]()
               { handleLiftIntParam("saturation", [](int v)
                                    { ConfigManager::setLiftSaturation((uint8_t)v); }, 0, 255); });
    server_.on("/set_lift_brightness", [this]()
               { handleLiftIntParam("brightness", [](int v)
                                    { ConfigManager::setLiftBrightness((uint8_t)v); }, 0, 255); });
    server_.on("/set_lift_skip_start", [this]()
               { handleLiftIntParam("count", [](int v)
                                    { ConfigManager::setLiftSkipStart(v); }, 0, TurboliftConfig::Hardware::NUM_LEDS / 2); });
    server_.on("/set_lift_skip_middle", [this]()
               { handleLiftIntParam("count", [](int v)
                                    { ConfigManager::setLiftSkipMiddle(v); }, 0, TurboliftConfig::Hardware::NUM_LEDS); });
    server_.on("/set_lift_skip_end", [this]()
               { handleLiftIntParam("count", [](int v)
                                    { ConfigManager::setLiftSkipEnd(v); }, 0, TurboliftConfig::Hardware::NUM_LEDS / 2); });
    server_.on("/set_lift_pulse_min", [this]()
               { handleLiftIntParam("value", [](int v)
                                    { ConfigManager::setLiftPulseMin(v); }, 0, 255); });
    server_.on("/set_lift_pulse_max", [this]()
               { handleLiftIntParam("value", [](int v)
                                    { ConfigManager::setLiftPulseMax(v); }, 0, 255); });
    server_.on("/set_lift_pulse_speed", [this]()
               { handleLiftIntParam("value", [](int v)
                                    { ConfigManager::setLiftPulseSpeed(v); }, 0, 10); });
    server_.on("/options", HTTP_OPTIONS, [this]()
               {
         server_.sendHeader("Access-Control-Allow-Origin", "*");
         server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
         server_.sendHeader("Access-Control-Allow-Headers", "*");
         server_.send(200, "text/plain", ""); });
    server_.onNotFound([this]()
                       {
         server_.sendHeader("Access-Control-Allow-Origin", "*");
         server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
         server_.sendHeader("Access-Control-Allow-Headers", "*");
         server_.send(404, "text/plain", "Not Found"); });
  }

private:
  static constexpr int MAX_EVENTS = 8;

  ESP8266WebServer server_;
  InputEvent eventQueue_[MAX_EVENTS];
  int eventQueueHead_;
  int eventQueueTail_;
  bool isConnected_;
  unsigned long connectionStartTime_;
  bool inAPMode_;
  bool apServerStarted_;

  /**
   * @brief Send CORS headers for all responses
   */
  void sendCORSHeaders()
  {
    server_.sendHeader("Access-Control-Allow-Origin", "*");
    server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    server_.sendHeader("Access-Control-Allow-Headers", "*");
  }

  /**
   * @brief Read file content from LittleFS
   * @param path File path to read
   * @return File content as String
   */
  String readFile(const char *path)
  {
#ifndef UNIT_TEST
    File file = LittleFS.open(path, "r");
    if (!file)
    {
      return "File not found";
    }

    String content = "";
    while (file.available())
    {
      content += (char)file.read();
    }
    file.close();
    return content;
#else
    return "File reading not supported in unit test mode";
#endif
  }

  /**
   * @brief Handle root web page request
   */
  void handleRoot()
  {
#ifndef UNIT_TEST
    sendCORSHeaders();
    File f = LittleFS.open("/index.html", "r");
    if (f)
    {
      server_.streamFile(f, "text/html");
      f.close();
    }
    else
    {
      server_.send(404, "text/plain", "index.html not found");
    }
#else
    sendCORSHeaders();
    server_.send(200, "text/plain", "Web interface not available in unit test mode");
#endif
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

    sendCORSHeaders();
    server_.send(200, "text/plain", response);
  }

  /**
   * @brief Handle status request
   */
  void handleStatus()
  {
    String status = "Turbolift Controller Status\n";
    status += "WiFi Connected: Yes\n";
    status += "IP Address: ";
    status += getIPAddress();
    status += "\n";
    status += "Available Commands:\n";
    status += "  /toggle - Toggle turbolift effect\n";
    status += "  /malfunction - Trigger malfunction\n";
    status += "  /fadeout - Fade out effect\n";
    status += "  /config - View current configuration\n";
    status += "  /set_speed?speed=0-10 - Set rotation speed\n";
    status += "  /set_brightness?brightness=0-255 - Set max brightness\n";
    status += "  /set_hue?min=0-255&max=0-255 - Set color hue range\n";
    status += "  /set_saturation?min=0-255&max=0-255 - Set color saturation range\n";

    sendCORSHeaders();
    server_.send(200, "text/plain", status);
  }

  /**
   * @brief Handle configuration request
   */
  void handleConfig()
  {
    char buf[384];
    snprintf(buf, sizeof(buf),
             "{"
             "\"effectMode\":%u,"
             "\"liftSubmode\":%u,"
             "\"liftSpeed\":%u,"
             "\"liftWidth\":%u,"
             "\"liftSpacing\":%u,"
             "\"liftHue\":%u,"
             "\"liftSaturation\":%u,"
             "\"liftBrightness\":%u,"
             "\"liftSkipStart\":%u,"
             "\"liftSkipMiddle\":%u,"
             "\"liftSkipEnd\":%u,"
             "\"liftPulseMin\":%u,"
             "\"liftPulseMax\":%u,"
             "\"liftPulseSpeed\":%u"
             "}",
             (unsigned)ConfigManager::getEffectMode(),
             (unsigned)ConfigManager::getLiftSubmode(),
             (unsigned)ConfigManager::getLiftSpeed(),
             (unsigned)ConfigManager::getLiftWidth(),
             (unsigned)ConfigManager::getLiftSpacing(),
             (unsigned)ConfigManager::getLiftHue(),
             (unsigned)ConfigManager::getLiftSaturation(),
             (unsigned)ConfigManager::getLiftBrightness(),
             (unsigned)ConfigManager::getLiftSkipStart(),
             (unsigned)ConfigManager::getLiftSkipMiddle(),
             (unsigned)ConfigManager::getLiftSkipEnd(),
             (unsigned)ConfigManager::getLiftPulseMin(),
             (unsigned)ConfigManager::getLiftPulseMax(),
             (unsigned)ConfigManager::getLiftPulseSpeed());

    sendCORSHeaders();
    server_.send(200, "application/json", buf);
  }

  /**
   * @brief Generic lift parameter handler — reads named arg, calls setter, returns OK
   * @param argName Query parameter name
   * @param setter Setter lambda that accepts int
   * @param lo Minimum allowed value
   * @param hi Maximum allowed value
   */
  void handleLiftIntParam(const char *argName, void (*setter)(int), int lo, int hi)
  {
    if (server_.hasArg(argName))
    {
      int v = constrain(server_.arg(argName).toInt(), lo, hi);
      setter(v);
      sendCORSHeaders();
      server_.send(200, "text/plain", "OK");
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing parameter");
    }
  }

  /**
   * @brief Handle set speed request
   */
  void handleSetSpeed()
  {
    if (server_.hasArg("speed"))
    {
      int speed = server_.arg("speed").toInt();
      ConfigManager::setRotationSpeed(speed);
      String response = "Rotation speed set to: " + String(speed) + " (0-10)";
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing speed parameter");
    }
  }

  /**
   * @brief Handle set brightness request
   */
  void handleSetBrightness()
  {
    if (server_.hasArg("brightness"))
    {
      int brightness = server_.arg("brightness").toInt();
      ConfigManager::setMaxBrightness(brightness);
      String response = "Max brightness set to: " + String(brightness) + " (0-255)";
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing brightness parameter");
    }
  }

  /**
   * @brief Handle set hue range request
   */
  void handleSetHue()
  {
    if (server_.hasArg("min") && server_.hasArg("max"))
    {
      int minHue = server_.arg("min").toInt();
      int maxHue = server_.arg("max").toInt();
      ConfigManager::setHueMin(minHue);
      ConfigManager::setHueMax(maxHue);
      String response = "Color hue range set to: " + String(minHue) + " - " + String(maxHue) + " (0-255)";
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing min or max parameter");
    }
  }

  /**
   * @brief Handle set saturation range request
   */
  void handleSetSaturation()
  {
    if (server_.hasArg("min") && server_.hasArg("max"))
    {
      int minSat = server_.arg("min").toInt();
      int maxSat = server_.arg("max").toInt();
      ConfigManager::setSatMin(minSat);
      ConfigManager::setSatMax(maxSat);
      String response = "Color saturation range set to: " + String(minSat) + " - " + String(maxSat) + " (0-255)";
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing min or max parameter");
    }
  }

  /**
   * @brief Add event to queue
   * @param event Event to queue
   */
  /**
   * @brief Handle set mode request
   */
  void handleSetMode()
  {
    if (server_.hasArg("mode"))
    {
      int mode = server_.arg("mode").toInt();
      ConfigManager::setTurboliftMode(mode);
      String response = "Turbolift mode set to: " + String(mode == 0 ? "Classic" : "Virtual Gradients");
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing mode parameter");
    }
  }
  /**
   * @brief Handle set effect mode request (single color vs lift animation)
   */
  void handleSetEffectMode()
  {
    if (server_.hasArg("mode"))
    {
      int mode = server_.arg("mode").toInt();
      ConfigManager::setEffectMode(mode);
      const char *modeName;
      switch (mode)
      {
      case 0:
        modeName = "Single Color";
        break;
      case 1:
        modeName = "Lift Animation";
        break;
      default:
        modeName = "Unknown";
        break;
      }
      String response = "Effect mode set to: " + String(modeName);
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing mode parameter (0=Single Color, 1=Lift Animation)");
    }
  }

  /**
   * @brief Switch to Access Point mode when STA connection fails
   */
  void switchToAPMode()
  {
#ifndef UNIT_TEST
    Serial.println("Switching to Access Point mode...");

    // Disconnect from any existing WiFi
    WiFi.disconnect();

    // Set WiFi mode to AP
    WiFi.mode(WIFI_AP);

    // Start AP with configured SSID and password
    WiFi.softAP(TurboliftConfig::WiFi::AP_NAME, TurboliftConfig::WiFi::AP_PASS);

    Serial.print("AP mode started. SSID: ");
    Serial.println(TurboliftConfig::WiFi::AP_NAME);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP().toString().c_str());

    inAPMode_ = true;
    StatusLED::update(TurboliftConfig::Hardware::WiFiStatus::AP_MODE, millis());
#endif
  }

  /**
   * @brief Start the web server in AP mode
   */
  void startAPServer()
  {
#ifndef UNIT_TEST
    Serial.println("Starting AP web server...");

    // Server is already configured in begin(), just need to ensure it's started
    apServerStarted_ = true;

    Serial.println("AP web server started");
    Serial.print("Connect to WiFi network: ");
    Serial.println(TurboliftConfig::WiFi::AP_NAME);
    Serial.println("Then navigate to: http://192.168.4.1");
    Serial.println("AP commands available:");
    Serial.println("  http://192.168.4.1/toggle - Toggle turbolift effect");
    Serial.println("  http://192.168.4.1/malfunction - Trigger malfunction");
    Serial.println("  http://192.168.4.1/fadeout - Fade out effect");
    Serial.println("  http://192.168.4.1/status - View status");
    Serial.println("  http://192.168.4.1/config - View configuration");
#endif
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
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED screen dimensions
int SCREEN_WIDTH = 128;
int SCREEN_HEIGHT = 32;

// I2C pins
int SCL_PIN = 22;
int SDA_PIN = 21;

// OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Text variables
int var1 = 123;
int var2 = 456;
int var3 = 789;

// Charging animation variables
bool chargerConnected = false;
int batteryLevel = 0;
int batteryBarWidth = 0;

// Timer variables
unsigned long chargerTimer = 0;
const unsigned long chargerOnTime = 30000;   // 30 seconds charger connected
const unsigned long chargerOffTime = 10000;  // 10 seconds charger disconnected

void setup() {
  // Initialize I2C bus
  Wire.begin(SDA_PIN, SCL_PIN);

  // Initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

  // Set the initial state of the charger
  setChargerState(false);
}

void loop() {
  // Check if it's time to switch the charger state
  unsigned long currentTime = millis();
  if (currentTime - chargerTimer >= (chargerConnected ? chargerOnTime : chargerOffTime)) {
    setChargerState(!chargerConnected);
  }

  // Clear the display
  display.clearDisplay();

  if (chargerConnected) {
    drawChargingAnimation();
  } else {
    drawBatteryText();
  }
}

void setChargerState(bool connected) {
  chargerConnected = connected;
  chargerTimer = millis();
  batteryLevel = 0;
  batteryBarWidth = 0;
}

void drawChargingAnimation() {
  // Draw charging animation
  batteryLevel = (batteryLevel + 1) % 101;

  // Draw battery shape icon
  int batteryWidth = SCREEN_WIDTH - 2;
  int batteryHeight = SCREEN_HEIGHT - 2;
  int batteryX = (SCREEN_WIDTH - batteryWidth) / 2;
  int batteryY = (SCREEN_HEIGHT - batteryHeight) / 2;
  int batteryCapWidth = 5;
  int batteryCapHeight = 15;
  int batteryBorderThickness = 2;

  display.drawRoundRect(batteryX, batteryY, batteryWidth-5, batteryHeight, 10, WHITE);
  display.drawRoundRect(batteryX + batteryBorderThickness, batteryY + batteryBorderThickness, batteryWidth - batteryBorderThickness*2, batteryHeight - batteryBorderThickness*2, 8, BLACK);
  display.drawRoundRect(batteryX + batteryWidth - batteryCapWidth - batteryBorderThickness, batteryY + batteryHeight / 2 - batteryCapHeight / 2 + batteryBorderThickness -2, batteryCapWidth, batteryCapHeight, 3, WHITE);
  //charging fill
  int fillX = batteryX;
  int fillY = batteryY;
  int fillWidth = map(batteryLevel, 0, 100, 0, batteryWidth - 6);
  int fillHeight = batteryHeight-1;
  
  display.fillRoundRect(fillX, fillY + 1, fillWidth, fillHeight, 10, WHITE);
  display.fillRoundRect(fillX, fillY, fillWidth, fillHeight, 10, WHITE);

  // Draw SOC text with border
  String soc = String(batteryLevel) + "%";
  int socWidth = soc.length() * 8;
  int socX = (SCREEN_WIDTH - socWidth) / 2;
  int socY = (SCREEN_HEIGHT) / 2 - 8;
  
  display.setTextColor(BLACK);
  display.setTextSize(2);
  display.setCursor(socX - 1, socY - 1);
  display.print(soc);
  display.setCursor(socX - 1, socY + 1);
  display.print(soc);
  display.setCursor(socX + 1, socY - 1);
  display.print(soc);
  display.setCursor(socX + 1, socY + 1);
  display.print(soc);
  display.setTextColor(WHITE);
  display.setCursor(socX, socY);
  display.print(soc);

  // Draw battery cap
  int capX = batteryX + batteryWidth - batteryCapWidth - batteryBorderThickness;
  int capY = batteryY + batteryHeight / 2 - batteryCapHeight / 2;
  display.fillRect(capX, capY, batteryCapWidth, batteryCapHeight, WHITE);
  
  display.display();
  delay(50);
}



void drawBatteryText() {
  // Draw text
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.print("col1 |");
  display.setCursor(36, 0);
  display.print("col2 |");
  display.setCursor(72, 0);
  display.print("col3");
  display.setCursor(0, 10);
  display.print(var1);
  display.setCursor(36, 10);
  display.print(var2);
  display.setCursor(72, 10);
  display.print(var3);
  display.display();
  delay(100);
}

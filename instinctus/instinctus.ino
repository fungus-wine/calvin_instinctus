// Calvin Instinctus - Teensy 4.1
// Hello World

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);  // Wait up to 3s for USB serial
    Serial.println("Hello from Calvin Instinctus!");
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.println("Hello from Calvin Instinctus!");
}

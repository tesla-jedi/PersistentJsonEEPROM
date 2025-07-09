#include <PersistentJsonEEPROM.h>
#include <ArduinoJson.h> // Necesitarás ArduinoJson para crear/parsear los JSON

// Usar la dirección I2C y tamaño por defecto para AT24C256
PersistentJsonEEPROM eeprom;
// O especificar: PersistentJsonEEPROM eeprom(0x51, 16384); // Para una EEPROM de 16Kbit en 0x51
//IMPORTANTE: cambiar puertos I2c a los correspondientes en tu placa de desarrollo
//#define SDApin 7
//#define SCLpin 8

void setup() {
  Serial.begin(115200);
  //Wire.begin(SDApin, SCLpin); 
  while (!Serial);
  Serial.println("PersistentJsonEEPROM Demo");

  if (eeprom.begin()) {
    Serial.println("EEPROM inicializada correctamente.");
    Serial.print("Mensajes almacenados: "); Serial.println(eeprom.data());
    Serial.print("Uso de memoria: "); Serial.print(eeprom.usage(), 2); Serial.println("%");
  } else {
    Serial.println("Error al inicializar EEPROM.");
    while (1); // Detener
  }

  // BORRAR TODO (descomentar para limpiar la EEPROM en cada ejecución de prueba)
  // Serial.println("Borrando todos los mensajes...");
  // eeprom.clearAll();
  // Serial.print("Mensajes después de borrar: "); Serial.println(eeprom.data());
  // Serial.print("Uso después de borrar: "); Serial.print(eeprom.usage(), 2); Serial.println("%");


  // --- Ejemplo de Append ---
  StaticJsonDocument<128> doc1;
  doc1["sensor"] = "temp1";
  doc1["value"] = 25.6;
  String jsonStr1;
  serializeJson(doc1, jsonStr1);

  Serial.println("Intentando añadir mensaje 1...");
  if (eeprom.append(jsonStr1)) {
    Serial.println("Mensaje 1 añadido.");
  } else {
    Serial.println("Error al añadir mensaje 1 (¿EEPROM llena?).");
  }
  Serial.print("Total mensajes: "); Serial.println(eeprom.data());
  Serial.print("Uso memoria: "); Serial.print(eeprom.usage(), 2); Serial.println("%");

  StaticJsonDocument<128> doc2;
  doc2["sensor"] = "hum1";
  doc2["value"] = 67.2;
  doc2["unit"] = "%";
  String jsonStr2;
  serializeJson(doc2, jsonStr2);
  
  Serial.println("Intentando añadir mensaje 2...");
  if (eeprom.append(jsonStr2)) {
    Serial.println("Mensaje 2 añadido.");
  } else {
    Serial.println("Error al añadir mensaje 2.");
  }
  Serial.print("Total mensajes: "); Serial.println(eeprom.data());
  Serial.print("Uso memoria: "); Serial.print(eeprom.usage(), 2); Serial.println("%");
  
  // --- Ejemplo de Read ---
  Serial.println("Leyendo mensaje 0:");
  String readJson0 = eeprom.read(0);
  if (readJson0.length() > 0) {
    Serial.println(readJson0);
    // Aquí podrías deserializar readJson0 con ArduinoJson si lo necesitas
  } else {
    Serial.println("Mensaje 0 no encontrado o vacío.");
  }

  Serial.println("Leyendo mensaje 1:");
  String readJson1 = eeprom.read(1);
  if (readJson1.length() > 0) {
    Serial.println(readJson1);
  } else {
    Serial.println("Mensaje 1 no encontrado o vacío.");
  }
  
  Serial.println("Leyendo mensaje 2 (debería fallar si solo hay 2 mensajes 0 y 1):");
  String readJson2 = eeprom.read(2);
  if (readJson2.length() > 0) {
    Serial.println(readJson2);
  } else {
    Serial.println("Mensaje 2 no encontrado o vacío.");
  }

  // --- Ejemplo de Delete ---
  // Supongamos que tenemos 2 mensajes (índices 0 y 1)
  // Si hacemos eeprom.Delete(1), se mantendrá el mensaje 0, y el contador será 1.
  // El puntero de uso se moverá al inicio de donde estaba el mensaje 1.
  Serial.println("Intentando Delete(1)... (mantendrá el mensaje 0, el contador será 1)");
  if (eeprom.Delete(1)) { // Mantiene 1 mensaje (el mensaje en el índice 0)
    Serial.println("Delete(1) exitoso.");
    Serial.print("Total mensajes después de Delete(1): "); Serial.println(eeprom.data()); // Debería ser 1
    Serial.print("Uso memoria después de Delete(1): "); Serial.print(eeprom.usage(), 2); Serial.println("%");

    Serial.println("Leyendo mensaje 0 (debería existir):");
    readJson0 = eeprom.read(0);
    if (readJson0.length() > 0) Serial.println(readJson0); else Serial.println("No encontrado.");
    
    Serial.println("Leyendo mensaje 1 (NO debería existir):");
    readJson1 = eeprom.read(1);
    if (readJson1.length() > 0) Serial.println(readJson1); else Serial.println("No encontrado.");

    Serial.println("Añadiendo un nuevo mensaje (debería ser el nuevo mensaje 1):");
    StaticJsonDocument<128> doc3;
    doc3["sensor"] = "light1";
    doc3["value"] = 780;
    String jsonStr3;
    serializeJson(doc3, jsonStr3);
    eeprom.append(jsonStr3);
    Serial.print("Total mensajes: "); Serial.println(eeprom.data()); // Debería ser 2
    Serial.print("Uso memoria: "); Serial.print(eeprom.usage(), 2); Serial.println("%");

    Serial.println("Leyendo mensaje 1 (el nuevo mensaje):");
    readJson1 = eeprom.read(1);
    if (readJson1.length() > 0) Serial.println(readJson1); else Serial.println("No encontrado.");

  } else {
    Serial.println("Delete(1) falló.");
  }
}

void loop() {
  // Nada aquí para este demo
  delay(5000);
}

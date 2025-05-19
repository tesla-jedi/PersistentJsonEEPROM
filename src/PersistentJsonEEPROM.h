#ifndef PERSISTENT_JSON_EEPROM_H
#define PERSISTENT_JSON_EEPROM_H

#include <Arduino.h>
#include <Wire.h>

// Valores por defecto para la AT24C256
#define PJ_EEPROM_DEFAULT_I2C_ADDR 0x50
#define PJ_EEPROM_DEFAULT_SIZE_BYTES 32768 // 32KB para AT24C256
#define PJ_EEPROM_WRITE_DELAY 5 // ms

// Direcciones internas fijas en la EEPROM para metadatos
#define PJ_ADDR_USAGE_POINTER 0 // 2 bytes para el puntero de uso
#define PJ_ADDR_MSG_COUNT_POINTER 2 // 2 bytes para el contador de mensajes
#define PJ_DATA_START_ADDRESS 4 // Los datos de usuario comienzan después de los metadatos

class PersistentJsonEEPROM {
public:
    PersistentJsonEEPROM(uint8_t i2cAddress = PJ_EEPROM_DEFAULT_I2C_ADDR, uint16_t eepromSizeBytes = PJ_EEPROM_DEFAULT_SIZE_BYTES);

    bool begin();
    bool append(const String& jsonData);
    uint16_t data(); // Devuelve el número de mensajes
    float usage(); // Devuelve el porcentaje de uso (0.0 - 100.0)
    bool Delete(uint16_t messageIndex); // Elimina desde el mensaje 'messageIndex' en adelante (0-indexed)
    String read(uint16_t messageIndex); // Lee el mensaje 'messageIndex' (0-indexed)

    // Funciones adicionales que podrían ser útiles
    bool clearAll(); // Borra todos los mensajes y resetea los punteros

private:
    uint8_t _i2cAddress;
    uint16_t _eepromSizeBytes;
    uint16_t _currentUsagePtr; // RAM copy of EEPROM usage pointer
    uint16_t _currentMsgCount; // RAM copy of EEPROM message count

    // Funciones I2C de bajo nivel para la EEPROM
    void _writeByte(uint16_t eeAddress, uint8_t val);
    uint8_t _readByte(uint16_t eeAddress);
    void _writeWord(uint16_t eeAddress, uint16_t val);
    uint16_t _readWord(uint16_t eeAddress);
    void _writeStringWithLength(uint16_t startAddress, const String& str, uint16_t& bytesWritten);
    String _readStringWithLength(uint16_t startAddress, uint16_t& bytesReadIncludingLength);

    bool _navigateToMessage(uint16_t messageIndex, uint16_t& messageStartAddress, uint16_t& messageDataLength);
    void _updatePersistentPointers();
};

#endif // PERSISTENT_JSON_EEPROM_H
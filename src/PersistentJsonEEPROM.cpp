#include "PersistentJsonEEPROM.h"

PersistentJsonEEPROM::PersistentJsonEEPROM(uint8_t i2cAddress, uint16_t eepromSizeBytes) {
    _i2cAddress = i2cAddress;
    _eepromSizeBytes = eepromSizeBytes;
    _currentUsagePtr = PJ_DATA_START_ADDRESS; // Valor inicial antes de leer de EEPROM
    _currentMsgCount = 0;                   // Valor inicial
}

bool PersistentJsonEEPROM::begin() {
    Wire.begin(7,9); // Inicializar I2C. Considerar Wire.begin(SDA, SCL) si no son pines por defecto.

    // Leer el puntero de uso desde la EEPROM
    _currentUsagePtr = _readWord(PJ_ADDR_USAGE_POINTER);
    // Leer el contador de mensajes desde la EEPROM
    _currentMsgCount = _readWord(PJ_ADDR_MSG_COUNT_POINTER);

    // Sanity checks para los valores leídos
    if (_currentUsagePtr == 0xFFFF || _currentUsagePtr < PJ_DATA_START_ADDRESS || _currentUsagePtr > _eepromSizeBytes) {
        // Puntero de uso inválido o EEPROM virgen, inicializar
        _currentUsagePtr = PJ_DATA_START_ADDRESS;
    }
    if (_currentMsgCount == 0xFFFF) { // 0xFFFF es 65535, improbable como contador válido inicial
        _currentMsgCount = 0;
    }
    
    // Si después de leer, el puntero de uso apunta al inicio pero hay un contador de mensajes > 0,
    // o si el contador es 0 pero el puntero no está al inicio, indica una posible inconsistencia.
    // Para simplificar, si el puntero está al inicio, reseteamos el contador.
    // Una lógica más robusta podría intentar validar/reconstruir.
    if (_currentUsagePtr == PJ_DATA_START_ADDRESS && _currentMsgCount > 0) {
        _currentMsgCount = 0;
    }
    if (_currentMsgCount == 0 && _currentUsagePtr != PJ_DATA_START_ADDRESS) {
        _currentUsagePtr = PJ_DATA_START_ADDRESS;
    }

    _updatePersistentPointers(); // Guardar los valores inicializados/validados si cambiaron

    //Serial.print("PJ_EEPROM: Begin. Usage Ptr: "); Serial.print(_currentUsagePtr);
    //Serial.print(", Msg Count: "); Serial.println(_currentMsgCount);
    return true;
}

bool PersistentJsonEEPROM::append(const String& jsonData) {
    uint16_t jsonLength = jsonData.length();
    uint16_t requiredSpace = sizeof(uint16_t) + jsonLength; // Espacio para longitud + datos

    if (_currentUsagePtr + requiredSpace > _eepromSizeBytes) {
        //Serial.println("PJ_EEPROM: Error append, no space.");
        return false; // No hay suficiente espacio
    }

    // Escribir longitud del JSON
    _writeWord(_currentUsagePtr, jsonLength);
    
    // Escribir datos del JSON
    uint16_t dataAddress = _currentUsagePtr + sizeof(uint16_t);
    for (uint16_t i = 0; i < jsonLength; ++i) {
        _writeByte(dataAddress + i, jsonData.charAt(i));
    }

    _currentUsagePtr += requiredSpace;
    _currentMsgCount++;
    _updatePersistentPointers();

    //Serial.print("PJ_EEPROM: Appended. New Usage Ptr: "); Serial.print(_currentUsagePtr);
    //Serial.print(", New Msg Count: "); Serial.println(_currentMsgCount);
    return true;
}

uint16_t PersistentJsonEEPROM::data() {
    return _currentMsgCount;
}

float PersistentJsonEEPROM::usage() {
    if (_eepromSizeBytes == 0) return 0.0;
    return ((float)_currentUsagePtr / _eepromSizeBytes) * 100.0;
}

String PersistentJsonEEPROM::read(uint16_t messageIndex) {
    uint16_t messageStartAddress;
    uint16_t messageDataLength;

    if (!_navigateToMessage(messageIndex, messageStartAddress, messageDataLength)) {
        return String(); // Índice inválido o error de navegación
    }

    // messageStartAddress ahora apunta al inicio de la longitud del mensaje deseado
    // messageDataLength contiene la longitud de los datos de ese mensaje
    
    if (messageDataLength == 0 || messageDataLength == 0xFFFF) return String(); // Mensaje vacío o inválido

    char* buffer = new char[messageDataLength + 1];
    if (!buffer) return String(); // No se pudo alocar memoria

    uint16_t actualDataStart = messageStartAddress + sizeof(uint16_t);
    for (uint16_t i = 0; i < messageDataLength; ++i) {
        buffer[i] = _readByte(actualDataStart + i);
    }
    buffer[messageDataLength] = '\0';

    //String result(buffer);
    String result(buffer, messageDataLength);
    delete[] buffer;
    return result;
}

bool PersistentJsonEEPROM::Delete(uint16_t messageIndex) {
    if (messageIndex > _currentMsgCount) {
         //Serial.println("PJ_EEPROM: Error delete, index out of bounds.");
        return false; // No se puede "borrar" más allá de los mensajes existentes
    }

    if (messageIndex == 0) { // Borrar todo
        _currentUsagePtr = PJ_DATA_START_ADDRESS;
        _currentMsgCount = 0;
    } else {
        uint16_t messageStartAddress;
        uint16_t messageDataLength; // No la necesitamos aquí, pero la función la devuelve
        
        // Navegar hasta el inicio del mensaje 'messageIndex'.
        // Este será el nuevo final de los datos.
        if (!_navigateToMessage(messageIndex, messageStartAddress, messageDataLength)) {
            //Serial.println("PJ_EEPROM: Error delete, failed to navigate to index.");
            return false; // No debería pasar si messageIndex <= _currentMsgCount
        }
        _currentUsagePtr = messageStartAddress; // El nuevo puntero de uso es el inicio de este mensaje
        _currentMsgCount = messageIndex;      // Mantenemos 'messageIndex' cantidad de mensajes (0 a messageIndex-1)
    }
    
    _updatePersistentPointers();
    //Serial.print("PJ_EEPROM: Deleted. New Usage Ptr: "); Serial.print(_currentUsagePtr);
    //Serial.print(", New Msg Count: "); Serial.println(_currentMsgCount);
    return true;
}

bool PersistentJsonEEPROM::clearAll() {
    _currentUsagePtr = PJ_DATA_START_ADDRESS;
    _currentMsgCount = 0;
    _updatePersistentPointers();
    //Serial.println("PJ_EEPROM: Cleared All.");
    return true;
}


// --- Private Helper Functions ---

void PersistentJsonEEPROM::_updatePersistentPointers() {
    _writeWord(PJ_ADDR_USAGE_POINTER, _currentUsagePtr);
    _writeWord(PJ_ADDR_MSG_COUNT_POINTER, _currentMsgCount);
}

bool PersistentJsonEEPROM::_navigateToMessage(uint16_t messageIndex, uint16_t& messageStartAddress, uint16_t& messageDataLength) {
    if (messageIndex >= _currentMsgCount) {
        return false; // Índice fuera de rango
    }

    uint16_t currentAddress = PJ_DATA_START_ADDRESS;
    for (uint16_t i = 0; i < messageIndex; ++i) {
        if (currentAddress + sizeof(uint16_t) > _currentUsagePtr) return false; // Consistencia
        uint16_t len = _readWord(currentAddress);
        if (len == 0xFFFF || len == 0) return false; // Error de lectura o mensaje vacío inesperado
        currentAddress += sizeof(uint16_t) + len;
        if (currentAddress > _currentUsagePtr) return false; // Consistencia
    }
    
    // Ahora currentAddress es el inicio de la longitud del mensaje 'messageIndex'
    if (currentAddress + sizeof(uint16_t) > _currentUsagePtr) return false; // No hay espacio ni para leer la longitud

    messageStartAddress = currentAddress;
    messageDataLength = _readWord(currentAddress);
    if (messageDataLength == 0xFFFF) return false; // Error al leer la longitud del mensaje objetivo

    // Chequeo final: que el mensaje completo esté dentro del área usada
    if (messageStartAddress + sizeof(uint16_t) + messageDataLength > _currentUsagePtr) {
        //Serial.println("PJ_EEPROM: Nav Consistency check failed.");
        return false;
    }

    return true;
}

// --- Low-level I2C EEPROM R/W ---
void PersistentJsonEEPROM::_writeByte(uint16_t eeAddress, uint8_t val) {
    Wire.beginTransmission(_i2cAddress); // Usa el miembro _i2cAddress de la clase
    Wire.write((uint8_t)(eeAddress >> 8));   // MSB de la dirección
    Wire.write((uint8_t)(eeAddress & 0xFF)); // LSB de la dirección
    Wire.write(val);                         // Dato
    byte status = Wire.endTransmission();    // Finalizar transmisión y obtener estado
    
    if (status != 0) {
        Serial.print("!! I2C Write Error en _writeByte (PJ) !! Status: "); Serial.println(status);
        Serial.print("   Dirección EEPROM: 0x"); Serial.print(eeAddress, HEX);
        Serial.print(", Valor: 0x"); Serial.println(val, HEX);
        // Status comunes:
        // 1: data too long to fit in transmit buffer
        // 2: received NACK on transmit of address
        // 3: received NACK on transmit of data
        // 4: other error
    }
    // else { Serial.print("_writeByte OK (PJ): Addr 0x"); Serial.print(eeAddress, HEX); Serial.print(" Val 0x"); Serial.println(val, HEX); }
    delay(PJ_EEPROM_WRITE_DELAY); // Usa la constante correcta de tu librería
}

// Asegúrate de que el nombre de la clase sea PersistentJsonEEPROM
uint8_t PersistentJsonEEPROM::_readByte(uint16_t eeAddress) {
    uint8_t rdata = 0xFF; // Valor por defecto en caso de error

    Wire.beginTransmission(_i2cAddress); // Usa el miembro _i2cAddress de la clase
    Wire.write((uint8_t)(eeAddress >> 8));   // MSB de la dirección
    Wire.write((uint8_t)(eeAddress & 0xFF)); // LSB de la dirección
    byte status = Wire.endTransmission();    // Finalizar transmisión (necesario antes de requestFrom)
    
    if (status != 0) {
        Serial.print("!! I2C Read Error (fase de set dirección) en _readByte (PJ) !! Status: "); Serial.println(status);
        Serial.print("   Dirección EEPROM: 0x"); Serial.println(eeAddress, HEX);
        return 0xFE; // Un código de error distinto para diferenciar de "no data"
    }

    uint8_t bytesRequested = 1;
    uint8_t bytesReceived = Wire.requestFrom((int)_i2cAddress, (int)bytesRequested); // Solicitar 1 byte

    if (bytesReceived == bytesRequested) {
        if (Wire.available()) { // Debería haber un byte disponible
            rdata = Wire.read();
            // Serial.print("_readByte OK (PJ): Addr 0x"); Serial.print(eeAddress, HEX); Serial.print(" Val 0x"); Serial.println(rdata, HEX);
        } else {
            Serial.print("!! I2C Read Error en _readByte (PJ): Wire.available() es false después de requestFrom exitoso. Addr: 0x");
            Serial.println(eeAddress, HEX);
            rdata = 0xFD; // Otro código de error
        }
    } else {
        Serial.print("!! I2C Read Error en _readByte (PJ): No se recibieron los bytes solicitados. Solicitados: "); 
        Serial.print(bytesRequested); Serial.print(", Recibidos: "); Serial.print(bytesReceived);
        Serial.print(". Addr: 0x"); Serial.println(eeAddress, HEX);
        // rdata permanece 0xFF, que es el error genérico de lectura fallida
    }
    return rdata;
}



void PersistentJsonEEPROM::_writeWord(uint16_t eeAddress, uint16_t val) {
    _writeByte(eeAddress, (uint8_t)(val >> 8));   // MSB
    _writeByte(eeAddress + 1, (uint8_t)(val & 0xFF)); // LSB
}

uint16_t PersistentJsonEEPROM::_readWord(uint16_t eeAddress) {
    uint8_t msb = _readByte(eeAddress);
    uint8_t lsb = _readByte(eeAddress + 1);
    return (uint16_t)((msb << 8) | lsb);
}
PersistentJsonEEPROM

Una librería de Arduino para almacenar y administrar cadenas de datos JSON de forma persistente en una EEPROM I2C externa (como la AT24C256).

Esta librería permite guardar mensajes o configuraciones en formato JSON, leerlos individualmente, y gestiona el uso del espacio en la EEPROM.

Características

* Almacena cadenas JSON en la EEPROM.
* Lleva un registro del número de mensajes almacenados.
* Calcula el porcentaje de uso de la EEPROM.
* Permite leer mensajes específicos por su índice.
* Permite eliminar mensajes a partir de un índice específico (marcando el espacio como disponible).
* Permite borrar todo el contenido de la EEPROM gestionado por la librería.
* Manejo de metadatos (puntero de uso y contador de mensajes) persistentes en la EEPROM.
* Configurable para diferentes direcciones I2C y tamaños de EEPROM.

Dependencias

* Wire.h: Para la comunicación I2C con la EEPROM.
* Arduino.h: Librería estándar de Arduino.

Instalación

1.  Descarga esta librería (o clona el repositorio).
2.  Ve al menú "Programa" -> "Incluir Librería" -> "Añadir biblioteca .ZIP" en el IDE de Arduino.
3.  Selecciona el archivo .ZIP que descargaste y la librería se instalará.
4.  También puedes copiar manualmente la carpeta de la librería a tu carpeta de `libraries` de Arduino (usualmente en `Documentos/Arduino/libraries`).

Cómo Usar

Aquí tienes un ejemplo básico de cómo usar la librería:

```cpp
#include <Wire.h>
#include <PersistentJsonEEPROM.h>

// Configura la dirección I2C de tu EEPROM y su tamaño en bytes.
// Por defecto usa 0x50 y 32768 bytes (para una AT24C256).
// PersistentJsonEEPROM eepromJson(0x50, 32768);
PersistentJsonEEPROM eepromJson; // Usará los valores por defecto

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Espera a que el puerto serial se conecte
  }

  // Inicializa la librería y la comunicación I2C.
  // Es importante llamar a Wire.begin() antes si usas pines no estándar para I2C,
  // o puedes pasar los pines SDA y SCL a eepromJson.begin(SDA_PIN, SCL_PIN)
  // La implementación actual en el .cpp llama a Wire.begin(7,9); asegúrate que esto es correcto para tu hardware.
  if (eepromJson.begin()) {
    Serial.println("Librería PersistentJsonEEPROM inicializada.");
  } else {
    Serial.println("Error al inicializar la librería PersistentJsonEEPROM.");
    while (1); // Detener si hay error
  }

  // Borrar todos los mensajes existentes (opcional, para empezar limpio)
  // eepromJson.clearAll();
  // Serial.println("EEPROM borrada.");

  // Añadir algunos datos JSON
  String jsonData1 = "{\"sensor\":\"temperatura\",\"valor\":25.5}";
  if (eepromJson.append(jsonData1)) {
    Serial.println("Dato 1 añadido.");
  } else {
    Serial.println("Error al añadir dato 1 (¿EEPROM llena?).");
  }

  String jsonData2 = "{\"sensor\":\"humedad\",\"valor\":60.2}";
  if (eepromJson.append(jsonData2)) {
    Serial.println("Dato 2 añadido.");
  } else {
    Serial.println("Error al añadir dato 2.");
  }

  // Mostrar información
  Serial.print("Número de mensajes almacenados: ");
  Serial.println(eepromJson.data()); //

  Serial.print("Porcentaje de uso de la EEPROM: ");
  Serial.print(eepromJson.usage()); //
  Serial.println("%");

  // Leer los mensajes
  for (uint16_t i = 0; i < eepromJson.data(); i++) { //
    Serial.print("Mensaje [");
    Serial.print(i);
    Serial.print("]: ");
    String mensaje = eepromJson.read(i); //
    if (mensaje.length() > 0) {
      Serial.println(mensaje);
    } else {
      Serial.println("Error al leer mensaje o mensaje vacío.");
    }
  }

  // Ejemplo de borrado: borrar desde el mensaje índice 1 en adelante
  // (esto mantendría solo el mensaje 0)
  /*
  if (eepromJson.Delete(1)) { //
    Serial.println("Mensajes desde el índice 1 borrados.");
    Serial.print("Nuevo número de mensajes: ");
    Serial.println(eepromJson.data());
    Serial.print("Nuevo uso de EEPROM: ");
    Serial.print(eepromJson.usage());
    Serial.println("%");
  }
  */
}

void loop() {
  // Tu código principal aquí
}
```

API de Funciones

La clase principal es `PersistentJsonEEPROM`.

Constructor

PersistentJsonEEPROM(uint8_t i2cAddress = PJ_EEPROM_DEFAULT_I2C_ADDR, uint16_t eepromSizeBytes = PJ_EEPROM_DEFAULT_SIZE_BYTES);

* `i2cAddress` (opcional): La dirección I2C de la EEPROM. Por defecto es `0x50`.
* `eepromSizeBytes` (opcional): El tamaño total en bytes de la EEPROM. Por defecto es `32768` (32KB para AT24C256).

Métodos Públicos

* bool begin()
    * Inicializa la librería y la comunicación I2C. Lee los metadatos (puntero de uso y contador de mensajes) desde la EEPROM.
    * Debe ser llamada en la función `setup()`.
    * La implementación actual en `PersistentJsonEEPROM.cpp` llama a `Wire.begin(7,9)`. Asegúrate de que esto es correcto para tu configuración de hardware o modifica/inicializa `Wire` antes.
    * Devuelve `true` si la inicialización fue exitosa, `false` en caso contrario.

* bool append(const String& jsonData)
    * Añade una cadena JSON a la EEPROM.
    * `jsonData`: La cadena `String` en formato JSON a almacenar.
    * Devuelve `true` si el dato fue añadido correctamente, `false` si no hay espacio suficiente o si ocurre un error.

* uint16_t data()
    * Devuelve el número actual de mensajes JSON almacenados en la EEPROM.

* float usage()
    * Devuelve el porcentaje de espacio utilizado en la EEPROM (0.0 a 100.0).

* String read(uint16_t messageIndex)
    * Lee un mensaje JSON específico de la EEPROM.
    * `messageIndex`: El índice del mensaje a leer (0-indexed).
    * Devuelve una `String` con el JSON leído. Si el índice es inválido o hay un error, devuelve una cadena vacía.

* bool Delete(uint16_t messageIndex)
    * "Elimina" mensajes desde el `messageIndex` especificado en adelante. Lo que realmente hace es mover el puntero de uso actual al inicio del `messageIndex`, marcando el espacio subsecuente como disponible para ser sobrescrito. Los mensajes anteriores al `messageIndex` se conservan.
    * `messageIndex`: El índice del primer mensaje a "eliminar" (0-indexed). Si es `0`, se borran todos los mensajes (similar a `clearAll()`).
    * Devuelve `true` si la operación fue exitosa, `false` si el índice está fuera de rango o hay un error.

* bool clearAll()
    * Borra todos los mensajes almacenados reseteando el puntero de uso y el contador de mensajes. Esto no borra físicamente los bytes de la EEPROM, pero los marca como disponibles para ser sobrescritos.
    * Devuelve `true` si la operación fue exitosa.

Archivo `keywords.txt`

Este archivo se utiliza para el resaltado de sintaxis en el IDE de Arduino.
Contiene las siguientes palabras clave:
* `PersistentJsonEEPROM` (KEYWORD1)
* `begin` (KEYWORD2)
* `append` (KEYWORD2)
* `data` (KEYWORD2)
* `usage` (KEYWORD2)
* `Delete` (KEYWORD2)
* `read` (KEYWORD2)
* `clearAll` (KEYWORD2)

Consideraciones Internas (Avanzado)

* Metadatos: La librería reserva los primeros bytes de la EEPROM para metadatos:
    * `PJ_ADDR_USAGE_POINTER` (0x00 - 0x01): Almacena un `uint16_t` que apunta al siguiente byte disponible para escritura.
    * `PJ_ADDR_MSG_COUNT_POINTER` (0x02 - 0x03): Almacena un `uint16_t` con el número total de mensajes.
    * `PJ_DATA_START_ADDRESS` (0x04): Los datos JSON de usuario comienzan a partir de esta dirección.
* Formato de Mensaje: Cada mensaje JSON se almacena precedido por un `uint16_t` que indica la longitud de la cadena JSON.
* Escritura en EEPROM: Se utiliza un pequeño retardo (`PJ_EEPROM_WRITE_DELAY`, por defecto 5ms) después de cada escritura de byte para asegurar la correcta escritura en la EEPROM.

Contribuciones

Las contribuciones son bienvenidas. Por favor, abre un *issue* para discutir cambios mayores o reportar errores. Los *Pull Requests* también son bienvenidos.

Licencia

Por favor, añade tu licencia preferida aquí (ej. MIT, Apache 2.0, GPL, etc.). Si no se especifica, se considera código propietario.

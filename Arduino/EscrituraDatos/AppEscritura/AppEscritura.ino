#include <SPI.h>//https://www.arduino.cc/en/reference/SPI
#include <MFRC522.h>//https://github.com/miguelbalboa/rfid


//Constants
#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);


// Clave de cifrado actuales
/*
MFRC522::MIFARE_Key keyA = {keyByte: {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}};
MFRC522::MIFARE_Key keyB = {keyByte: {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}};
*/
MFRC522::MIFARE_Key keyA = { keyByte: { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };
MFRC522::MIFARE_Key keyB = { keyByte: { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };

// Número de viajes array 16 bytes
//byte datosBloque[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// Datos del sector
byte sector = 2;  // se van a modificar según se vayan rellenando, dejar constantes??
//byte numBlque = 8;
byte bloqueTrailer = 11;
byte numViajes = 10;
const byte tamBloque = 16;

byte tagID = 0xD1;
//byte datosID[4] = { 0x00, 0x00, 0x00, 0x01 };
byte tagCargo = 0xD2;
//byte datosCargo = 0x00;
byte tagNombre = 0xD3;
//byte datosNombre[7] = { 0x47, 0x6F, 0x6E, 0x7A, 0x61, 0x6C, 0x6F };

const int TAM_ID = 4;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SPI.begin();      // Iniciar bus SPI
  rfid.PCD_Init(); 
  delay(5000);
  while (!Serial){
    ;
  }
  Serial.println("Aplicacion para introudcir datos en la tarjeta");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Introduzca sus credenciales de administrador (exit para salir)");
  while (Serial.available() == 0) {}  
  String credentials;
  credentials = Serial.readString();
  credentials.trim();
  if(credentials == "Gonzalo"){
      Serial.print("Acceso concedido Mister ");
      Serial.println(credentials);
      
      int datos_id[TAM_ID];
      int datos_cargo = -1;
      String nombre;
      byte * name_hex;
      bool bool_array_bloques[3] = {true,false,false};
      byte datosBloque[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
      byte datosBloqueName1[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
      byte datosBloqueName2[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

      Serial.println("Introduzca los siguientes datos, por favor");
      Serial.println("1.- Identificador unico del usuario (4Bytes) ");
      /*int len = Serial.readBytes(datos_id, TAM_ID);
      for(int i = 0; i < len ; i ++){
        Serial.print(datos_id[i], HEX);
        Serial.print(" ");
      }*/
      for(int i = 0; i < TAM_ID; i++){
        while (!Serial){;}  
        Serial.print("Introduce el byte ");
        Serial.print(i);
        Serial.print(" ");
        while (Serial.available() == 0) {}  
        datos_id[i] = Serial.parseInt();
        Serial.println(datos_id[i], HEX);
      }    
      Serial.println("");
      while(datos_cargo < 0 || datos_cargo > 1){
        Serial.println("2.- Cargo del usuario (0: profesor | 1: alumno)");
        while (Serial.available() == 0) {}  
        datos_cargo = Serial.parseInt();    
            
      }
      if(datos_cargo == 0){
        Serial.println("Profesor");
      }
      else{
        Serial.println("Alumno");
      }
      Serial.println("2.- Nombre del usuario");
      while (Serial.available() == 0) {} 
      nombre = Serial.readString();           
      Serial.println(nombre);
      int name_size = nombre.length();
      name_hex = (byte *)malloc(sizeof(byte) * name_size);
      stringToHex(nombre, name_hex, name_size);    
      for(int i = 0; i < name_size; i++){
        Serial.print(name_hex[i]);
        Serial.print(" ");
      }
      Serial.println("");
      datosBloque[0] = tagID;
      for (int i = 1; i < 5; i++) {
      datosBloque[i] = datos_id[i - 1];
      }
      
      datosBloque[5] = tagCargo;
      datosBloque[6] = datos_cargo;
      datosBloque[7] = tagNombre;
      datosBloque[8] = name_size;
      int size_aux = 0;
      for(int i = 9; i < 15 && size_aux < name_size; i++){
        datosBloque[i] = name_hex[size_aux];
        size_aux++;
      }
      for(int i = 0; i < 15 && size_aux < name_size; i++){
        datosBloqueName1[i] = name_hex[size_aux];
        size_aux++;
        bool_array_bloques[1] = true;
      }
      for(int i = 0; i < 15 && size_aux < name_size; i++){
        datosBloqueName2[i] = name_hex[size_aux];
        size_aux++;
        bool_array_bloques[2] = true;
      }
      

      
      /*for (int i = 8; i < 15; i++) {
        datosBloque[i] = name_hex[i - 8];
      } */   
      
      Serial.println(F("Acerca la tarjeta al lector para escanear...."));

      rfid.PCD_Init(); 
      delay(1000);

      while (!rfid.PICC_IsNewCardPresent()) {
      }
      // Si hay una tarjeta cerca, que la eleccione
      // En caso contrario que no continúe
      if (!rfid.PICC_ReadCardSerial()) {
        Serial.println("Entro en el return");
        return;
      }
      // Mostrar información de la tarjeta por el monitor serie
      Serial.print(F("UID de la tarjeta:"));
      mostrarByteArray(rfid.uid.uidByte, rfid.uid.size);  // Motrar el UID
      Serial.println();
      Serial.print(F("Tipo de tarjeta: "));
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);  //Motrar el tipo
      Serial.println(rfid.PICC_GetTypeName(piccType));   
      
      int resultadoEb;
      for(int i = 0; i < 3; i++){
        if(bool_array_bloques[i] == true){
          switch(i){
            case 0:
              resultadoEb = escribirBloque(datosBloque,i + 8);
            break;
            case 1:
              resultadoEb = escribirBloque(datosBloqueName1,i + 8);
            break;
            case 2:
              resultadoEb = escribirBloque(datosBloqueName2,i + 8);
            break;
          }
          // Dependiendo del resultado
          if (resultadoEb == 1) {
            Serial.println(F("No se puede escribir en un bloque Trailer"));
          } 
          else if (resultadoEb == 2) {
            Serial.println(F("No se puede escribir en un bloque del fabricante"));
          } 
          else if (resultadoEb == 3) {
            Serial.println(F("Problemas al comunicar con la clave proporcionada"));
          } 
          else if (resultadoEb == 4) {
            Serial.println(F("Problemas al escribir en el bloque"));
          } 
          else {
            Serial.println(F("Se han guardado los datos correctamente"));
            byte * datosLectura;
            datosLectura = (byte *)malloc(sizeof(byte) * (tamBloque+2));
            byte readCheck = lecturaBloque(datosLectura, i + 8);
            Serial.println("");
            if(readCheck <= 2){
              Serial.println("Error en la lectura");
            }
            
            else{
              byte idLeido[4];
              byte cargoLeido;
              byte nombreLeido[7];
              for(int i = 1; i < 5;i++){
                idLeido[i-1] = datosLectura[i];
              }
              cargoLeido = datosLectura[6];
              for(int j = 8; j < 16; j++){
                nombreLeido[j-8] = datosLectura[j];
              }
              free(datosLectura);   
            }
          }
        }
      }
      
    } 
    else if(credentials == "exit"){
      //exit(0);    
    }
}

void stringToHex(String name, byte * hex, int len){
  for(int j = 0; j < len; j++){
    hex[j] = byte(name[j]);
  }
}

byte escribirBloque(byte datosBloque[], int numBloque) {
  
  MFRC522::StatusCode estado;

  Serial.println("DatosBloque[1]");
  Serial.println(datosBloque[1]);

  // Comenzar comunicación cifrada con Key-A
  estado = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloqueTrailer, &keyA, &(rfid.uid));
  if (estado != MFRC522::STATUS_OK) {
    return 3;
  }
  
  estado = rfid.MIFARE_Write(numBloque, datosBloque, 16);
  if (estado != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_Write() fallo: ");
    Serial.println(rfid.GetStatusCodeName(estado));
    return 4;
  }

  return 0;
}

void mostrarByteArray(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    /*switch (buffer[i]) {
      case 0xD1:
        Serial.println("El id del usuario es: ");
        break;
      case 0xD2:
        Serial.println("El cargo del usuario es: ");
        break;
      case 0xD3:
        Serial.println("El nombre del usuario es: ");
        break;
    }*/
    Serial.print(buffer[i], HEX);
  }
}

byte lecturaBloque(byte * datosLectura, int numBloque) {
  MFRC522::StatusCode estado;
  //byte datosLectura[18];  // por qué 18, no serán 16??tamBloque
  byte tamBuffer = (sizeof(uint8_t))*(tamBloque + 2);

  // Comenzar comunicación cifrada con Key-A
  estado = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloqueTrailer, &keyA, &(rfid.uid));
  if (estado != MFRC522::STATUS_OK) {
    return 1;
  }

  // Leer bloque
  estado = rfid.MIFARE_Read(numBloque, datosLectura, &tamBuffer);
  if (estado != MFRC522::STATUS_OK) {
    Serial.print("MIFARE_Read() fallo: ");
    Serial.println(rfid.GetStatusCodeName(estado));
    return 2;
  }

  mostrarByteArray(datosLectura, tamBuffer);

  return datosLectura[0];
}

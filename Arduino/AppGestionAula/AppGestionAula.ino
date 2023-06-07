//Libraries
#include <SPI.h>//https://www.arduino.cc/en/reference/SPI
#include <MFRC522.h>//https://github.com/miguelbalboa/rfid
#include <WiFi.h>
#include <WiFiManager.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <GestionAulaAux.h>
#include <oledFunctions.h>
#include "httpSecure.h"

// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

char jsonOutput[128];
String id_sala;

// Clave de cifrado actuales
/*
MFRC522::MIFARE_Key keyA = {keyByte: {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5}};
MFRC522::MIFARE_Key keyB = {keyByte: {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5}};
*/
MFRC522::MIFARE_Key keyA = { keyByte: { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };
MFRC522::MIFARE_Key keyB = { keyByte: { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } };

//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

WiFiClientSecure client;

void setup() {
  //Init Serial USB
  Serial.begin(115200);
  Serial.println(F("Initialize System"));

  //init rfid D8,D5,D6,D7
  SPI.begin();
  rfid.PCD_Init();

  WiFiManager wm;
  bool res;
  res = wm.autoConnect("AutoConnectAP"); // password protected ap
  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  }
  else{
    String mac, body, sala;
    int body_len;
    Serial.print("ESP Board IP Address:  ");
    Serial.println(WiFi.localIP());
    Serial.println("\nStarting connection to server...");
    client.setCACert(test_root_ca);
    if (!client.connect(https_server, 443)) {
      Serial.println("Connection failed!");
    }
    else{
      Serial.println("Connected to server!");
      // Data to send with HTTP POST
      DynamicJsonDocument doc(1024);
      mac = WiFi.macAddress();
      doc["mac"] = mac;
      serializeJson(doc, jsonOutput);   
      body = String(jsonOutput);
      sala = httpsPost(body, server_Prov);
      if(sala == "400"){
        Serial.println("ERROR, NODO INVÁLIDO");
        id_sala = "-1";
      }
      Serial.println("Recibido" + sala);
      id_sala = sala;
    }
      
    client.stop();
  }

  Serial.print(F("Reader :"));
  rfid.PCD_DumpVersionToSerial();

  // initialize OLED display with I2C address 0x3C
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }
  oled.setCursor(0, 0);  
}


void loop() {
  
  //Declaracion de variables
  byte * datosLectura;
  int idLeido[4];
  //byte * idLeido;
  byte cargoLeido;
  char * nombreLeido;
  int nameSize,posName, auxSize, iteracion, i, checkResponse;
  String httpResponseCode, body; 
  long long int idDefinitivo;

  //Inicio programa
  Serial.println(F("Acerca la tarjeta al lector para escanear...."));
  oledDisplayMsg("BIENVENIDO, ACERQUE ","SU TARJETA","",oled);
  
  while (!rfid.PICC_IsNewCardPresent()) {
  }
  // Si hay una tarjeta cerca se lee.
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Entro en return para salir");
    return;
  }

  Serial.println("Leyendo datos...");
  
  
  datosLectura = (byte *)malloc(sizeof(byte) * (TAM_BLOQUE+2));
  byte readCheck = lecturaBloque(datosLectura, BLOQUE_INICIAL);
  if(readCheck > 0 && readCheck < 3){
    Serial.println("Error en la lectura");
  }
  else if(readCheck == 3){
    Serial.println("Error en el formato de los datos");
  }
  //Si la lectura ha sido correcta, se guardan los datos
  else if (readCheck == 0){
    //Lectura de los 4 bytes del id del usuario
    i = 1;
    for(i = 1; i < 1 + TAM_ID;i++){ 
      idLeido[i-1] = int(datosLectura[i]);
    }
    idDefinitivo = convertID(idLeido);
    //Lectura del cargo del usuario
    cargoLeido = datosLectura[i + 1];
    //Lectura del tamaño del nombre del usuario
    nameSize = datosLectura[i + TAM_CARGO + 2]; //se excluyen los tags
    nombreLeido = (char *)malloc(sizeof(char)*nameSize);
    auxSize = nameSize;
    posName = 0;
    //Lectura de la parte del nombre correspondiente al primer bloque
    for(int j = 9; j < TAM_BLOQUE - 1; j++){
      nombreLeido[posName] = char(datosLectura[j]);
      auxSize--;
      posName++;
    }
    Serial.print("Tamanyo nombre restante = ");
    Serial.println(nameSize);
    iteracion = 1;
    //Leer el resto del nombre (Maximo 2 bloques mas)
    while(auxSize > 0 && iteracion < 3){
      readCheck = lecturaBloque(datosLectura, BLOQUE_INICIAL + iteracion);
      for(int j= 0; j < (TAM_BLOQUE - 1) && auxSize >= 0; j++){        
        nombreLeido[posName] = char(datosLectura[j]);
        auxSize--;
        posName++;
      }
      iteracion++;
    }
    free(datosLectura); 
    //oledDisplayMsg("LEYENDO...", "ESPERE, POR FAVOR", "");

    
  }
  if (!client.connect(https_server, 443)) {
    Serial.println("Connection failed!");
  }
  else{ 
    
      // Data to send with HTTP POST
      DynamicJsonDocument doc(1024);
      doc["idtarjeta"] = idDefinitivo;
      doc["cargo"] = cargoLeido;
      doc["nombre"] = nombreLeido;       
      doc["sala"] = id_sala;    
      serializeJson(doc, jsonOutput);      
      
      
      // Send HTTP POST request
      body = String(jsonOutput);
      httpResponseCode = httpsPost(body, server_App);
      checkResponse = httpResponseCode.toInt();
      switch(checkResponse){
        case 290:
        case 291:
          Serial.println("Acceso permitido, OK " + String(nombreLeido));
          oledDisplayImage(0, oled);
        break;
        case 390:
        case 391:
        case 392:
          Serial.println("Acceso permitido con retraso " + String(nombreLeido));
        case 393:
          Serial.println("Profesor no imparte " + String(nombreLeido));
        
        oledDisplayImage(0, oled);
        break;

        case 490:
          Serial.println("Alumno no matriculado, WRONG" + String(nombreLeido));
        case 491:
          Serial.println("Sala sin horario docente" + String(nombreLeido));
        
        oledDisplayImage(1, oled);
        break;

        default:
          Serial.println("Error en la conexion con el servidor");
          //oledDisplayMsg("ERROR.", "SERVER NO DISPONIBLE, ",nombreLeido);
          oledDisplayImage(1,oled);
      }
      
        
      // Free resources
      client.stop();
    }  
    //Detener el lector
    rfid.PICC_HaltA();
    // Detener la encriptación Crypto1
    rfid.PCD_StopCrypto1();
}



String httpsPost(String body, String server){
  int body_length;
  String res;

  body_length = body.length();
  client.println("POST " + server + " HTTP/1.0");
  //Headers
  client.println("Content-Type: application/json");
  client.print("Content-Length: "); 
  client.println(body_length);
  client.println("Connection: Close");
  client.println();
  //Body
  client.println(body);
  client.println();

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.print(line);
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  if(client.available()){
    res = client.readString();
  }
  Serial.println(res);
  return res;
}

byte lecturaBloque(byte * datosLectura, int numBloque) {
  MFRC522::StatusCode estado;
  //byte datosLectura[18];  // por qué 18, no serán 16??tamBloque
  byte tamBuffer = (sizeof(uint8_t))*(TAM_BLOQUE) + 2;

  // Comenzar comunicación cifrada con Key-A
  estado = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, BLOQUE_TRAILER, &keyA, &(rfid.uid));
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

  if(checkTags(datosLectura) == true){
    return 0;
  }
  else{
    return 3;
  }
}


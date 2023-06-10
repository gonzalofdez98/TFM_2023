#include "GestionAulaAux.h"

void mostrarByteArray(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize - 2; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    switch (buffer[i]) {
      case 0xD1:
        Serial.println("El id del usuario es: ");
        break;
      case 0xD2:
        Serial.println("El cargo del usuario es: ");
        break;
      case 0xD3:
        Serial.println("El nombre del usuario es: ");
        break;
    }
    Serial.print(buffer[i], HEX);
  }
}

long long int convertID(int * id) {
  long long int resultado = 0;
  for(int i = 3; i > 0; i--){
    Serial.println(id[i]);
    resultado += id[i] * pow(1000, 4 - (i+1));
    Serial.println(resultado);
  }
  return resultado;
}

bool checkTags(byte * datosLectura){
  if(datosLectura[0] != TAG_ID){
    return false;
  }
  if(datosLectura[5] != TAG_CARGO){
    return false;
  }
  if(datosLectura[7] != TAG_NOMBRE){
    return false;
  }
  return true;
}


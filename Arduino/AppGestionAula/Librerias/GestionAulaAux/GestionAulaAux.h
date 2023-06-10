#ifndef GESTIONAULAAUX_H
#define GESTIONAULAAUX_H

#include <Arduino.h>

//Constants
#define SS_PIN 5
#define RST_PIN 0
#define TAM_ID 4
#define TAM_BLOQUE 16
#define TAM_CARGO 1

const byte TAG_ID = 0xD1;
const byte TAG_CARGO = 0xD2;
const byte TAG_NOMBRE = 0xD3;
//const String ID_SALA = "5";
const int BLOQUE_INICIAL = 8;
const byte BLOQUE_TRAILER = 11;

void mostrarByteArray(byte* buffer, byte bufferSize);
long long int convertID(int * id);
bool checkTags(byte * datosLectura);

#endif
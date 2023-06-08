# TFM_2023
Este es el repositorio del Trabajo de Fin de Master "Automatización del seguimiento docente utilizando tarjetas inteligentes sin contacto",
desarrollado por Gonzalo Fernández Megía, alumno del Master de Internet de las Cosas de la Facultad de Informática de la UCM.

En la carpeta "Arduino" se encuentra todo el código en lenguaje Arduino y que se carga en la placa ESP32. A su vez, está dividida
en dos carpetas. En la carpeta AppGestionAula se almacena la aplicación cliente principal que ejecuta un bucle infinito para el
seguimiento docente en las aulas. La carpeta "librerias" contiene las librerías que han sido desarrolladas para extender su su funcionalidad,
y su contenido ha de ser colocado en el  directorio donde se haya realizado la instalación de Arduino, y en concreto, bajo la carpeta "libraries".
La librería "GestionAulaAux" contiene constantes y funciones relacionadas con la lectura de datos de la memoria de una tarjeta Mifare Classic 1k. 
En cuanto a la librería "OLEDFunctions", se utiliza para la configuración e impresión de contenido a través de la pantalla oled.
Por otro lado, la carpeta "EscrituraDatos" contiene un proyecto auxiliar cuya única finalidad es la escritura de datos en una tarjeta  
para simular la estructura de memoria de una Tarjeta Universitaria Inteligente utilizada por el personal vinculado a la UCM. 
Para la ejecución de estos debe abrirse el archivo con extensión ".ino" en el entorno Arduino IDE. Tras haber instalado las librerías MFRC522 de GitHubCommunity, WiFIManager y ADAFruit SSD1303 de Adafruit, basta con compilarlo y cargarlo en la placa mediante los mecanismos que proporciona el entorno.

En la carpeta "Servidor" está el código en Node JS que se utiliza para levantar el servidor seguro. Se trata de un único fichero con 
extensión ".js" que se ejecuta mediante la consola de comandos con la siguiente instrucción: "node secureServer.js". Una vez ejecutado,
se mantendrá activo hasta que se pare su ejecución de manera intencionada mediante el atajo "CTRL+C" que matará dicho proceso, o exista
algún error fatal en el servidor.

La carpeta "ReporteIncidencias" incluye el código en Python del script utilizado para generar un reporte de las incidencias ocurridas en el seguimiento de la asistencia docente del profesorado. Para poder ejecutarlo, es necesario instalar la versión 3 de Python. Existen dos modos para la generación de reportes:
* Reporte de las incidencias de un único día. Ejecutar la siguiente sentencia por consola de comandos: " python3 checkAsistencia.py dia dia" , donde el formato del día es "año-mes-dia".
* Reporte de las incidencias de una semana completa. Ejecutar la siguiente sentencia por consola de comandos: " python3 checkAsistencia.py semana diaInicio" , donde el formato del día es "año-mes-dia" e indica el lunes que inicia dicha semana.

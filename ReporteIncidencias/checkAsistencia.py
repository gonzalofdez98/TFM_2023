import mysql.connector
import datetime
from datetime import datetime
from datetime import timedelta
import json
import sys

#Conexión con la base de datos
mydb = mysql.connector.connect(
  host="localhost",
  user="root",
  password="",
)

#Funcion para obtener el cuatrimestre dado un dia y un mes
def checkCuatri(mes, dia):
    if(mes == 9 or mes == 10 or mes == 11 or mes == 12):
        cuatri = 1


    if(mes == 1 and dia < 25):
        cuatri = 1


    if((mes == 1 and dia >=25) or (mes > 1 and mes < 9)):
        cuatri = 2


    return cuatri

#Funcion que genera un fichero de salida con el registro de las incidencias surgidas en un dia
def asistenciaProfesores(fecha, semana):
    print("Funcion dia")
    month = fecha.month
    year = fecha.year
    day = fecha.day
    dia = fecha.weekday()
    cuatri = checkCuatri(month, day)
    
    #Creacion del documento JSON
    reg_Incidencias = {}
    incidenciasList = []
    fecha_str = str(fecha)
    fecha_incidencia = fecha_str.split(" ")[0]
    reg_Incidencias["fecha"] = str(fecha_incidencia)
    

    mycursor = mydb.cursor()
    parameters = (cuatri, dia)
    #Query para buscar todas las horas docentes que hay en el dia seleccionado
    query = "SELECT aula,hora_inicio,profe_imparte,idAsignatura,idGrupo FROM fdi_ucm.info_docente WHERE cuatrimestre = %s AND dia = %s"
    mycursor.execute(query, parameters)
    myresult = mycursor.fetchall()
    print(myresult)
    #Se recorren todas las clases que se imparten durante el dia
    for lesson in myresult:
        print(lesson)
        parameters = (lesson[0], lesson[2], fecha)
        #Query a la tabla de asistencia para comprobar que el profesor ha asistido en el horario
        query = "SELECT * FROM fdi_ucm.asistencia WHERE aula = %s AND idUsuario = %s AND fecha = %s"
        mycursor.execute(query, parameters)
        myresult = mycursor.fetchall()
        #Si hay algun resultado es posible que haya asistido, pero hay que comprobar la hora
        if len(myresult) > 0:
            time = myresult[0][4]
            aux = str(time)
            hour = int(aux.split(":")[0]) #Substring con las horas
            minutes = int(aux.split(":")[1]) #Substring con los minutos
            incidencia = {}
            if str(hour) == lesson[1]:
                print("Asistencia correcta")
                #Se comprueba si el profesor ha llegado tarde
                
                if minutes > 5 and minutes < 45:
                    print("Con retraso")
                    parameters = []
                    parameters.append(lesson[2])
                    query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s"
                    mycursor.execute(query, parameters)
                    myresult = mycursor.fetchall()
                    rellenarIncidencia(incidencia, lesson[2], myresult[0][0], lesson[0], lesson[3], lesson[4], "Retraso de " + str(minutes) + " minutos")
                    
            else:
                if minutes > 45:
                    hour = hour + 1
                if str(hour) == lesson[1]:
                    print("Asistencia correcta")

                #Si se comprueba que no asistió a esa hora, se introduce en el json de incidencias                
                else:
                    print("Asistencia incorrecta")
                    incidencia = profesorNoAsiste(incidencia, lesson, fecha)
            
            if incidencia:
                incidenciasList.append(incidencia)
                    
   
        #Si no hay ningun resultado, se introduce en el json de incidencias
        else:
            print("Asistencia incorrecta")
            incidencia = {}
            incidencia = profesorNoAsiste(incidencia, lesson, fecha)
            incidenciasList.append(incidencia)
            
    reg_Incidencias["incidencias"] = incidenciasList
    json_data = json.dumps(reg_Incidencias)
    
    if semana == 0:
        fileName = fecha_incidencia + "_registroIncidencias.json"
        with open(fileName, 'w') as outfile:
            outfile.write(json_data)
    return incidenciasList
    
    
    
#Funcion que genera un fichero de salida con el registro de las incidencias surgidas en una semana
def asistenciaProfesoresSemana(fechaInicial):
    reg_total = []
    dateStr = str(fechaInicial)
    dateStr = dateStr.split(" ")[0]
    fileName = "Semana" + dateStr + "_registroIncidencias.json"
    
    for i in range(5):
        incidencias = []
        reg_Incidencias = {}
        fecha_str = str(fechaInicial)
        fecha_str = fecha_str.split(" ")[0]
        reg_Incidencias["fecha"] = fecha_str
        incidencias = asistenciaProfesores(fechaInicial, 1)
        reg_Incidencias["incidencias"] = incidencias
        reg_total.append(reg_Incidencias)
        fechaInicial += timedelta(days=1)

    json_data = json.dumps(reg_total)    
    
    print(fileName)
    with open(fileName, 'w') as outfile:
        outfile.write(json_data)

#Funcion auxiliar para introducir datos en la estructura json
def rellenarIncidencia(incidencia, idP, nombre, aula, idA, idG, motivo):
    incidencia["idProfesor"] = idP
    incidencia["nombreProfesor"] = nombre
    incidencia["aula"] = aula
    incidencia["idAsignatura"] = idA
    incidencia["idGrupo"] = idG
    incidencia["motivo"] = motivo
    
#Funcion auxiliar que cubre la funcionalidad en el caso que un profesor no haya asistido a su clase
def profesorNoAsiste(incidencia, lesson, fecha):
    mycursor = mydb.cursor()
    parameters = []
    parameters.append(lesson[2])
    query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s"
    mycursor.execute(query, parameters)
    myresult = mycursor.fetchall()
    rellenarIncidencia(incidencia, lesson[2], myresult[0][0], lesson[0], lesson[3], lesson[4], "No asiste")
    #Ahora se comprueba si ha ido un profesor suplente
    parameters = (lesson[0], fecha)
    print("parameters " + str(lesson[0]))
    query = "SELECT idUsuario, hora FROM fdi_ucm.asistencia WHERE aula = %s AND fecha = %s"
    mycursor.execute(query, parameters)
    myresult = mycursor.fetchall()
    print(myresult)
    if len(myresult) > 0:
        horaSustituto = str(myresult[0][1])
        horaSustituto = horaSustituto.split(":")[0]
        print("Horasustituto = ")
        print(horaSustituto)
        if lesson[1] ==  horaSustituto:
            parameters = (myresult[0][0],"0")
            query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s AND cargo = %s"
            mycursor.execute(query, parameters)
            myresult = mycursor.fetchall()
            incidencia["sustituto"] = myresult[0][0]
    
    return incidencia

def main():   
    if len(sys.argv) == 3:
        if sys.argv[1] == "dia":
            print("Mostrando incidencias del dia: ")
            print(sys.argv[2])
            date = datetime.strptime(sys.argv[2], '%Y-%m-%d')
            print(date)
            asistenciaProfesores(date, 0)
            
        elif sys.argv[1] == "semana":
            print("Mostrando incidencias de la semana del lunes: ")
            print(sys.argv[2])
            date = datetime.strptime(sys.argv[2], '%Y-%m-%d')
            asistenciaProfesoresSemana(date)

    else:
        print("El formato de los argumentos debe ser el siguiente: ")
        print("python3 checkAsistencia.py dia/semana dia/diaInicio")
    
    #asistenciaProfesores()
    
    

if __name__ == "__main__":
    main()


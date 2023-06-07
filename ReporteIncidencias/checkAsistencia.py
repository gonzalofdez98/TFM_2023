import mysql.connector
import datetime
from datetime import datetime
from datetime import timedelta
import json
import sys




mydb = mysql.connector.connect(
  host="localhost",
  user="root",
  password="Elgoncho8!FM",
  #database="mydatabase"
)


def checkCuatri(mes, dia):
    if(mes == 9 or mes == 10 or mes == 11 or mes == 12):
        cuatri = 1


    if(mes == 1 and dia < 25):
        cuatri = 1


    if((mes == 1 and dia >=25) or (mes > 1 and mes < 9)):
        cuatri = 2


    return cuatri


def asistenciaProfesores(fecha, semana):
    #fecha = datetime.datetime(2023, 5, 5)
    month = fecha.month
    year = fecha.year
    day = fecha.day
    cuatri = checkCuatri(month, day)
    
    #Creacion del documento JSON
    reg_Incidencias = {}
    incidenciasList = []
    reg_Incidencias["fecha"] = str(fecha)
    

    mycursor = mydb.cursor()
    parameters = (cuatri, day)
    #Query para buscar todas las horas docentes que hay en el dia seleccionado
    query = "SELECT aula,hora_inicio,profe_imparte,idAsignatura,idGrupo FROM fdi_ucm.info_docente WHERE cuatrimestre = %s AND dia = %s"
    mycursor.execute(query, parameters)
    myresult = mycursor.fetchall()
    #Se recorren todas las horas
    for x in myresult:
        print(x)
        parameters = (x[0], x[2], fecha)
        #Query a la tabla de asistencia para comprobar que el profesor ha asistido en el horario
        query = "SELECT * FROM fdi_ucm.asistencia WHERE aula = %s AND idUsuario = %s AND fecha = %s"
        mycursor.execute(query, parameters)
        myresult = mycursor.fetchall()
        #Si hay algun resultado es posible que haya asistido, pero hay que comprobar la hora
        if len(myresult) > 0:
            time = myresult[0][4]
            aux = str(time)
            hour = int(aux[0:2]) #Substring con las horas
            minutes = int(aux[3:5]) #Substring con los minutos
            if hour == x[1]:
                print("Asistencia correcta")
                #Se comprueba si el profesor ha llegado tarde
                if minutes > 5 and minutes < 45:
                    parameters = (x[2])
                    query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s"
                    mycursor.execute(query, parameters)
                    myresult = mycursor.fetchall()
                    incidencia = {}
                    incidencia["idProfesor"] = x[2]
                    incidencia["nombreProfesor"] = myresult[0][0]
                    incidencia["aula"] = x[0]
                    incidencia["idAsignatura"] = x[3]
                    incidencia["idGrupo"] = x[4]
                    incidencia["motivo"] = "Retraso de " + str(minutes) + " minutos"
                    
            else:
                if minutes > 45:
                    hour = hour + 1
                if str(hour) == x[1]:
                    print("Asistencia correcta")
                
                #Si se comprueba que no asistió a esa hora, se introduce en el json de incidencias                
                else:
                    print("Asistencia incorrecta")
                    parameters = (x[2])
                    query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s"
                    mycursor.execute(query, parameters)
                    myresult = mycursor.fetchall()
                    incidencia = {}
                    incidencia["idProfesor"] = x[2]
                    incidencia["nombreProfesor"] = myresult[0][0]
                    incidencia["aula"] = x[0]
                    incidencia["idAsignatura"] = x[3]
                    incidencia["idGrupo"] = x[4]
                    incidencia["motivo"] = "No asiste"
                    #Ahora se comprueba si ha ido un profesor suplente
                    parameters = (x[0], fecha, 1)
                    query = "SELECT idUsuario FROM fdi_ucm.asistencia WHERE aula = %s AND fecha = %s AND codigo_incidencia = %s"
                    mycursor.execute(query, parameters)
                    myresult = mycursor.fetchall()
                    if len(myresult) > 0:
                        parameters = myresult[0][0]
                        query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s"
                        mycursor.execute(query, parameters)
                        myresult = mycursor.fetchall()
                        incidencia["sustituto"] = myresult[0][0]
    
                    incidenciasList.append(incidencia)
                    
   
        #Si no hay ningun resultado, se introduce en el json de incidencias
        else:
            print("Asistencia incorrecta")
            parameters = []
            parameters.append(x[2])
            query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s"
            mycursor.execute(query, parameters)
            myresult = mycursor.fetchall()
            incidencia = {}
            incidencia["idProfesor"] = x[2]
            incidencia["nombreProfesor"] = myresult[0][0]
            incidencia["aula"] = x[0]
            incidencia["idAsignatura"] = x[3]
            incidencia["idGrupo"] = x[4]
            incidenciasList.append(incidencia)
            #Ahora se comprueba si ha ido un profesor suplente
            parameters = (x[0], fecha, 1)
            query = "SELECT idUsuario FROM fdi_ucm.asistencia WHERE aula = %s AND fecha = %s AND codigo_incidencia = %s"
            mycursor.execute(query, parameters)
            myresult = mycursor.fetchall()
            if len(myresult) > 0:
                parameters = myresult[0][0]
                query = "SELECT nombre FROM fdi_ucm.usuarios WHERE idUsuario = %s"
                mycursor.execute(query, parameters)
                myresult = mycursor.fetchall()
                incidencia["sustituto"] = myresult[0][0]

            incidenciasList.append(incidencia)
            
    reg_Incidencias["incidencias"] = incidenciasList
    json_data = json.dumps(reg_Incidencias)
    
    if semana == 0:
        dateStr = str(fecha)
        dateStr = dateStr[0:10]
        fileName = dateStr + "_registroIncidencias.json"
        with open(fileName, 'w') as outfile:
            outfile.write(json_data)
    return incidenciasList
    
    
    

def asistenciaProfesoresSemana(lunesInicio):
    reg_total = []
    dateStr = str(lunesInicio)
    dateStr = dateStr[0:10]
    fileName = "Semana" + dateStr + "_registroIncidencias.json"
    
    for i in range(5):
        incidencias = []
        reg_Incidencias = {}
        reg_Incidencias["fecha"] = str(lunesInicio)
        incidencias = asistenciaProfesores(lunesInicio, 1)
        reg_Incidencias["incidencias"] = incidencias
        reg_total.append(reg_Incidencias)
        lunesInicio += timedelta(days=1)

    json_data = json.dumps(reg_total)    
    
    print(fileName)
    with open(fileName, 'w') as outfile:
        outfile.write(json_data)
    

def main():   
    if len(sys.argv) == 3:
        if sys.argv[1] == "dia":
            print("Mostrando incidencias del dia: ")
            print(sys.argv[2])
            date = datetime.strptime(sys.argv[2], '%d/%m/%Y')
            print(date)
            asistenciaProfesores(date, 0)
            
        elif sys.argv[1] == "semana":
            print("Mostrando incidencias de la semana del lunes: ")
            print(sys.argv[2])
            date = datetime.strptime(sys.argv[2], '%d/%m/%Y')
            asistenciaProfesoresSemana(date)

    else:
        print("El formato de los argumentos debe ser el siguiente: ")
        print("python3 checkAsistencia.py dia/semana dia/diaInicio")
    
    #asistenciaProfesores()
    
    

if __name__ == "__main__":
    main()


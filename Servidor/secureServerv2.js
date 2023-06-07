// Require express and body-parser
const express = require("express")
const bodyParser = require("body-parser")
var mysql = require('mysql');
const https = require(`https`);
const fs = require(`fs`);

//Conexion con express
const app = express()
const PORT = 443

https.createServer({
   cert: fs.readFileSync(`server.crt`),
   key: fs.readFileSync(`server.key`)
 },app).listen(PORT, function(){
	console.log('Servidor https corriendo en el puerto 443');
});

//Conexion con el servidor SQL
var con = mysql.createConnection({
  host: "localhost",
  user: "root",
  password: "Elgoncho8!FM"
});



//Funcion que devuelve el cuatrimestre dada una fecha
function checkCuatri(mes, dia){
	var cuatri;
	if(mes == 9 || mes == 10 || mes == 11 || mes == 12){
	cuatri = 1;
	}

	if(mes == 1 && dia < 25){
		cuatri = 1;
	}

	if((mes == 1 && dia >=25) || (mes > 1 && mes < 9)){
		cuatri = 2;
	}
	
	return cuatri;
}
/*TODO: Funcion que checkea el formato de los datos del JSON. 
function checkFormat(){
}
TODO: Devolver codigo para que se vuelva a intentar la inserción en caso correcto de comando,
pero incorrecto de insercion
*/

//Funcion que gestiona los retrasos y adelantos de entrada al aula
function checkHora(hora, min){
	if(min > 50){
		hora++;
	}
	return hora;
}

app.use(bodyParser.json())
app.set("view engine", "ejs");
app.get('/',function(req,res) {
    res.send('Hola, has podido acceder al servidor');
	console.log('Se recibio una petición get a través de https');
});


app.post("/GetIdSala", (req, res) => {
	var body = req.body;
	var mac = body["mac"]
	con.connect(function(err) {
		var sqlParams = [mac];
		var sql = "SELECT sala FROM fdi_ucm.lectoressala WHERE mac = ?";
		con.query(sql, sqlParams, function (err, result) {
			if(result[0] != null){
				console.log("Se ha obtenido correctamente el id");
				var idSala = parseInt(result[0].sala);
				var str = idSala.toString();
				res.status(200).send(str);
				res.end();
			}
			else{
				console.log("Dicha MAC no aparece en la tabla");
				res.status(400).send("400");
				res.end();
			}
		});
	});
	
		
});

app.post("/GestionAula", (req, res) => {
	
	// Se obtiene los datos recibidos mediante http
	console.log(req.headers);
	console.log(req.body);
	var body = req.body;
	var aula,idUsuario, cargo;	
	aula = body["sala"];
	idUsuario = body["idtarjeta"];
	cargo = body["cargo"];
	
	//Se obtiene la hora en la que se recibe la peticion y datos derivados
	var hora, cuatri, date, seconds, minutes, hour, month, dayWeek, dayMonth, timestamp;
	date = new Date();
	seconds = date.getSeconds();
	minutes = date.getMinutes();
	hour = date.getHours();
	month = date.getMonth();
	dayWeek = date.getDay();
	dayMonth = date.getDate();
	timestamp = Math.floor(date / 1000);
	cuatri = checkCuatri(month, dayMonth);
	console.log(cuatri);
	hour = checkHora(hour, minutes);
	
	
  
	con.connect(function(err) {

		var asignatura, grupo, hora_inicio;
		var sqlParams = [cuatri, dayWeek, hour, hour - 1, 2, aula];
		//Query que comprueba que haya horario docente dados los parametros registrados
		var sql = "SELECT idAsignatura, idGrupo, hora_inicio FROM fdi_ucm.info_docente WHERE cuatrimestre = ? AND dia = ? AND (hora_inicio = ?  OR (hora_inicio = ? AND duracion = ?)) AND aula = ?";
		con.query(sql, sqlParams, function (err, result) {
			if (err) throw err;
			if(result[0] != null){
				//Hay horario docente en dichos aulas e instante de tiempo
				asignatura = result[0].idAsignatura;
				grupo = result[0].idGrupo;
				hora_inicio = result[0].hora_inicio;
				//Lógica para los alumnos
				if(cargo == 1){ 
					var sqlParams2 = [asignatura, grupo, idUsuario];
					var sql = "SELECT * FROM fdi_ucm.matriculados WHERE idAsignatura = ? AND idGrupo = ? AND idAlumno = ?";
					con.query(sql, sqlParams2, function (err, result) {
						if (err) throw err;
						//Si el alumno está matriculado en el grupo de la asignatura
						if(result[0] != null){
							//Si asiste a primera hora
							if(hour == hora_inicio){
								//Gestión del retraso 
								if(minutes > 10 && minutes < 45){
									console.log("Asistencia correcta con retraso");
									res.status(390).send("390");
									//Inserción de la asistencia
									var timestampTime = date.getTime();
									var time = String(new Date(timestampTime).toTimeString());				
									var sqlParamsInsert = [timestampTime/1000, aula, idUsuario, date, time.substring(0,8)];//tambien idasignatura e idgrupo
									var sql = "INSERT INTO fdi_ucm.asistencia VALUES(?,?,?,?,?)"; 
									con.query(sql, sqlParamsInsert, function (err, result) {
										if (err) throw err;
										console.log(result);
										if(result!= null){
											console.log("Insercion de la asistencia correcta");
										}
										else{
											console.log("Insercion de la asistencia incorrecta");
										}
									});
									//Inserción de la incidencia con código retraso
									var timestampTime = date.getTime();
									var time = String(new Date(timestampTime).toTimeString());				
									var sqlParamsInsert = [timestampTime/1000, idUsuario, date,time.substring(0,8),"0", aula];
									var sql = "INSERT INTO fdi_ucm.incidencias VALUES(?,?,?,?,?,?)"; 
									con.query(sql, sqlParamsInsert, function (err, result) {
										if (err) throw err;
										console.log(result);
										if(result!= null){
											console.log("Insercion de la incidencia correcta");
										}
										else{
											console.log("Insercion de la incidencia incorrecta");
										}
									});
								
									res.end();
								}
								//Asistencia correcta
								else{
									console.log("Asistencia correcta");
									res.status(290).send("290");
									//Insertar en tabla asistencia
									var timestampTime = date.getTime();
									var time = String(new Date(timestampTime).toTimeString());				
									var sqlParamsInsert = [timestampTime/1000, aula, idUsuario, date, time.substring(0,8)];//tambien idasignatura e idgrupo
									var sql = "INSERT INTO fdi_ucm.asistencia VALUES(?,?,?,?,?)"; 
									con.query(sql, sqlParamsInsert, function (err, result) {
									if (err) throw err;
									console.log(result);
									if(result!= null){
										console.log("Insercion de la asistencia correcta");
									}
									else{
										console.log("Insercion de la asistencia incorrecta");
									}
									});
									
									res.end();
								}
							}
							//Asistencia a la segunda hora en una clase de 2 horas
							else{
								console.log("Asiste a segunda hora");
								res.status(392).send("392");
								//Inserción de la asistencia
								var timestampTime = date.getTime();
								var time = String(new Date(timestampTime).toTimeString());				
								var sqlParamsInsert = [timestampTime/1000, aula, idUsuario, date, time.substring(0,8)];//tambien idasignatura e idgrupo
								var sql = "INSERT INTO fdi_ucm.asistencia VALUES(?,?,?,?,?)"; 
								con.query(sql, sqlParamsInsert, function (err, result) {
									if (err) throw err;
									console.log(result);
									if(result!= null){
										console.log("Insercion de la asistencia correcta");
									}
									else{
										console.log("Insercion de la asistencia incorrecta");
									}
								});
								//Inserción de la incidencia con codigo el alumno asiste a segunda hora 
								var timestampTime = date.getTime();
								var time = String(new Date(timestampTime).toTimeString());				
								var sqlParamsInsert = [timestampTime/1000, idUsuario, date,time.substring(0,8),"3", aula];
								var sql = "INSERT INTO fdi_ucm.incidencias VALUES(?,?,?,?,?,?)"; 
								con.query(sql, sqlParamsInsert, function (err, result) {
									if (err) throw err;
									console.log(result);
									if(result!= null){
										console.log("Insercion de la incidencia correcta");
									}
									else{
										console.log("Insercion de la incidencia incorrecta");
									}
								});
							
								res.end();
							}
							
							
						}
						//Alumno no matriculado en el grupo de la asignatura
						else{
							console.log("El alumno no está matriculado en esa asignatura");
							res.status(490).send("Alumno no matriculado en la asignatura");
							//Insertar en tabla incidencias con codigo "no matriculado en la asignatura"
							var timestamp = date.getTime();
							var time = String(new Date(timestamp).toTimeString());							
							var sqlParamsInsert = [timestamp/1000, idUsuario, date,time.substring(0,8),"4", aula ];
							var sql = "INSERT INTO fdi_ucm.incidencias VALUES(?,?,?,?,?,?)"; 
							con.query(sql, sqlParamsInsert, function (err, result) {
							if (err) throw err;
							console.log(result);
							if(result!= null){
								console.log("Insercion de la incidencia correcta");
							}
							else{
								console.log("Insercion de la incidencia incorrecta");
							}
							});
							res.end();
						}
					});
				}
				//Lógica para los profesores
				else{
					
					//No se comprueban los retrasos, se hace en el script checkAsistencia
					if(hour == hora_inicio){
						var sqlParams = [cuatri, dayWeek, hour, aula, idUsuario];
						var sql = "SELECT * FROM fdi_ucm.info_docente WHERE cuatrimestre = ? AND dia = ? AND hora_inicio = ? AND aula = ? AND profe_imparte = ?";
						con.query(sql, sqlParams, function (err, result) {
							if (err) throw err;
							//Si el profesor imparte la asignatura 
							if(result[0] != null){
								console.log("Asistencia correcta");
								res.status(291).send("291");
								var timestampTime = date.getTime();
								var time = String(new Date(timestampTime).toTimeString());
								//Insercion de la asistencia							
								var sqlParamsInsert = [timestampTime/1000, aula, idUsuario, date, time.substring(0,8)];//tambien idasignatura e idgrupo
								var sql = "INSERT INTO fdi_ucm.asistencia VALUES(?,?,?,?,?)"; 
								con.query(sql, sqlParamsInsert, function (err, result) {
									if (err) throw err;
									console.log(result);
									if(result!= null){
										console.log("Insercion de la asistencia correcta");
									}
									else{
										console.log("Insercion de la asistencia incorrecta");
									}
								});
								res.end();
							}
							//Si el profesor no imparte la asignatura 
							else{
								console.log("El profesor no imparte la asignatura");
								res.status(393).send("393");
								//Inserción de la incidencia con código profesor no imparte la asignatura. (Atención, podría ser un profesor sustituto)
								var timestamp = date.getTime();
								var time = String(new Date(timestamp).toTimeString());							
								var sqlParamsInsert = [timestamp/1000, idUsuario, date,time.substring(0,8),"1", aula ];
								var sql = "INSERT INTO fdi_ucm.incidencias VALUES(?,?,?,?,?,?)"; 
								con.query(sql, sqlParamsInsert, function (err, result) {
									if (err) throw err;
									console.log(result);
									if(result!= null){
										console.log("Insercion de la incidencia correcta");
									}
									else{
										console.log("Insercion de la incidencia incorrecta");
									}
								});
								res.end();
							}
						});
					}
					
				}
			}
			else{
				
				console.log("No hay ningun horario disponible para esos parametros");
				res.status(491).send("491");
				res.end();
			}
		});

  });
  
  //res.end();
});





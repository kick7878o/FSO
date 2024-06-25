#===========================================================================
# 		      		1. Automatización con CRONTAB
#
# Comando: ./script dia hora script parametros
# 
# Configuracion de una copia de seguridad automática de forma periódica
#  usando "crontab".
#
# "dia": dia para ir haciendo la copia de seguridad
#
# "hora": hora para ir haciendo la copia de seguridad
#
# "script": script que ejecutar
#
# "parametros": parametros necesarios para hacer funcionar al script
#
# El script ha de ser programado para que ejecute el comando especificado
#  todos los días a la hora especificada.
#
# NOTA: "shift" se usará para desplazar los argumentos pasados hacia la 
#	izquierda
#
#===========================================================================
#!/bin/bash

# Funcion que muestra un mensaje de error si se usa mal el comando
function mssgError () {
	echo "<<< Comando incorrecto >>>"
	echo "$0 dia hora script parametros"
	exit 1 
}

# Comprobamos si hay suficientes argumentos de entrada
if [  $# -lt 4 ]; then
	mssgError
fi

dia=$1 # Obtenemos el dia
hora=$2 # Obtenemos la hora

# Comprobamos que se introduzca un numero
if ! [ $dia =~ ^[0-9]+$ ]; then
	echo "<< Error: 'dia' debe ser un numero. >>"
	exit 1
fi

# Comprobamos que sea un día de la semana
if [ $dia -gt 6 ] || [ $dia -lt 0 ]; then
	echo "<< Error: 'dia' esta fuera del rango esperado: [0-6]. >>"
	echo " < 0-Domingo; 1-Lunes; 2-Martes; 3-Miercoles; 4-Jueves; 5-Viernes; 6-Sabado >"
	exit 1
fi
shift

# Comprobamos que se introduzca un numero
if ! [ $hora =~ ^[0-9]+$ ]; then
	echo "<< Error: 'hora' debe ser un numero. >>"
	exit 1
fi

# Comprobamos que la hora sea la correcta
if [ $hora -gt 23 ] || [ $hora -lt 0 ]; then
	echo "<< Error: 'hora' not inside the expected range: [0-23]. >>"
	exit 1
fi
shift

nScript=$1 # Obtenemos el nombre del script
shift

argScript=$@ # Obtenemos los argumentos de los script

comandoCrontab="* ${hora} * * ${dia} ${nScript} ${argScript}"

echo "-- Comando preparado para ejecutar --"
echo "$comandoCrontab" | crontab -

# Miramos si crontab se ejecuta correctamente
if [ $? -eq 0 ]; then
    echo "Crontab configurado correctamente."
else
    echo "-- ABORTANDO -- Hubo un error al configurar el crontab."
fi

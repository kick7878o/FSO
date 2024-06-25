#===========================================================================
# 	1. Recuperación de un directorio o fichero a otro directorio
#
# Comando: ./script [-D] fich_tgz fich1 fich2 ... dir_destino 
#
# Recupera los ficheros y directorios que se le pasan por parámetro des de un archivo
#  comprimido (.tgz o .tar.gz).
#
# "fich_tgz": comprimido a extraer
#
# "fich1 fich2 ...": ficheros para extraer
#
# "dir_destino": ubicación de la extracción de ficheros del comprimido
#
# Opción -D: (por temas de precisión solo se aceptará la D MAYÚSCULA)
#  Va informando al usuario que hace el script sin ejecutar nada
#
#===========================================================================
#!/bin/bash

# Funcion que sirve para mostrar un mensaje de error de comando
function mssgError () {
	echo "Comando introducido erroneo"
  echo "$0 [-D] fich_tgz fich1 fich2 ... dir_destino"
  exit 1
}

# Para saber si -D está activado
DFlag=0

# Comprobamos si -D está activado
for arg in "$1"; do
  case "$arg" in
  	-D) DFlag=1 # Actualizamos bandera
		shift # Desplazamos argumentos para leer el resto
  		echo "Opcion -D activado" ;;

	-d) echo "-- ABORTANDO -- Opción -d incorrecta. Debe ser -D."
		exit 1 ;;
  esac
done

# Comprobamos que se han isertado suficientes argumentos
#  Mínimo: nombreComprimido ficheroAExtraer DirectorioDestino
if [ $# -lt 3 ]; then
  mssgError
fi

# Obtenemos el comprimido a leer
dirComprimido="$1"
shift # Desplazamos argumentos para leer el resto

# Obtenemos los ficheros a recuperar excepto el ultimo
ficheros="${@:1:$#-1}"

# Obtenemos el directorio a guardar
dirGuardar="${@: -1}"
 
# Miramos si existe el comprimido
if [ ! -e $dirComprimido ]; then
	echo "El comprimido a leer no existe: << $dirComprimido >>"
	exit 1
fi

dirTmp=$(mktemp -d) # Creamos directorio temporal
tar -xzf $dirComprimido -C $dirTmp # Descomprimimos en la temporal

# Iteramos sobre los archivos a recuperar
for arg in ${ficheros[@]}; do
	baseName=$(basename "$arg") # obtenemos el nombre base del archivo

	if [ -e "$dirTmp/$basename" ]; then	# Verificamos si el archivo existe en el temporal
		if [ $DFlag -eq 1 ]; then
			echo "Recuperamos << $baseName >> a << $dirGuardar >>"
		else
			cp -r "$dirTmp/$baseName" "$dirGuardar" # Copiamos al directorio destino
		fi
	else # Si no existe en el temporal, buscamos el mismo fichero con fecha
		fileMod=$(echo "$baseName" | grep -oE '[0-9]{8}$')

		if [ -n $fileMod ]; then # Miramos si se encontro un fichero con fecha
			# Reemplazamos el fichero eliminadno la fecha
			baseName=$(echo "$baseName" | sed "s/.$fileMod//")

			if [ $DFlag -eq 1 ]; then
				echo "Recuperamos << $baseName.$fileMod >> a << $dirGuardar >>"

			else # Copiamos del temporal al directorio destino
				cp -r "$dirTmp/$baseName.$fileMod" "$dirGuardar"
			fi
		fi
	fi
done

# Eliminamos el directorio temporal
echo "Eliminamos el directorio temporal << $dirTmp >>"
rm -rf "$dirTmp"

echo "Recuperacion completada"
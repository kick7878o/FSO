#===========================================================================
# 						1. COPIA DE SEGURIDAD
#
# Comando: ./script [-k] [-D] fich1 fich2 ... fich.tgz(o tar.gz) 
#
#	"fich1 fich2 ,,,": ficheros para realizar la copia de seguridad.
#
#	"fich.tgz(o tar.gz)": nombre del comprimido
#
# Claramente ha de existir los ficheros o directorios a copiar
# Las carpetas que se hayan introducido se ha de copiar TODO el
#   contenido que tenga dentro correctamente sin modificar.
#
# Opción -k: (por temas de precisión solo se aceptará la k minúscula)
#  Si al añadir un fichero al comprimido ya existe, conservar las dos
#   y, la mas reciente, ponerle la fecha AAAAMMDD al final del nombre
#  Ej: ak.c(existente) == ak.c(añadido) --> ak.c y ak20240309.c
#
# Opción -D: (por temas de precisión solo se aceptará la D MAYÚSCULA)
#  Va informando al usuario que hace el script (comprobando, añadiendo...)
#    sin ejecutar nada
#
# Nota Adicional: ha de permitir tanto rutas absolutas como relativas
#  en la entrada y/o salida
#
#===========================================================================
#!/bin/bash

# Funcion que muestra un mensaje de error si no se
#  llama al script correctamente
function mssgError () {
   echo " <<< Comando Erroneo. ABORTAMOS >>>"
   echo "$0 [-k] [-D] fich1 fich2 ... nombreComprimido"
   exit 1
}

# Definimos banderas para el -k y -D para saber si se han introducido
kFlag=0
DFlag=0 # Ambos desactivados inicialmente

# Iteramos sobre los primeros dos argumentos
#  para comprobar si hemos activado alguna opción
for arg in "$1" "$2"; do
	case "$arg" in
  		-k) kFlag=1 # Actualizamos la bandera
	  		shift # Desplazamos argumentos para leer el resto
	    	echo "Opcion -k activado" ;;

		-D) DFlag=1 # Actualizamos bandera
	    	shift # Desplazamos argumentos para leer el resto
	    	echo "Opcion -D activado" ;;

		-K) echo "-- ABORTANDO -- Opcion -K incorrecta. Debe ser -k" 
			exit 1 ;;

		-d) echo "-- ABORTANDO -- Opción -d incorrecta. Debe ser -D"
			exit 1 ;;
  	esac
done

# Mostramos si no se han activado ningun flag
#if [ $kFlag -eq 0 ] && [ $DFlag -eq 0 ]; then   
#	echo "Ninguna opcion activada (-k y -D)."
#fi

# Comprobamos los argumentos después de mirar las opciones
if [ $# -lt 2 ]; then
	mssgError
fi

# Obtenemos el nombre del comprimido (ultimo argumento)
dirDest="${@: -1}"

# Obtenemos el resto de argumentos excepto el ultimo y -k/-D
ficheros="${@:1:$#-1}"

# Bucle que recorre los ficheros a hacer copia y comprueba si existen
for arg in ${ficheros[@]}; do
	# Miramos si no existe y, si es el caso, mostramos mensaje de error y abortamos
	if [ ! -e $arg ]; then
		echo "-- Arch/Dir no existe: '$arg' ABORTAMOS--"
		exit 1
	fi

	# Indicamos que existe el archivo o directorio insertado
	echo "-- Arch/Dir existe: '$arg' --"
done

if [ -e "$dirDest" ]; then # Verificar si el comprimido ya existe

	if [ $kFlag -eq 1 ]; then # Verificamos si -k está activado

		# Creamos directorio temporal
		dirTmp=$(mktemp -d)

		# Descomprimimos el comprimido existente en el dir temporal
		tar -xzf $dirDest -C $dirTmp

		for arg in ${ficheros[@]}; do # Bucle que itera sobre cada argumento de los ficheros
			baseName=$(basename "$arg") # Obtenemos el nombre del fichero
			fichDest="$dirTmp/$baseName" # Creamos ruta con el nombre del fichero

			# Verificamos si el archivo ya existe
			if [ -e $fichDest ]; then
				# Obtenemos fecha de modificación del archivo
				dateMod=$(stat -c %y "$fichDest" | cut -d ' ' -f1)
				# Elimina los guiones. P.Ej: 2024-05-23 -> 20240523
				dateMod=$(echo "$dateMod" | tr -d '-')

				# Cambiar el nombre del archivo nuevo agregando la fecha
				nuevoFich="${baseName}.${dateMod}"
				cp -r "$arg" "$dirTmp/$nuevoFich" # Copia recursiva del original al original+fecha

			else # Si el archivo no existe, simplemente lo copiamos
				cp -r "$arg" "$dirTmp"
			fi
		done

		# Volvemos a comprimir los archivos
		if [ $DFlag -eq 1 ]; then
			echo "Añadiendo nuevo ficheros al comprimido << $dirDest >>"
			find "$dirTmp" -type f
		else # Hacemos comprimido del temporal al destino con todo el contenido
			tar -czf $dirDest -C $dirTmp .
		fi
		
		rm -rf "$dirTmp" # Eliminamos el directorio temporal

	else # Si el comprimido ya existe y -k desactivado
		echo "El comprimido << $dirDest >> ya existe y no se ha activado -k"
		exit 1
	fi

else # No existe el comprimido a crear
	if [ $DFlag -eq 1 ]; then # Si DryRun está activado
		echo "Creando el comprimido << $dirDest >> con los siguientes ficheros:"

		# Recorrido por todos los argumentos de ficheros 
		for arg in ${ficheros[@]}; do
			echo "$arg"
		done

	else # Simplemente creamos el nuevo comprimido
		tar -zcf $dirDest $ficheros
	fi
fi

echo "Copia de seguridad completada"
#!/bin/sh

cd /home/utnso/so-commons-library/

sudo make install

cd /home/utnso/parser-ansisop/parser/

sudo make all
sudo make install

echo "Commons y Parser instalados"

cd /home/utnso/tp-2016-1c--Dieta/LibreriasSO/

sudo make install

echo "LibreriasSO instalada."

cd /home/utnso/tp-2016-1c--Dieta/Consola/src/

sudo make all

echo "Proceso Consola compilado."

cd /home/utnso/tp-2016-1c--Dieta/CPU/src/

sudo make all

echo "Proceso CPU compilado."

cd /home/utnso/tp-2016-1c--Dieta/Nucleo/src/

sudo make all

echo "Proceso Nucleo compilado."

cd /home/utnso/tp-2016-1c--Dieta/UMC/src/

sudo make all

echo "Proceso UMC compilado."

cd /home/utnso/tp-2016-1c--Dieta/Swap/src/

sudo make all

echo "Proceso Swap compilado."


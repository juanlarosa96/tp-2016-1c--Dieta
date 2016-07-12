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

cd /home/utnso/tp-2016-1c-SO-2016-1C/Consola/src/

sudo make all

echo "Proceso Consola compilado."

cd /home/utnso/tp-2016-1c-SO-2016-1C/CPU/src/

sudo make all

echo "Proceso CPU compilado."

cd /home/utnso/tp-2016-1c-SO-2016-1C/Nucleo/src/

sudo make all

echo "Proceso Nucleo compilado."

cd /home/utnso/tp-2016-1c-SO-2016-1C/UMC/src/

sudo make all

echo "Proceso UMC compilado."

cd /home/utnso/tp-2016-1c-SO-2016-1C/Swap/src/

sudo make all

echo "Proceso Swap compilado."


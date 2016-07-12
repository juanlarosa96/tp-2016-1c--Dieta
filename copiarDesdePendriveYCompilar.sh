#!/bin/sh

#Copio las bibliotecas 
sudo cp -r ansisop-parser/ /home/utnso
sudo cp -r so-commons-library/ /home/utnso
sudo cp -r tp-2016-1c--Dieta/ /home/utnso

cd /home/utnso/tp-2016-1c--Dieta/scripts/

#Pongo en modo ejecutable los scripts
sudo chmod +x *.ansisop

cd /home/utnso/tp-2016-1c--Dieta/
sudo sh compilar.sh



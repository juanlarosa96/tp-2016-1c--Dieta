/*
 * funciones.c
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#include "funciones.h"

void manejarCPU(int socketCpu) {

	/*
	 * chequeo que el CPU no haya hecho un hot plug
	 * t_pcb unPcb = obtenerPcbEnEstadoDeListo();
	 * Se ejecuta un quantum
	 * Se pone el pcb en estado de ejecucion
	 * Se envia pcb al cpu
	 * termina cuantum/interrupcion
	 * Se pone pcb en cola de listo/bloqueado segun corresponda
	 * repetir
	 */

	int desconectado = 0, cambioProceso;

	while (!desconectado) {

		cambioProceso = 0;

		t_pcbConConsola siguientePcb = DevolverProcesoColaListos();
		if (siguientePcb.socketConsola != -1) {
			enviarPcb(socketCpu, siguientePcb.pcb);

			int respuesta;

			while (!cambioProceso) {
				if (recibirRespuestaCPU(socketCpu, &respuesta)) {
					//Se desconecto el CPU
					//finalizarProceso(siguientePcb);
					desconectado = 1;
					return;
				}
				switch (respuesta) {

				case 99: //Fin programa
					//finalizarProceso(siguientePcb);
					break;

				case 100: //Fin quantum
					AgregarAProcesoColaListos(siguientePcb);
					cambioProceso = 1;
					break;

				}
			}
		}
	}
	pthread_exit(NULL);
}

void AgregarACola(t_pcbConConsola elemento, t_queue * cola) {
	queue_push(cola, &elemento);
	return;
}

t_pcbConConsola sacarPrimeroCola(t_queue * cola) {
	t_pcbConConsola elemento;
	void * elementoPop = queue_pop(cola);
	if (elementoPop == NULL) {
		elemento.socketConsola = -1;
		return elemento;
	}
	memcpy(&elemento, elementoPop, sizeof(t_pcbConConsola));
	return elemento;
}

t_pcbConConsola DevolverProcesoColaListos() {
	return (sacarPrimeroCola(cola_PCBListos));
}

t_pcbConConsola DevolverProcesoColaNuevos() {
	return (sacarPrimeroCola(cola_PCBNuevos));
}

t_pcbConConsola DevolverProcesoColaFinalizados() {
	return (sacarPrimeroCola(cola_PCBFinalizados));
}

t_pcbConConsola DevolverProcesoColaLBloqueados() {
	return (sacarPrimeroCola(cola_PCBListos));
}

void AgregarAProcesoColaListos(t_pcbConConsola elemento) {
	AgregarACola(elemento, cola_PCBListos);
}

void AgregarAProcesoColaNuevos(t_pcbConConsola elemento) {
	AgregarACola(elemento, cola_PCBNuevos);
}

void AgregarAProcesoColaFinalizados(t_pcbConConsola elemento) {
	AgregarACola(elemento, cola_PCBFinalizados);
}

void AgregarAProcesoColaBloqueados(t_pcbConConsola elemento) {
	AgregarACola(elemento, cola_PCBBloqueados);
}

t_pcb crearPcb(char * programa, int largoPrograma) {
	t_pcb nuevoPcb;
	t_metadata_program * metadata;
	nuevoPcb.pid = pidPcb;
	pidPcb++;
	nuevoPcb.pc = 0;
	metadata = metadata_desde_literal(programa);

	nuevoPcb.indice_etiquetas.etiquetas = metadata->etiquetas;
	nuevoPcb.indice_etiquetas.largoTotalEtiquetas = metadata->etiquetas_size;

	nuevoPcb.indice_codigo.instrucciones = metadata->instrucciones_serializado;
	nuevoPcb.indice_codigo.cantidadInstrucciones = metadata->instrucciones_size;
	nuevoPcb.indice_codigo.numeroInstruccionInicio = metadata->instruccion_inicio;

	t_list * pilaInicial;
	pilaInicial = list_create();
	nuevoPcb.indice_stack = pilaInicial;
	nuevoPcb.paginas_codigo = calcularPaginasCodigo(largoPrograma);

	free(metadata);

	return nuevoPcb;
}

int calcularPaginasCodigo(int largoPrograma) {
	int paginas = 0;
	paginas = largoPrograma / tamanioPagina;
	if (largoPrograma % tamanioPagina) {
		paginas++;
	}
	return paginas;

}

int iniciarUnPrograma(int clienteUMC, t_pcb nuevoPcb, int largoPrograma, char * programa) {
	enviarInicializacionPrograma(clienteUMC, nuevoPcb.pid, largoPrograma, programa, nuevoPcb.paginas_codigo);
	return recibirRespuestaInicializacion(clienteUMC);

}

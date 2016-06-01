/*
 * funciones.c
 *
 *  Created on: 15/5/2016
 *      Author: utnso
 */

#include "funciones.h"

void manejarCPU(void * socket) {

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

	int socketCpu;
	memcpy(&socketCpu,socket,sizeof(int));
	pthread_mutex_unlock(&mutexVariableNuevaConexion);

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
					finalizarProceso(siguientePcb);
					desconectado = 1;
					log_info(logger, "Se desconecto el CPU", texto);
					pthread_exit(NULL);
				}
				switch (respuesta) {

				case 99: //Fin programa
					siguientePcb.pcb = recibirPcb(socketCpu);
					finalizarProceso(siguientePcb);
					pthread_mutex_lock(&mutexListaConsolas);
					int largoLista = list_size(listaConsolas), i;
					for (i = 0; i < largoLista; i++) {
						t_pcbConConsola * pcbBusqueda = (t_pcbConConsola *) list_get(listaConsolas, i);
						if (pcbBusqueda->socketConsola == siguientePcb.socketConsola) {
							t_pcbConConsola * pcbFinalizado = (t_pcbConConsola *) list_remove(listaConsolas, i);
							AgregarAProcesoColaFinalizados(*pcbFinalizado);
							free(pcbFinalizado);
						}
					}
					pthread_mutex_unlock(&mutexListaConsolas);
					break;

				case 100: //Fin quantum

					siguientePcb.pcb = recibirPcb(socketCpu);
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
	pthread_mutex_lock(&mutexColaListos);
	return (sacarPrimeroCola(cola_PCBListos));
	pthread_mutex_unlock(&mutexColaListos);
}

t_pcbConConsola DevolverProcesoColaNuevos() {
	return (sacarPrimeroCola(cola_PCBNuevos));
}

t_pcbConConsola DevolverProcesoColaFinalizados() {
	pthread_mutex_lock(&mutexColaFinalizados);
	return (sacarPrimeroCola(cola_PCBFinalizados));
	pthread_mutex_unlock(&mutexColaFinalizados);
}

t_pcbConConsola DevolverProcesoColaLBloqueados() {
	return (sacarPrimeroCola(cola_PCBListos));
}

void AgregarAProcesoColaListos(t_pcbConConsola elemento) {
	pthread_mutex_lock(&mutexColaListos);
	AgregarACola(elemento, cola_PCBListos);
	pthread_mutex_unlock(&mutexColaListos);
}

void AgregarAProcesoColaNuevos(t_pcbConConsola elemento) {
	AgregarACola(elemento, cola_PCBNuevos);
}

void AgregarAProcesoColaFinalizados(t_pcbConConsola elemento) {
	pthread_mutex_lock(&mutexColaFinalizados);
	AgregarACola(elemento, cola_PCBFinalizados);
	pthread_mutex_unlock(&mutexColaFinalizados);
}

void AgregarAProcesoColaBloqueados(t_pcbConConsola elemento) {
	AgregarACola(elemento, cola_PCBBloqueados);
}

t_pcb crearPcb(char * programa, int largoPrograma) {
	t_pcb nuevoPcb;
	t_metadata_program * metadata;
	nuevoPcb.pid = pidPcb;
	pidPcb++;

	metadata = metadata_desde_literal(programa);

	nuevoPcb.pc = metadata->instruccion_inicio;
	nuevoPcb.indice_etiquetas.etiquetas = metadata->etiquetas;
	nuevoPcb.indice_etiquetas.largoTotalEtiquetas = metadata->etiquetas_size;

	nuevoPcb.indice_codigo.instrucciones = metadata->instrucciones_serializado;
	nuevoPcb.indice_codigo.cantidadInstrucciones = metadata->instrucciones_size;
	nuevoPcb.indice_codigo.numeroInstruccionInicio = metadata->instruccion_inicio;

	t_list * pilaInicial;
	pilaInicial = list_create();
	nuevoPcb.indice_stack = pilaInicial;
	nuevoPcb.paginas_codigo = calcularPaginasCodigo(largoPrograma);

	metadata_destruir(metadata);

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

int iniciarUnPrograma(int clienteUMC, t_pcb nuevoPcb, int largoPrograma, char * programa, uint32_t paginasStack) {
	enviarInicializacionPrograma(clienteUMC, nuevoPcb.pid, largoPrograma, programa, nuevoPcb.paginas_codigo + paginasStack);
	return recibirRespuestaInicializacion(clienteUMC);

}

void finalizarProceso(t_pcbConConsola siguientePcb) {
	enviarFinalizacionProgramaUMC(clienteUMC, siguientePcb.pcb.pid);
	enviarFinalizacionProgramaConsola(siguientePcb.socketConsola);

}

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
	memcpy(&socketCpu, socket, sizeof(int));
	free(socket);

	int desconectado = 0, cambioProceso;
	int respuesta;
	t_pcbConConsola siguientePcb;

	while (!desconectado) {

		cambioProceso = 0;

		sem_wait(&semaforoColaListos);

		siguientePcb = DevolverProcesoColaListos();
		pthread_mutex_lock(&mutexUnidadesQuantum);
		enviarUnidadesQuantum(socketCpu, cantidadQuantum);
		pthread_mutex_unlock(&mutexUnidadesQuantum);

		pthread_mutex_lock(&mutexRetardoQuantum);
		enviarSleepQuantum(socketCpu, retardoQuantum);
		pthread_mutex_unlock(&mutexRetardoQuantum);

		enviarPcb(socketCpu, siguientePcb.pcb);

		if (recibirHeader(socketCpu) != CPUListo) {

			cambioProceso = 1;
			desconectado = 1;

			log_info(logger, "CPU socket %d desconectado", socketCpu);
			close(socketCpu);
			pthread_mutex_lock(&mutexListaFinalizacionesPendientes);
			int j, sizeLista = list_size(listaFinalizacionesPendientes), finalizar = 0;
			int * socketEnLista;

			for (j = 0; j < sizeLista; j++) {
				socketEnLista = (int *) list_get(listaFinalizacionesPendientes, j);
				if (siguientePcb.socketConsola == *socketEnLista) {
					finalizar = 1;
					list_remove(listaFinalizacionesPendientes, j);
					cambioProceso = 1;
					j = sizeLista;
					free(socketEnLista);
				}
			}
			pthread_mutex_unlock(&mutexListaFinalizacionesPendientes);
			if (finalizar) {
				finalizarProceso(siguientePcb);
			} else {
				AgregarAProcesoColaListos(siguientePcb);
			}
			pthread_exit(NULL);
		}

		while (!cambioProceso) {
			if (recibirRespuestaCPU(socketCpu, &respuesta)) {
				//Se desconecto el CPU
				finalizarProceso(siguientePcb);
				desconectado = 1;
				log_info(logger, "Se desconectó el CPU socket %d", socketCpu);
				close(socketCpu);
				pthread_exit(NULL);
			}

			switch (respuesta) {

			case finalizacionPrograma: //Fin programa
				if (recibirHeader(socketCpu) == headerPcb) {
					destruirPcb(siguientePcb.pcb);
					siguientePcb.pcb = recibirPcb(socketCpu);
					log_info(logger, "Se finalizo el proceso pid %d", siguientePcb.pcb.pid);
					finalizarProceso(siguientePcb);
					cambioProceso = 1;
				} else {
					log_error(logger, "El CPU no envio un Pcb", texto);
					finalizarProceso(siguientePcb);
					pthread_exit(NULL);
				}
				break;
			case abortarPrograma:
				//Fin programa por abortado
				if (recibirHeader(socketCpu) == headerPcb) {
					destruirPcb(siguientePcb.pcb);
					siguientePcb.pcb = recibirPcb(socketCpu);
					log_info(logger, "Se aborto el proceso pid %d", siguientePcb.pcb.pid);
					abortarProceso(siguientePcb);
					cambioProceso = 1;
				} else {
					log_error(logger, "El CPU no envio un Pcb", texto);
					finalizarProceso(siguientePcb);
					pthread_exit(NULL);
				}
				break;

			case finDeQuantum:
				//Fin quantum

				if (recibirHeader(socketCpu) == headerPcb) {
					destruirPcb(siguientePcb.pcb);
					siguientePcb.pcb = recibirPcb(socketCpu);
					log_info(logger, "Fin de quantum de proceso pid %d", siguientePcb.pcb.pid);
					int j, sizeLista = list_size(listaFinalizacionesPendientes), finalizar = 0;
					int * socketEnLista;

					pthread_mutex_lock(&mutexListaFinalizacionesPendientes);

					for (j = 0; j < sizeLista; j++) {
						socketEnLista = (int *) list_get(listaFinalizacionesPendientes, j);
						if (siguientePcb.socketConsola == *socketEnLista) {
							finalizar = 1;
							list_remove(listaFinalizacionesPendientes, j);
							cambioProceso = 1;
							j = sizeLista;
							free(socketEnLista);
						}
					}
					pthread_mutex_unlock(&mutexListaFinalizacionesPendientes);
					if (finalizar)
						finalizarProceso(siguientePcb);
					if (!cambioProceso) {
						AgregarAProcesoColaListos(siguientePcb);
						cambioProceso = 1;
					}
				} else {
					log_error(logger, "El CPU no envio un Pcb", texto);
					finalizarProceso(siguientePcb);
					pthread_exit(NULL);
				}
				log_info(logger, "Quantum de CPU socket %d terminado", socketCpu);
				break;

			case finalizacionCPU:
				cambioProceso = 1;
				desconectado = 1;
				log_info(logger, "CPU socket %d apagado", socketCpu);
				AgregarAProcesoColaListos(siguientePcb);
				break;

			case primitivaImprimir:
				;
				int largoTexto;
				char *texto;
				uint32_t pid;
				recibirValorAImprimir(socketCpu, &pid, &largoTexto, &texto);
				int listSize = list_size(listaConsolas), i;

				pthread_mutex_lock(&mutexListaConsolas);
				for (i = 0; i < listSize; i++) {
					t_pidConConsola * elemento = (t_pidConConsola *) list_get(listaConsolas, i);
					if (elemento->pid == pid) {
						enviarResultadoDeEjecucionAnsisop(elemento->socketConsola, texto, largoTexto);
						log_info(logger, "Se envió texto a imprimir, del proceso PID: %d a Consola (socket nro %d)", elemento->pid,
								elemento->socketConsola);
						break;
					}
				}
				pthread_mutex_unlock(&mutexListaConsolas);

				break;

			case headerEntradaSalida:
				if (recibirHeader(socketCpu) == headerPcb) {
					destruirPcb(siguientePcb.pcb);
					siguientePcb.pcb = recibirPcb(socketCpu);

					int largo, tiempo;
					char * nombre;
					recibirEntradaSalida(socketCpu, &largo, &nombre, &tiempo);
					ponerEnColaBloqueados(siguientePcb, nombre, largo, tiempo);
					cambioProceso = 1;
				}

				break;

			case headerSignal:
				;
				int largoNombre, contador = 0;
				uint32_t id;
				char * nombre;
				recibirSignal(socketCpu, &id, &largoNombre, &nombre);

				while (vectorSemaforosAnsisop[contador] != NULL) {

					if (!strcmp(vectorSemaforosAnsisop[contador], nombre)) {
						pthread_mutex_lock(vectorMutexSemaforosAnsisop[contador]);
						vectorValoresSemaforosAnsisop[contador] = vectorValoresSemaforosAnsisop[contador] + 1;

						if (queue_size(vectorColasSemaforosAnsisop[contador])) {
							t_pcbConConsola *pcbPop = (t_pcbConConsola *) queue_pop(vectorColasSemaforosAnsisop[contador]);
							AgregarAProcesoColaListos(*pcbPop);
							free(pcbPop);
						}
						pthread_mutex_unlock(vectorMutexSemaforosAnsisop[contador]);

					}
					contador++;
				}
				break;

			case headerWait:
				;
				int largoNombreWait, contadorWait = 0;
				uint32_t idProceso;
				char * nombreWait;
				int bloqueado = 0;
				recibirWait(socketCpu, &idProceso, &largoNombreWait, &nombreWait);

				while (vectorSemaforosAnsisop[contadorWait] != NULL) {
					if (!strcmp(vectorSemaforosAnsisop[contadorWait], nombreWait)) {
						pthread_mutex_lock(vectorMutexSemaforosAnsisop[contadorWait]);
						vectorValoresSemaforosAnsisop[contadorWait] = vectorValoresSemaforosAnsisop[contadorWait] - 1;

						if (vectorValoresSemaforosAnsisop[contadorWait] < 0) {
							enviarRespuestaSemaforo(socketCpu, headerBloquear);
							t_pcbConConsola * pcbABloquear = malloc(sizeof(t_pcbConConsola));
							if (recibirHeader(socketCpu) == headerPcb) {
								pcbABloquear->pcb = recibirPcb(socketCpu);
								pcbABloquear->socketConsola = siguientePcb.socketConsola;
								queue_push(vectorColasSemaforosAnsisop[contadorWait], pcbABloquear);
							} else {
								log_error(logger, "CPU no envio un pcb");
							}
							bloqueado = 1;
							cambioProceso = 1;
						}
						pthread_mutex_unlock(vectorMutexSemaforosAnsisop[contadorWait]);
					}
					contadorWait++;
				}
				if (!bloqueado) {
					enviarRespuestaSemaforo(socketCpu, headerSeguir);
				}
				break;

			case pedidoVariableCompartida:
				;
				char * nombrePedidoCompartida;
				int contadorPedidoCompartida = 0;
				recibirVariableCompartida(socketCpu, &nombrePedidoCompartida);

				while (vectorVariablesCompartidas[contadorPedidoCompartida] != NULL) {

					if (!strcmp(vectorVariablesCompartidas[contadorPedidoCompartida], nombrePedidoCompartida)) {
						pthread_mutex_lock(vectorMutexVariablesCompartidas[contadorPedidoCompartida]);
						enviarValorVariableCompartida(socketCpu, vectorValoresVariablesCompartidas[contadorPedidoCompartida]);
						pthread_mutex_unlock(vectorMutexVariablesCompartidas[contadorPedidoCompartida]);
						log_info(logger, "Se envio valor variable compartida %s al proceso pid %d",
								vectorVariablesCompartidas[contadorPedidoCompartida], siguientePcb.pcb.pid);
					}
					contadorPedidoCompartida++;
				}
				break;

			case asignacionVariableCompartida:
				;
				char * nombreAsignacionCompartida;
				int contadorAsignacionCompartida = 0, valorVariableCompartida;
				recibirVariableCompartidaConValor(socketCpu, &nombreAsignacionCompartida, &valorVariableCompartida);

				while (vectorVariablesCompartidas[contadorAsignacionCompartida] != NULL) {
					if (!strcmp(vectorVariablesCompartidas[contadorAsignacionCompartida], nombreAsignacionCompartida)) {
						pthread_mutex_lock(vectorMutexVariablesCompartidas[contadorAsignacionCompartida]);
						vectorValoresVariablesCompartidas[contadorAsignacionCompartida] = valorVariableCompartida;
						pthread_mutex_unlock(vectorMutexVariablesCompartidas[contadorAsignacionCompartida]);
						log_info(logger, "Se asigno valor %d a la variable compartida %s por pedido del proceso pid %d", valorVariableCompartida,
								vectorVariablesCompartidas[contadorAsignacionCompartida], siguientePcb.pcb.pid);
					}
					contadorAsignacionCompartida++;
				}
				break;
			}
		}
	}
	pthread_exit(NULL);
}

void AgregarACola(t_pcbConConsola elemento, t_queue * cola) {
	void * nuevoElemento = malloc(sizeof(t_pcbConConsola));
	memcpy(nuevoElemento, &elemento, sizeof(t_pcbConConsola));
	queue_push(cola, nuevoElemento);
}

t_pcbConConsola sacarPrimeroCola(t_queue * cola) {
	t_pcbConConsola elemento;
	void * elementoPop = queue_pop(cola);
	if (elementoPop == NULL) {
		elemento.socketConsola = -1;
		return elemento;
	}
	memcpy(&elemento, elementoPop, sizeof(t_pcbConConsola));
	free(elementoPop);
	return elemento;
}

t_pcbBloqueado sacarPrimeroColaBloqueados(t_queue * cola) {
	t_pcbBloqueado elemento;
	void * elementoPop = queue_pop(cola);
	if (elementoPop == NULL) {
		elemento.pcb.socketConsola = -1;
		return elemento;
	}
	memcpy(&elemento, elementoPop, sizeof(t_pcbBloqueado));
	free(elementoPop);
	return elemento;
}

t_pcbConConsola DevolverProcesoColaListos() {
	pthread_mutex_lock(&mutexColaListos);
	t_pcbConConsola pcb = (sacarPrimeroCola(cola_PCBListos));
	pthread_mutex_unlock(&mutexColaListos);
	return pcb;
}

t_pcbConConsola DevolverProcesoColaFinalizados() {
	pthread_mutex_lock(&mutexColaFinalizados);
	t_pcbConConsola pcb = (sacarPrimeroCola(cola_PCBFinalizados));
	pthread_mutex_unlock(&mutexColaFinalizados);
	return pcb;
}

void AgregarAProcesoColaListos(t_pcbConConsola elemento) {
	pthread_mutex_lock(&mutexColaListos);
	AgregarACola(elemento, cola_PCBListos);
	pthread_mutex_unlock(&mutexColaListos);
	log_info(logger, "Se puso en cola listos al proceso pid %d", elemento.pcb.pid);
	sem_post(&semaforoColaListos);
}

void AgregarAProcesoColaNuevos(t_pcbConConsola elemento) {
	AgregarACola(elemento, cola_PCBNuevos);
}

void AgregarAProcesoColaFinalizados(t_pcbConConsola elemento) {
	pthread_mutex_lock(&mutexColaFinalizados);
	AgregarACola(elemento, cola_PCBFinalizados);
	pthread_mutex_unlock(&mutexColaFinalizados);
}

void AgregarAProcesoColaBloqueados(t_queue * cola, t_pcbBloqueado elemento) {
	void * nuevoElemento = malloc(sizeof(t_pcbBloqueado));
	memcpy(nuevoElemento, &elemento, sizeof(t_pcbBloqueado));
	queue_push(cola, nuevoElemento);
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

	nuevoPcb.paginas_codigo = calcularPaginasCodigo(largoPrograma);

	t_list * pilaInicial;
	pilaInicial = list_create();

	t_registro_pila * registroPila = malloc(sizeof(t_registro_pila));
	registroPila->lista_argumentos = list_create();
	registroPila->lista_variables = list_create();
	registroPila->posicionUltimaVariable = nuevoPcb.paginas_codigo * tamanioPagina;

	list_add(pilaInicial, (void *) registroPila);

	nuevoPcb.indice_stack = pilaInicial;

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

int iniciarUnPrograma(int clienteUMC, t_pcb nuevoPcb, int largoPrograma, char * programa, uint32_t paginasStack) {
	enviarInicializacionPrograma(clienteUMC, nuevoPcb.pid, largoPrograma, programa, nuevoPcb.paginas_codigo + paginasStack);
	return recibirRespuestaInicializacion(clienteUMC);

}

void finalizarProceso(t_pcbConConsola siguientePcb) {
	FD_CLR(siguientePcb.socketConsola, &bolsaDeSockets);
	pthread_mutex_lock(&mutexUMC);
	enviarFinalizacionProgramaUMC(clienteUMC, siguientePcb.pcb.pid);
	pthread_mutex_unlock(&mutexUMC);
	enviarFinalizacionProgramaConsola(siguientePcb.socketConsola);
	pthread_mutex_lock(&mutexListaConsolas);
	int largoLista = list_size(listaConsolas), i;
	for (i = 0; i < largoLista; i++) {
		t_pidConConsola * pcbBusqueda = (t_pidConConsola *) list_get(listaConsolas, i);
		if (pcbBusqueda->socketConsola == siguientePcb.socketConsola) {
			t_pidConConsola * pcbFinalizado = (t_pidConConsola *) list_remove(listaConsolas, i);
			free(pcbFinalizado);
			break;
		}
	}
	pthread_mutex_unlock(&mutexListaConsolas);
	log_info(logger, "Se finalizó programa pid %d", siguientePcb.pcb.pid);

	pthread_mutex_lock(&mutexListaFinalizacionesPendientes);
	int cantElementos = list_size(listaFinalizacionesPendientes);
	int * socketBusqueda;
	for (i = 0; i < cantElementos; i++) {

		socketBusqueda = (int *) list_get(listaFinalizacionesPendientes, i);

		if (*socketBusqueda == siguientePcb.socketConsola) {

			free(list_remove(listaFinalizacionesPendientes, i));
		}

	}
	pthread_mutex_unlock(&mutexListaFinalizacionesPendientes);

	destruirPcb(siguientePcb.pcb);

}

void abortarProceso(t_pcbConConsola siguientePcb) {

	FD_CLR(siguientePcb.socketConsola, &bolsaDeSockets);
	enviarFinalizacionProgramaConsola(siguientePcb.socketConsola);

	pthread_mutex_lock(&mutexListaConsolas);
	int largoLista = list_size(listaConsolas), i;
	for (i = 0; i < largoLista; i++) {
		t_pidConConsola * pcbBusqueda = (t_pidConConsola *) list_get(listaConsolas, i);
		if (pcbBusqueda->socketConsola == siguientePcb.socketConsola) {
			t_pidConConsola * pcbFinalizado = (t_pidConConsola *) list_remove(listaConsolas, i);
			free(pcbFinalizado);
		}
	}
	pthread_mutex_unlock(&mutexListaConsolas);
	log_info(logger, "Se abortó programa pid %d", siguientePcb.pcb.pid);

	pthread_mutex_lock(&mutexListaFinalizacionesPendientes);
	int cantElementos = list_size(listaFinalizacionesPendientes);
	int * socketBusqueda;
	for (i = 0; i < cantElementos; i++) {

		socketBusqueda = (int *) list_get(listaFinalizacionesPendientes, i);

		if (*socketBusqueda == siguientePcb.socketConsola) {

			free(list_remove(listaFinalizacionesPendientes, i));
		}

	}
	pthread_mutex_unlock(&mutexListaFinalizacionesPendientes);

	destruirPcb(siguientePcb.pcb);
}

void manejarIO(t_parametroThreadDispositivoIO * datosHilo) {

	while (1) {

		t_pcbBloqueado pedidoDeIO;
		sem_wait(datosHilo->semaforo);
		pthread_mutex_lock(datosHilo->mutex);
		pedidoDeIO = sacarPrimeroColaBloqueados(datosHilo->colaBloqueados);
		pthread_mutex_unlock(datosHilo->mutex);

		usleep(pedidoDeIO.unidadesTiempoIO * datosHilo->retardoDispositivo * 1000);
		log_info(logger, "Programa pid %d termino IO", pedidoDeIO.pcb.pcb.pid);

		int sizeLista = list_size(listaFinalizacionesPendientes), encontrado = -1, i;

		for (i = 0; i < sizeLista; i++) {

			int * socket = list_get(listaFinalizacionesPendientes, i);
			if (*socket == pedidoDeIO.pcb.socketConsola) {
				encontrado = pedidoDeIO.pcb.socketConsola;
				free(list_remove(listaFinalizacionesPendientes, i));
			}
		}
		if (encontrado == -1) {
			AgregarAProcesoColaListos(pedidoDeIO.pcb);
		} else {
			finalizarProceso(pedidoDeIO.pcb);
		}

	}

}

void crearHilosEntradaSalida() {

	int contador = 0, i;

	while (vectorDispositivos[contador] != NULL) {
		contador++;
	}
	vectorColasBloqueados = malloc(sizeof(t_queue *) * contador);
	vectorMutexDispositivosIO = malloc(sizeof(pthread_mutex_t *) * contador);
	vectorSemaforosDispositivosIO = malloc(sizeof(sem_t) * contador);

	for (i = 0; i < contador; i++) {
		t_parametroThreadDispositivoIO * parametro = malloc(sizeof(t_parametroThreadDispositivoIO));
		vectorColasBloqueados[i] = (t_queue *) queue_create();
		parametro->colaBloqueados = vectorColasBloqueados[i];
		vectorMutexDispositivosIO[i] = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(vectorMutexDispositivosIO[i], NULL);
		parametro->mutex = vectorMutexDispositivosIO[i];
		parametro->retardoDispositivo = atoi(vectorRetardoDispositivos[i]);
		sem_init(&(vectorSemaforosDispositivosIO[i]), 1, 0);
		parametro->semaforo = &(vectorSemaforosDispositivosIO[i]);

		pthread_t nuevoHilo;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&nuevoHilo, &attr, (void *) manejarIO, parametro);
		pthread_attr_destroy(&attr);

	}
}

void ponerEnColaBloqueados(t_pcbConConsola siguientePcb, char * nombre, int largo, int tiempo) {

	int j, sizeLista = list_size(listaFinalizacionesPendientes), encontrado = 0;
	int * socketEnLista;

	pthread_mutex_lock(&mutexListaFinalizacionesPendientes);

	for (j = 0; j < sizeLista; j++) {
		socketEnLista = (int *) list_get(listaFinalizacionesPendientes, j);
		if (siguientePcb.socketConsola == *socketEnLista) {
			list_remove(listaFinalizacionesPendientes, j);
			encontrado = 1;
			j = sizeLista;
		}
	}
	pthread_mutex_unlock(&mutexListaFinalizacionesPendientes);

	if (!encontrado) {
		t_pcbBloqueado pcbBloqueado;
		pcbBloqueado.pcb = siguientePcb;
		pcbBloqueado.unidadesTiempoIO = tiempo;

		int contador = 0, i, existeDispositivo = 0;

		while (vectorDispositivos[contador] != NULL) {
			contador++;
		}

		for (i = 0; i < contador; i++) {

			if (!strcmp(vectorDispositivos[i], nombre)) {
				pthread_mutex_lock(vectorMutexDispositivosIO[i]);
				AgregarAProcesoColaBloqueados(vectorColasBloqueados[i], pcbBloqueado);
				log_info(logger, "Se puso en cola bloqueados de dispositivo %s al proceso pid %d", vectorDispositivos[i], siguientePcb.pcb.pid);
				pthread_mutex_unlock(vectorMutexDispositivosIO[i]);
				sem_post(&(vectorSemaforosDispositivosIO[i]));
				existeDispositivo = 1;
			}

		}
		if (!existeDispositivo) {
			log_error(logger, "No existe el dispositivo solicitado por el proceso pid %d", pcbBloqueado.pcb.pcb.pid);
		}

	} else {
		finalizarProceso(siguientePcb);
	}

}

void destruirPcb(t_pcb pcb) {
	free(pcb.indice_codigo.instrucciones);
	free(pcb.indice_etiquetas.etiquetas);
	list_destroy_and_destroy_elements(pcb.indice_stack, (void *) destruirRegistroStack);
}

void destruirRegistroStack(t_registro_pila * registro) {
	list_destroy_and_destroy_elements(registro->lista_argumentos, free);
	list_destroy_and_destroy_elements(registro->lista_variables, free);
}

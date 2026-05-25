#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <set>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <vector>
#include "juego.h"

using namespace std;

#define MSG_SIZE 250
#define MAX_CLIENTS 30

/*
 * El servidor ofrece el servicio de un chat
 */

void manejador(int signum);
void salirCliente(int socket, fd_set *readfds, int *numClientes, int arrayClientes[], vector<jugador> &jugadores);

int main()
{
    /*----------------------------------------------------
        Variables del juego
    ----------------------------------------------------*/
    char mensaje[MSG_SIZE];
    char usuario[MSG_SIZE];
    char password[MSG_SIZE];
    vector<jugador> jugadores;
    vector<juego> partidas;
    vector<carta> baraja;
    set<string> usuariosConectados;

    /*----------------------------------------------------
        Descriptor del socket y buffer de datos
    -----------------------------------------------------*/
    int sd, new_sd;
    struct sockaddr_in sockname, from;
    char buffer[MSG_SIZE];
    socklen_t from_len;
    fd_set readfds, auxfds;
    int salida;
    int arrayClientes[MAX_CLIENTS];
    int numClientes = 0;
    // contadores
    int i, j, k;
    int recibidos;
    char identificador[MSG_SIZE];

    int on, ret;

    /* --------------------------------------------------
        Se abre el socket
    ---------------------------------------------------*/
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("No se puede abrir el socket cliente\n");
        exit(1);
    }

    // Activaremos una propiedad del socket para permitir· que otros
    // sockets puedan reutilizar cualquier puerto al que nos enlacemos.
    // Esto permite· en protocolos como el TCP, poder ejecutar un
    // mismo programa varias veces seguidas y enlazarlo siempre al
    // mismo puerto. De lo contrario habrÌa que esperar a que el puerto
    // quedase disponible (TIME_WAIT en el caso de TCP)
    on = 1;
    ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    sockname.sin_family = AF_INET;
    sockname.sin_port = htons(2060);
    sockname.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *)&sockname, sizeof(sockname)) == -1)
    {
        perror("Error en la operación bind");
        exit(1);
    }

    /*---------------------------------------------------------------------
        Del las peticiones que vamos a aceptar sólo necesitamos el
        tamaño de su estructura, el resto de información (familia, puerto,
        ip), nos la proporcionará el método que recibe las peticiones.
    ----------------------------------------------------------------------*/
    from_len = sizeof(from);

    if (listen(sd, 1) == -1)
    {
        perror("Error en la operación de listen");
        exit(1);
    }

    printf("El servidor está esperando conexiones...\n"); // Inicializar los conjuntos fd_set

    FD_ZERO(&readfds);
    FD_ZERO(&auxfds);
    FD_SET(sd, &readfds);
    FD_SET(0, &readfds);

    // Capturamos la señal SIGINT (Ctrl+c)
    signal(SIGINT, manejador);

    /*-----------------------------------------------------------------------
        El servidor acepta una petición
    ------------------------------------------------------------------------ */
    while (1)
    {

        // Esperamos recibir mensajes de los clientes (nuevas conexiones o mensajes de los clientes ya conectados)

        auxfds = readfds;

        salida = select(FD_SETSIZE, &auxfds, NULL, NULL, NULL);

        if (salida > 0)
        {

            for (i = 0; i < FD_SETSIZE; i++)
            {

                // Buscamos el socket por el que se ha establecido la comunicación
                if (FD_ISSET(i, &auxfds))
                {

                    if (i == sd)
                    {
                        // aqui aceptamos la peticion de conexion, si se cumple el if, ha fallado la conexion
                        if ((new_sd = accept(sd, (struct sockaddr *)&from, &from_len)) == -1)
                        {
                            perror("Error aceptando peticiones");
                        }
                        else
                        { // aqui entramos cuando se conecta un nuevo cliente
                            if (numClientes < MAX_CLIENTS)
                            {
                                arrayClientes[numClientes] = new_sd;
                                numClientes++;
                                FD_SET(new_sd, &readfds);

                                strcpy(buffer, "Bienvenido al Blackjack\n");

                                send(new_sd, buffer, sizeof(buffer), 0);

                                // Guardo en el vector de jugadores el nuevo jugador
                                jugador j;
                                j.socket = new_sd;
                                j.estado = NO_IDENTIFICADO;
                                jugadores.push_back(j);
                            }
                            else
                            {
                                bzero(buffer, sizeof(buffer));
                                strcpy(buffer, "Demasiados clientes conectados\n");
                                send(new_sd, buffer, sizeof(buffer), 0);
                                close(new_sd);
                            }
                        }
                    }
                    else if (i == 0)
                    {
                        // Se ha introducido información de teclado
                        bzero(buffer, sizeof(buffer));
                        fgets(buffer, sizeof(buffer), stdin);

                        // Controlar si se ha introducido "SALIR", cerrando todos los sockets y finalmente saliendo del servidor. (implementar)
                        if (strcmp(buffer, "SALIR\n") == 0)
                        {

                            for (j = 0; j < numClientes; j++)
                            {
                                bzero(buffer, sizeof(buffer));
                                strcpy(buffer, "Desconexión servidor\n");
                                send(arrayClientes[j], buffer, sizeof(buffer), 0);
                                close(arrayClientes[j]);
                                FD_CLR(arrayClientes[j], &readfds);
                            }
                            close(sd);
                            exit(-1);
                        }
                        // Mensajes que se quieran mandar a los clientes (implementar)
                    }
                    else
                    {
                        bzero(buffer, sizeof(buffer));

                        recibidos = recv(i, buffer, sizeof(buffer), 0);

                        if (recibidos > 0)
                        {
                            if (strcmp(buffer, "SALIR\n") == 0)
                            {
                                jugador j = obtenerJugador(jugadores, i);

                                // Si estaba en mitad de una partida
                                if (j.estado == EN_PARTIDA || j.estado == PLANTADO)
                                {
                                    // Buscamos la partida
                                    for (juego &partida : partidas)
                                    {
                                        if (partida.socket_jugador_1 == i || partida.socket_jugador_2 == i)
                                        {
                                            // Identificamos el socket del otro jugador
                                            int socket_otro_jugador = (partida.socket_jugador_1 == i)
                                                                          ? partida.socket_jugador_2
                                                                          : partida.socket_jugador_1;

                                            jugador otro_jugador = obtenerJugador(jugadores, socket_otro_jugador); // Identificamos al otro jugador

                                            bzero(buffer, sizeof(buffer));
                                            strcpy(buffer, "+Ok. Tu oponente ha terminado la partida");
                                            send(socket_otro_jugador, buffer, sizeof(buffer), 0);

                                            j.estado = NO_IDENTIFICADO; // Desconexión del jugador que sale del servidor

                                            reiniciarPartida(partida); // Ponemos la partida en espera en busca de que otro jugador se conecte

                                            otro_jugador.estado = EN_ESPERA; // El otro jugador se pasa a estado EN_ESPERA hasta que otro jugador se conecte

                                            break; // Rompemos el bucle porque la partida ya ha sido encontrada y reiniciada
                                        }
                                    }
                                }
                                else if (j.estado == EN_ESPERA)
                                {
                                    // Buscamos la partida del jugador
                                    for (juego &partida : partidas)
                                    {
                                        if (partida.socket_jugador_1 == i) // Al estar en espera, él es el jugador 1
                                        {
                                            int indicePartida = obtenerIndicePartida(partidas, i);
                                            eliminarPartida(partidas, indicePartida); // Eliminamos la partida

                                            j.estado = NO_IDENTIFICADO; // Desconexión del jugador que sale del servidor
                                            break;
                                        }
                                    }
                                }

                                // Eliminamos el jugador saliente del conjunto de usuarios conectados
                                usuariosConectados.erase(j.usuario);
                                // Finalmente, desconectamos al jugador y actualizamos la lista de clientes
                                salirCliente(i, &readfds, &numClientes, arrayClientes, jugadores);
                            }
                            else if (sscanf(buffer, "USUARIO %s", mensaje) == 1)
                            {
                                // Si alguien ya ha iniciado sesion con el nombre de usuario que hemos introducido
                                if (usuariosConectados.find(mensaje) != usuariosConectados.end())
                                {
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. El usuario ya está en uso");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                                else
                                {
                                    jugador j = obtenerJugador(jugadores, i);

                                    if (j.estado == NO_IDENTIFICADO)
                                    {
                                        if (comprobarUsuario(jugadores, i, mensaje) == true)
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            strcpy(buffer, "+Ok. Usuario correcto");
                                            send(i, buffer, sizeof(buffer), 0);
                                            usuariosConectados.insert(mensaje); // Agregamos el usuario al conjunto de usarios conectados
                                        }
                                        else
                                        {
                                            bzero(buffer, sizeof(buffer));
                                            strcpy(buffer, "-Err. Usuario incorrecto");
                                            send(i, buffer, sizeof(buffer), 0);
                                        }
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "-Err. Tu estado no te permite este mensaje");
                                        send(i, buffer, sizeof(buffer), 0);
                                    }
                                }
                            }
                            else if (sscanf(buffer, "PASSWORD %s", mensaje) == 1)
                            {
                                jugador j = obtenerJugador(jugadores, i);

                                if (j.estado == CONECTADO)
                                {
                                    if (comprobarPassword(jugadores, i, mensaje) == 1)
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "+Ok. Usuario validado");
                                        send(i, buffer, sizeof(buffer), 0);
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "-Err. Error en la validación");
                                        send(i, buffer, sizeof(buffer), 0);
                                    }
                                }
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. Tu estado no te permite este mensaje");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                            }
                            else if (sscanf(buffer, "REGISTRO -u %s -p %s", usuario, password) == 2)
                            {
                                jugador j = obtenerJugador(jugadores, i);

                                if (j.estado == NO_IDENTIFICADO)
                                {
                                    int resultado = registrarUsuario(usuario, password);

                                    switch (resultado)
                                    {
                                    case -1:
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "-Err. Error al abrir el fichero");
                                        send(i, buffer, sizeof(buffer), 0);
                                        break;

                                    case 0:
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "-Err. Colisión de usuario");
                                        send(i, buffer, sizeof(buffer), 0);
                                        break;

                                    case 1:
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "+Ok. Usuario registrado");
                                        send(i, buffer, sizeof(buffer), 0);

                                        // Cuando registramos un usuario nuevo, inicia sesión automáticamente
                                        j.estado = VALIDADO;
                                        break;
                                    }
                                }
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. Tu estado no te permite este mensaje");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                            }
                            else if (strcmp(buffer, "INICIAR-PARTIDA\n") == 0)
                            {
                                jugador j = obtenerJugador(jugadores, i);
                                if (j.estado == VALIDADO)
                                {
                                    vector<carta> cartas_jugador_1, cartas_jugador_2; // Vectores para almacenar las cartas
                                    int encontrado = buscarPartida(partidas, i, cartas_jugador_1, cartas_jugador_2, jugadores);
                                    if (partidas[encontrado].estado == 1)
                                    {
                                        // Crear el mensaje para el jugador 1
                                        sprintf(buffer, "+Ok. Empieza la partida. TUS-CARTAS:[%s,%d][%s,%d].OPONENTE:[%s,%d]",
                                                cartas_jugador_1[0].palo, cartas_jugador_1[0].numero,
                                                cartas_jugador_1[1].palo, cartas_jugador_1[1].numero,
                                                cartas_jugador_2[0].palo, cartas_jugador_2[0].numero);

                                        // Enviar mensaje al jugador 1
                                        send(partidas[encontrado].socket_jugador_1, buffer, sizeof(buffer), 0);
                                        bzero(buffer, sizeof(buffer)); // Limpiamos el buffer

                                        // Crear el mensaje para el jugador 2
                                        sprintf(buffer, "+Ok. Empieza la partida. TUS-CARTAS:[%s,%d][%s,%d].OPONENTE:[%s,%d]",
                                                cartas_jugador_2[0].palo, cartas_jugador_2[0].numero,
                                                cartas_jugador_2[1].palo, cartas_jugador_2[1].numero,
                                                cartas_jugador_1[0].palo, cartas_jugador_1[0].numero);

                                        // Enviar mensaje al jugador 2
                                        send(partidas[encontrado].socket_jugador_2, buffer, sizeof(buffer), 0);
                                    }
                                    else
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "+Ok. Esperando otro jugador");
                                        send(i, buffer, sizeof(buffer), 0);
                                    }
                                }
                            }
                            else if (strcmp(buffer, "PEDIR-CARTA\n") == 0)
                            {
                                jugador j = obtenerJugador(jugadores, i);
                                if (j.estado == EN_PARTIDA)
                                {
                                    // Encontrar la partida del jugador
                                    for (juego &partida : partidas)
                                    {
                                        if (partida.socket_jugador_1 == i || partida.socket_jugador_2 == i)
                                        {
                                            // Determinamos el socket del otro jugador
                                            int socket_otro_jugador = (partida.socket_jugador_1 == i)
                                                                          ? partida.socket_jugador_2
                                                                          : partida.socket_jugador_1;

                                            jugador otro_jugador = obtenerJugador(jugadores, socket_otro_jugador);
                                            bool turno;

                                            // Si el otro jugador no se ha plantado, verificar que es el turno del jugador que pide la carta
                                            if (otro_jugador.estado == EN_PARTIDA)
                                            {
                                                turno = tuTurno(partida, i);
                                            }
                                            else // Si el otro jugador se ha plantado, será turno del jugador que pide la carta, hasta que se plante o supere los 21 puntos
                                            {
                                                turno = true;
                                            }

                                            if (turno == true)
                                            {
                                                // Intentamos pedir una carta
                                                int resultado = pedirCarta(partida, i);

                                                switch (resultado)
                                                {
                                                case -3:
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-Err. No hay más cartas disponibles");
                                                    send(i, buffer, sizeof(buffer), 0);
                                                    break;

                                                case -2:
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-Err. Excedido el valor de 21");
                                                    send(i, buffer, sizeof(buffer), 0);
                                                    break;

                                                case -1:
                                                    bzero(buffer, sizeof(buffer));
                                                    strcpy(buffer, "-Err. Jugador no encontrado en la partida");
                                                    send(i, buffer, sizeof(buffer), 0);
                                                    break;

                                                default:
                                                    bzero(buffer, sizeof(buffer));
                                                    carta c = quienPidioCarta(partida, i); // Verificar quién pidió la carta para obtener su última carta
                                                    sprintf(buffer, "+Ok.[%s, %d]", c.palo, c.numero);
                                                    send(i, buffer, sizeof(buffer), 0);

                                                    // Alternamos el turno
                                                    cambiarTurno(partida);
                                                    break;
                                                }
                                                break; // Ya encontramos la partida, salimos del bucle
                                            }
                                            else
                                            {
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "-Err. No es tu turno");
                                                send(i, buffer, sizeof(buffer), 0);
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. Tu estado no te permite este mensaje");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                            }
                            else if (strcmp(buffer, "PLANTARME\n") == 0)
                            {
                                jugador *j = obtenerJugadorPtr(jugadores, i);

                                if (j->estado == EN_PARTIDA)
                                {
                                    // Buscamos la partida del jugador
                                    for (juego &partida : partidas)
                                    {
                                        if (partida.socket_jugador_1 == i || partida.socket_jugador_2 == i)
                                        {
                                            // Determinamos el socket del otro jugador
                                            int socket_otro_jugador = (partida.socket_jugador_1 == i)
                                                                          ? partida.socket_jugador_2  // Si se cumple el if
                                                                          : partida.socket_jugador_1; // Si no hace el else

                                            jugador otro_jugador = obtenerJugador(jugadores, socket_otro_jugador);
                                            bool turno;

                                            // Si el otro jugador no se ha plantado, verificar si es el turno del jugador que pide plantarse
                                            if (otro_jugador.estado == EN_PARTIDA)
                                            {
                                                turno = tuTurno(partida, i);
                                            }
                                            else // Si el otro jugador ya se ha plantado, será turno del que pide plantarse ahora
                                            {
                                                turno = true;
                                            }

                                            if (turno == true)
                                            {
                                                // Modificamos el estado del jugador a PLANTADO para que no se le vuelva a conceder el turno
                                                j->estado = PLANTADO;

                                                // Verificamos si es el primer jugador en plantarse, o si se han plantado los dos
                                                if (otro_jugador.estado != PLANTADO)
                                                {
                                                    // Es el primer jugador en plantarse
                                                    bzero(buffer, sizeof(buffer));
                                                    sprintf(buffer, "+Ok. Esperando que finalice el otro jugador");
                                                    send(i, buffer, sizeof(buffer), 0);

                                                    // Alternamos el turno para que el otro jugador pueda seguir jugando
                                                    cambiarTurno(partida);
                                                }
                                                else
                                                {
                                                    // Ambos jugadores se han plantado, evaluar el resultado de la partida
                                                    int resultado = ganador(partida, jugadores);

                                                    jugador j1 = obtenerJugador(jugadores, partida.socket_jugador_1);
                                                    jugador j2 = obtenerJugador(jugadores, partida.socket_jugador_2);

                                                    // Obtenemos el índice de la partida del jugador sin crear una nueva
                                                    int indice_partida = obtenerIndicePartida(partidas, i);

                                                    switch (resultado)
                                                    {
                                                    case -1:
                                                        bzero(buffer, sizeof(buffer));
                                                        strcpy(buffer, "+Ok. No hay ganadores");
                                                        send(partida.socket_jugador_1, buffer, sizeof(buffer), 0);
                                                        send(partida.socket_jugador_2, buffer, sizeof(buffer), 0);

                                                        // Cambiamos el estado de los jugadores a VALIDADO, antes de salir, para que puedan volver a iniciar otra partida si lo desean
                                                        cambiarEstado(jugadores, partida.socket_jugador_1, partida.socket_jugador_2);
                                                        eliminarPartida(partidas, indice_partida); // Eliminamos la partida del vector de partidas mediante el indice
                                                        break;

                                                    case 0:
                                                        bzero(buffer, sizeof(buffer));
                                                        sprintf(buffer, "+Ok. Jugador %s y Jugador %s habéis empatado la partida", j1.usuario.c_str(), j2.usuario.c_str());
                                                        send(partida.socket_jugador_1, buffer, sizeof(buffer), 0);
                                                        send(partida.socket_jugador_2, buffer, sizeof(buffer), 0);

                                                        // Cambiamos el estado de los jugadores a VALIDADO, antes de salir, para que puedan volver a iniciar otra partida si lo desean
                                                        cambiarEstado(jugadores, partida.socket_jugador_1, partida.socket_jugador_2);
                                                        eliminarPartida(partidas, indice_partida); // Eliminamos la partida del vector de partidas mediante el indice
                                                        break;

                                                    case 1:
                                                        bzero(buffer, sizeof(buffer));
                                                        sprintf(buffer, "+Ok. Jugador %s ha ganado la partida", j1.usuario.c_str()); // c_str() se usa porque la funcion espera un char *
                                                        send(partida.socket_jugador_1, buffer, sizeof(buffer), 0);                   // y j1.usuario es de tipo string
                                                        send(partida.socket_jugador_2, buffer, sizeof(buffer), 0);                   // es igual en el resto de casos

                                                        // Cambiamos el estado de los jugadores a VALIDADO, antes de salir, para que puedan volver a iniciar otra partida si lo desean
                                                        cambiarEstado(jugadores, partida.socket_jugador_1, partida.socket_jugador_2);
                                                        eliminarPartida(partidas, indice_partida); // Eliminamos la partida del vector de partidas mediante el indice
                                                        break;

                                                    case 2:
                                                        bzero(buffer, sizeof(buffer));
                                                        sprintf(buffer, "+Ok. Jugador %s ha ganado la partida", j2.usuario.c_str());
                                                        send(partida.socket_jugador_1, buffer, sizeof(buffer), 0);
                                                        send(partida.socket_jugador_2, buffer, sizeof(buffer), 0);

                                                        // Cambiamos el estado de los jugadores a VALIDADO, antes de salir, para que puedan volver a iniciar otra partida si lo desean
                                                        cambiarEstado(jugadores, partida.socket_jugador_1, partida.socket_jugador_2);
                                                        eliminarPartida(partidas, indice_partida); // Eliminamos la partida del vector de partidas mediante el indice
                                                        break;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                bzero(buffer, sizeof(buffer));
                                                strcpy(buffer, "-Err. No es tu turno");
                                                send(i, buffer, sizeof(buffer), 0);
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. Tu estado no te permite este mensaje");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                            }
                            else // Si el mensaje introducido no es ninguno de los paquetes del enunciado
                            {
                                bzero(buffer, sizeof(buffer));
                                strcpy(buffer, "-Err. El mensaje introducido no se ha reconocido");
                                send(i, buffer, sizeof(buffer), 0);
                            }
                        }
                        // Si el cliente introdujo ctrl+c
                        if (recibidos == 0)
                        {
                            printf("El socket %d, ha introducido ctrl+c\n", i);

                            jugador j = obtenerJugador(jugadores, i);

                            // Si estaba en mitad de una partida
                            if (j.estado == EN_PARTIDA || j.estado == PLANTADO)
                            {
                                // Buscamos la partida
                                for (juego &partida : partidas)
                                {
                                    if (partida.socket_jugador_1 == i || partida.socket_jugador_2 == i)
                                    {
                                        // Identificamos el socket del otro jugador
                                        int socket_otro_jugador = (partida.socket_jugador_1 == i)
                                                                      ? partida.socket_jugador_2
                                                                      : partida.socket_jugador_1;

                                        jugador otro_jugador = obtenerJugador(jugadores, socket_otro_jugador); // Identificamos al otro jugador

                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "+Ok. Tu oponente ha terminado la partida");
                                        send(socket_otro_jugador, buffer, sizeof(buffer), 0);

                                        j.estado = NO_IDENTIFICADO; // Desconexión del jugador que sale del servidor

                                        reiniciarPartida(partida); // Ponemos la partida en espera en busca de que otro jugador se conecte

                                        otro_jugador.estado = EN_ESPERA; // El otro jugador se pasa a estado EN_ESPERA hasta que otro jugador se conecte

                                        break; // Rompemos el bucle porque la partida ya ha sido encontrada y reiniciada
                                    }
                                }
                            }
                            else if (j.estado == EN_ESPERA)
                            {
                                // Buscamos la partida del jugador
                                for (juego &partida : partidas)
                                {
                                    if (partida.socket_jugador_1 == i) // Al estar en espera, él es el jugador 1
                                    {
                                        int indicePartida = obtenerIndicePartida(partidas, i);
                                        eliminarPartida(partidas, indicePartida); // Eliminamos la partida

                                        j.estado = NO_IDENTIFICADO; // Desconexión del jugador que sale del servidor
                                        break;
                                    }
                                }
                            }

                            // Eliminamos el jugador saliente del conjunto de usuarios conectados
                            usuariosConectados.erase(j.usuario);
                            // Finalmente, desconectamos al jugador y actualizamos la lista de clientes
                            salirCliente(i, &readfds, &numClientes, arrayClientes, jugadores);
                        }
                    }
                }
            }
        }
    }

    close(sd);
    return 0;
}

void salirCliente(int socket, fd_set *readfds, int *numClientes, int arrayClientes[], vector<jugador> &jugadores)
{
    char buffer[250];
    int j;

    // Cambiar el estado del jugador asociado a este socket a NO_IDENTIFICADO
    jugador *jugadorSaliente = obtenerJugadorPtr(jugadores, socket);
    if (jugadorSaliente != nullptr)
    {
        jugadorSaliente->estado = NO_IDENTIFICADO; // Cambiar estado a NO_IDENTIFICADO
    }

    // Cerrar el socket y eliminarlo del conjunto de descriptores
    close(socket);
    FD_CLR(socket, readfds);

    // Buscar el índice del cliente que se desconecta
    for (j = 0; j < *numClientes; j++)
    {
        if (arrayClientes[j] == socket)
            break; // Encontramos el socket a eliminar
    }

    // Reestructurar el array de clientes, moviendo los demás hacia abajo
    for (int k = j; k < *numClientes - 1; k++)
    {
        arrayClientes[k] = arrayClientes[k + 1]; // Desplazar el resto de clientes
    }

    // Reducimos el número de clientes
    (*numClientes)--;

    // Preparar el mensaje de desconexión
    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Desconexión del cliente <%d>", socket);

    // Informar a los otros clientes de la desconexión
    for (j = 0; j < *numClientes; j++)
    {
        send(arrayClientes[j], buffer, sizeof(buffer), 0);
    }
}

void manejador(int signum)
{
    printf("\nSe ha recibido la señal sigint\n");
    signal(SIGINT, manejador);

    // Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}

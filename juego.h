#ifndef JUEGO_H
#define JUEGO_H

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>
#include <stdio.h>

using namespace std;

// Definir los distintos estados de un jugador
#define NO_IDENTIFICADO 0
#define CONECTADO 1
#define VALIDADO 2
#define EN_ESPERA 3
#define EN_PARTIDA 4
#define PLANTADO 5

struct carta
{
  int numero;
  char palo[20];  // El palo de la carta (corazones, diamantes, tréboles, picas)
  int disponible; // 1 si, 0 no
};

struct jugador
{
  int socket;
  string usuario;
  string password;
  int estado;
};

struct juego
{
  int socket_jugador_1;
  int socket_jugador_2;
  vector<carta> cartas_jugador_1;
  vector<carta> cartas_jugador_2;
  vector<carta> baraja_crupier;
  int estado; // 0 un jugador esperando, 1 le toca al jugador1, 2 le toca al jugador2
};

jugador obtenerJugador(vector<jugador> jugadores, int socket)
{
  for (size_t i = 0; i < jugadores.size(); i++)
  {
    if (jugadores[i].socket == socket)
    {
      return jugadores[i];
    }
  }
  // Esto lo ponemos por si el jugador no existiese, no de error al compilar, pero en verdad el jugador siempre va a existir
  jugador aux;
  return aux;
}

// Misma funcion que la anterior pero con un puntero para poder modificar el jugador
jugador *obtenerJugadorPtr(vector<jugador> &jugadores, int socket)
{
  for (jugador &j : jugadores)
  {
    if (j.socket == socket)
    {
      return &j; // Devolvemos el puntero al jugador encontrado
    }
  }
  // Esto lo ponemos por si el jugador no existiese, no de error al compilar, pero en verdad el jugador siempre va a existir
  return nullptr;
}

bool comprobarUsuario(vector<jugador> &jugadores, int socket, char *usuarioIntroducido)
{
  ifstream fich("usuarios.txt");
  if (fich.is_open() == false)
  {
    return false;
  }

  string usuarioLeido, passLeida;
  while (getline(fich, usuarioLeido, ' '))
  {
    getline(fich, passLeida, '\n');
    if (string(usuarioIntroducido) == usuarioLeido)
    {
      // busco al usuario en el vector de jugadores
      for (size_t i = 0; i < jugadores.size(); i++)
      {
        if (jugadores[i].socket == socket)
        {
          jugadores[i].usuario = usuarioLeido;
          jugadores[i].password = passLeida;
          jugadores[i].estado = CONECTADO;
          fich.close();
          return true;
        }
      }
    }
  }
  fich.close();
  return false;
}

bool comprobarPassword(vector<jugador> &jugadores, int socket, char *passIntroducida)
{
  ifstream fich("usuarios.txt");
  if (fich.is_open() == false)
  {
    return false; // No se pudo abrir el archivo
  }

  string usuarioLeido, passLeida;

  // Buscar al jugador con el socket especificado en el vector de jugadores
  for (size_t i = 0; i < jugadores.size(); i++)
  {
    if (jugadores[i].socket == socket)
    {
      // Leer el archivo y buscar el nombre de usuario del jugador
      while (getline(fich, usuarioLeido, ' '))
      {
        getline(fich, passLeida, '\n'); // Leer la contraseña del archivo

        // Si encontramos el nombre de usuario en el archivo
        if (jugadores[i].usuario == usuarioLeido)
        {
          // Comparamos la contraseña introducida con la contraseña del archivo
          if (string(passIntroducida) == passLeida)
          {
            jugadores[i].estado = VALIDADO;
            fich.close();
            return true; // Contraseña correcta
          }
          else
          {
            fich.close();
            return false; // Contraseña incorrecta
          }
        }
      }
    }
  }
  // No se encontró al jugador
  fich.close();
  return false;
}

int usuarioExiste(char *nombreUsuario)
{
  FILE *f = fopen("usuarios.txt", "r"); // Abrimos el archivo en modo lectura
  if (f == NULL)
  {
    return -1; // Error, no se pudo abrir el fihero
  }

  char usuario[100], pass[250];
  while (fscanf(f, "%s %s", usuario, pass) != EOF)
  {
    if (strcmp(usuario, nombreUsuario) == 0)
    {
      fclose(f);
      return 1; // El usuario introducido ya existe
    }
  }

  fclose(f);
  return 0; // El usuario introducido no existe
}

int registrarUsuario(char *usuarioIntroducido, char *passIntroducida)
{
  // Primero verificamos si el nombre de usuario introducido esta disponible
  int colision = usuarioExiste(usuarioIntroducido);

  if (colision == -1)
  {
    return -1; // Ha ocurrido un error al abrir el fichero
  }
  else if (colision == 1)
  {
    return 0; // Colision de nombres de usuario
  }

  // Si colision == 0, significa que el nombre de usuario introducido no existe, es decir, esta libre
  FILE *f = fopen("usuarios.txt", "a"); // En modo "a" para añadir sin sobreescribir
  if (f == NULL)
  {
    return -1; // Error al abrir el fichero;
  }

  // Añadimos el usuario nuevo
  fprintf(f, "%s %s\n", usuarioIntroducido, passIntroducida);
  fclose(f);
  return 1; // Usuario registrado con éxito
}

void barajarCartas(carta baraja[], int tam)
{
  srand(time(0)); // Inicializamos la semilla de números aleatorios

  for (int i = tam - 1; i > 0; i--)
  {
    // Elegimos un índice aleatorio entre 0 y i
    int j = rand() % (i + 1);

    // Intercambiamos la carta en la posición i con la carta en la posición j
    carta temp = baraja[i];
    baraja[i] = baraja[j];
    baraja[j] = temp;
  }
}

vector<carta> Baraja()
{
  std::vector<carta> baraja(52); // Crea un vector de 52 cartas
  char palos[4][20] = {"Corazones", "Diamantes", "Tréboles", "Picas"};
  int indice = 0;

  for (int p = 0; p < 4; p++) // Recorre los 4 palos
  {
    for (int n = 1; n <= 13; n++) // Recorre las 13 cartas de cada palo
    {
      if (n == 1) // Caso para el As
      {
        baraja[indice].numero = 1;
      }
      else if (n >= 11) // Caso para J, Q, K
      {
        baraja[indice].numero = 10;
      }
      else
      {
        baraja[indice].numero = n; // Cartas del 2 al 10 valen su número
      }
      strcpy(baraja[indice].palo, palos[p]); // Copia el nombre del palo en la carta
      baraja[indice].disponible = 1;         // Marca la carta como disponible
      indice++;                              // Avanza al siguiente índice del vector baraja
    }
  }
  // Una vez que la baraja ha sido rellenada, la barajamos
  barajarCartas(baraja.data(), baraja.size());

  // Retornar las cartas ya barajadas
  return baraja;
}

int buscarPartida(vector<juego> &partidas, int socket_jugador, vector<carta> &cartas_jugador_1, vector<carta> &cartas_jugador_2, vector<jugador> &jugadores)
{
  for (int i = 0; i < partidas.size(); i++)
  {
    if (partidas[i].estado == 0) // Si se encuentra una partida esperando a un jugador
    {
      partidas[i].socket_jugador_2 = socket_jugador;
      // Inicializamos la partida
      partidas[i].baraja_crupier = Baraja(); // Creamos la baraja y la barajamos
      partidas[i].cartas_jugador_1.clear();  // Limpiamos el vector del j1
      partidas[i].cartas_jugador_1.push_back(partidas[i].baraja_crupier[0]);
      partidas[i].cartas_jugador_1.push_back(partidas[i].baraja_crupier[1]);
      partidas[i].cartas_jugador_2.clear(); // Limpiamos el vector del j2
      partidas[i].cartas_jugador_2.push_back(partidas[i].baraja_crupier[2]);
      partidas[i].cartas_jugador_2.push_back(partidas[i].baraja_crupier[3]);

      // Marcamos como no disponibles las cartas repartidas por el crupier
      partidas[i].baraja_crupier[0].disponible = 0;
      partidas[i].baraja_crupier[1].disponible = 0;
      partidas[i].baraja_crupier[2].disponible = 0;
      partidas[i].baraja_crupier[3].disponible = 0;

      // Marcamos como disponibles las cartas repartidas por el crupier a j1
      partidas[i].cartas_jugador_1[0].disponible = 1;
      partidas[i].cartas_jugador_1[1].disponible = 1;

      // Marcamos como disponibles las cartas repartidas por el crupier a j2
      partidas[i].cartas_jugador_2[0].disponible = 1;
      partidas[i].cartas_jugador_2[1].disponible = 1;

      partidas[i].estado = 1; // Se inicia la partida

      // Cambiamos el estado del jugador 1 y el jugador 2 a EN_PARTIDA
      for (jugador &j : jugadores)
      {
        if (j.socket == partidas[i].socket_jugador_1 || j.socket == partidas[i].socket_jugador_2)
        {
          j.estado = EN_PARTIDA;
        }
      }

      // Asignamos las cartas a los vectores externos para el uso en el servidor
      cartas_jugador_1 = partidas[i].cartas_jugador_1;
      cartas_jugador_2 = partidas[i].cartas_jugador_2;

      return i; // Retornamos el índice de la partida que se acaba de llenar
    }
  }

  // Si no se encontró partida disponible, se crea una nueva partida
  juego partidaNueva;
  partidaNueva.socket_jugador_1 = socket_jugador;
  partidaNueva.estado = 0;
  partidas.push_back(partidaNueva);

  // Cambiamos el estado del jugador a EN_ESPERA
  for (jugador &j : jugadores)
  {
    if (j.socket == socket_jugador)
    {
      j.estado = EN_ESPERA;
      break;
    }
  }

  return partidas.size() - 1; // Retorna el índice de la nueva partida creada
}

bool tuTurno(juego &partida, int socket_jugador)
{
  if (partida.estado == 1 && socket_jugador == partida.socket_jugador_1)
  {
    return true; // Es el turno del jugador 1
  }
  else if (partida.estado == 2 && socket_jugador == partida.socket_jugador_2)
  {
    return true; // Es el turno del jugador 2
  }
  else
  {
    return false; // No es el turno del jugador
  }
}

int calcularPuntos(const vector<carta> &cartas)
{
  int puntos = 0;
  int ases = 0;

  // Sumar los puntos de las cartas
  for (const carta &c : cartas)
  {
    if (c.numero >= 2 && c.numero <= 10)
    {
      puntos += c.numero;
    }
    else if (c.numero >= 11 && c.numero <= 13)
    {
      puntos += 10; // J, Q, K valen 10 puntos
    }
    else if (c.numero == 1)
    {
      ases++;       // Contar los ases para tratarlos luego
      puntos += 11; // Inicialmente, tratamos el as como 11
    }
  }

  // Ajustar el valor de los ases si es necesario
  while (puntos > 21 && ases > 0)
  {
    puntos -= 10; // Convertir el as de 11 a 1
    ases--;
  }

  return puntos;
}

int pedirCarta(juego &partida, int socket_jugador)
{
  // Encontrar qué jugador está pidiendo la carta
  vector<carta> *cartas_jugador = nullptr;
  if (partida.socket_jugador_1 == socket_jugador)
  {
    cartas_jugador = &(partida.cartas_jugador_1);
  }
  else if (partida.socket_jugador_2 == socket_jugador)
  {
    cartas_jugador = &(partida.cartas_jugador_2);
  }
  else
  {
    return -1; // Error: jugador no encontrado
  }

  // Calcular puntos antes de dar la carta
  int puntos_antes = calcularPuntos(*cartas_jugador);
  if (puntos_antes > 21)
  {
    return -2; // Error: jugador ya ha superado los 21 puntos, no puede pedir más cartas
  }

  // Buscar la primera carta disponible en la baraja del crupier
  for (auto &c : partida.baraja_crupier)
  {
    if (c.disponible)
    {
      // Dar la carta al jugador
      cartas_jugador->push_back(c);
      c.disponible = 0; // Marcar la carta como no disponible

      return c.numero; // Retornar el número de la carta pedida
    }
  }

  return -3; // Error: no hay cartas disponibles
}

void cambiarTurno(juego &partida)
{
  if (partida.estado == 1)
  {
    partida.estado = 2; // Cambia a turno del jugador 2
  }
  else
  {
    partida.estado = 1; // Cambia a turno del jugador 1
  }
}

carta quienPidioCarta(const juego &partida, int socket_jugador)
{
  carta c; // Variable para almacenar la carta solicitada

  if (partida.socket_jugador_1 == socket_jugador)
  {
    c = partida.cartas_jugador_1.back(); // Si es el jugador 1, obtener su última carta
  }
  else if (partida.socket_jugador_2 == socket_jugador)
  {
    c = partida.cartas_jugador_2.back(); // Si es el jugador 2, obtener su última carta
  }

  return c; // Devolver la carta encontrada
}

int ganador(juego &partida, vector<jugador> &jugadores)
{
  // Calculamos los puntos de cada jugador
  int puntos_jugador1 = calcularPuntos(partida.cartas_jugador_1);
  int puntos_jugador2 = calcularPuntos(partida.cartas_jugador_2);

  // Comprobamos si ambos jugadores han superado los 21 puntos
  if (puntos_jugador1 > 21 && puntos_jugador2 > 21)
  {
    return -1; // Ambos jugadores superaron los 21 puntos (no hay ganadores)
  }

  // Si ambos jugadores tienen el mismo número de puntos sin exceder 21
  if (puntos_jugador1 == puntos_jugador2)
  {
    return 0; // Empate
  }

  // Comprobamos quién tiene el puntaje más cercano a 21 sin excederlo
  if (puntos_jugador1 <= 21 && (puntos_jugador1 > puntos_jugador2 || puntos_jugador2 > 21))
  {
    return 1; // Jugador 1 gana
  }
  else if (puntos_jugador2 <= 21 && (puntos_jugador2 > puntos_jugador1 || puntos_jugador1 > 21))
  {
    return 2; // Jugador 2 gana
  }

  // Para que compile sin warnings
  return -1;
}

void cambiarEstado(vector<jugador> &jugadores, int socket_jugador_1, int socket_jugador_2)
{
  for (jugador &j : jugadores)
  {
    // Verificamos si el jugador es uno de los dos de la partida finalizada
    if (j.socket == socket_jugador_1 || j.socket == socket_jugador_2)
    {
      j.estado = VALIDADO; // Cambiamos el estado del jugador a VALIDADO
    }
  }
}

void eliminarPartida(vector<juego> &partidas, int indicePartida)
{
  if (indicePartida >= 0 && indicePartida < partidas.size())
  {
    partidas.erase(partidas.begin() + indicePartida); // Elimina la partida en la posición del índice
  }
}

int obtenerIndicePartida(vector<juego> &partidas, int socket_jugador)
{
  for (size_t i = 0; i < partidas.size(); ++i)
  {
    juego &partida = partidas[i];
    if (partida.socket_jugador_1 == socket_jugador || partida.socket_jugador_2 == socket_jugador)
    {
      return i; // Retorna el índice de la partida encontrada
    }
  }
  return -1; // Si no se encuentra la partida, devuelve -1
}

void reiniciarPartida(juego &partida)
{
  // Vaciar las cartas de los jugadores y del crupier
  partida.cartas_jugador_1.clear();
  partida.cartas_jugador_2.clear();
  partida.baraja_crupier.clear();

  // Reiniciar estado de la partida para esperar a que un jugador se una
  partida.estado = 0; // Estado 0 indica que la partida está en espera de un nuevo jugador
}

#endif
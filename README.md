# **Red vs Blue**

**Capture the Flag multijugador desarrollado en Unreal Engine 5.6.1 utilizando C++ y Blueprints.**

---

[**LINK AL REPOSITORIO**](https://github.com/Capogrossi32bit/RedVSBlue/tree/main)

[**DESCARGAR BUILD PARA WINDOWS**](https://drive.google.com/file/d/1YrhpJayb4dqYZxSnu0GX3Hz_KH2aw9kC/view?usp=drive_link)

---

## **1\. Descripción**

Red vs Blue es un videojuego multijugador competitivo en tercera persona basado en el modo de juego **Capture the Flag (CTF)**.

Los jugadores son divididos en dos equipos, rojo y azul, que compiten por capturar la bandera enemiga y transportarla hasta su propia base para obtener puntos.

El proyecto implementa combate con armas de fuego, respawn, gestión de equipos, estadísticas individuales y un sistema de partidas online basado en sesiones de Steam.

## **2\. Tecnologías utilizadas**

* **Motor:** Unreal Engine 5.6.1.  
* **Lenguajes:** C++ y Blueprints.  
* **Networking:** sistema de replicación y RPC de Unreal Engine.  
* **Input:** Enhanced Input.  
* **Interfaz:** Unreal Motion Graphics (UMG).  
* **Multiplayer online:** Online Subsystem Steam y Steam Sockets.  
* **Gestión de sesiones:** plugin MultiplayerSessionOW.  
* **Plataforma de compilación:** Windows 64 bits.

## **3\. Mecánicas principales**

### **Equipos**

Los jugadores son asignados automáticamente a uno de los dos equipos, buscando mantener una distribución equilibrada.

Cada equipo dispone de puntos de aparición propios y una bandera que debe proteger.

### **Captura de bandera**

El objetivo consiste en recoger la bandera enemiga y llevarla hasta la propia base.

* Las banderas pueden encontrarse en su base, siendo transportadas o caídas.  
* Un jugador solamente puede transportar una bandera enemiga a la vez.  
* Si el portador muere, deja caer la bandera.  
* Los integrantes del equipo propietario pueden recuperar una bandera caída.  
* Para capturar una bandera enemiga, la bandera propia debe encontrarse en su base.

### **Combate**

Se implementó un sistema de disparos mediante Line Traces, validado por el servidor.

El sistema incluye daño, munición, reserva de balas, recarga, cadencia de fuego, animaciones y efectos de sonido.

La salud de los jugadores se administra mediante un componente reutilizable.

### **Muerte y respawn**

Cuando la salud de un jugador llega a cero:

* Se registra su muerte.  
* Se actualizan las estadísticas del atacante cuando corresponde.  
* Se suelta la bandera transportada, si existe.  
* Se destruye el personaje y se programa su reaparición.  
* El jugador reaparece en un punto correspondiente a su equipo mientras la partida continúe.

### **Ciclo de partida**

Las partidas se dividen en tres fases:

1. **Waiting:** período de preparación.  
2. **Playing:** desarrollo de la partida.  
3. **Finished:** finalización y presentación del resultado.

Al terminar el tiempo, se comparan las puntuaciones para determinar victoria, derrota o empate.

## **4\. Arquitectura del proyecto**

El proyecto combina clases C++ con Blueprints derivados para separar la lógica de funcionamiento de la configuración visual.

| Clase | Responsabilidad |
| ----- | ----- |
| `ARedVsBlueGameModeBase` | Reglas, equipos, respawn, puntuación y fases |
| `ARedVsBlueGameState` | Estado compartido, temporizador, puntuaciones y ganador |
| `ARedVsBluePlayerState` | Equipo y estadísticas individuales |
| `ARedVsBluePlayerController` | Entrada, HUD y regreso al menú |
| `ARedVsBlueCharacter` | Movimiento, combate, interacción y transporte de banderas |
| `UHealthComponent` | Salud, recepción de daño y muerte |
| `AFlag` | Estados e interacción de las banderas |
| `ATeamPlayerStart` | Puntos de aparición por equipo |
| `URedVsBlueHUDWidget` | Información de la partida e interfaz de combate |
| `UMenuLan` | Creación y conexión a partidas LAN |

### **Autoridad del servidor**

El proyecto utiliza un modelo de networking con autoridad del servidor.

Las acciones que afectan el estado compartido, como aplicar daño, capturar banderas, asignar equipos y modificar puntuaciones, son validadas o ejecutadas por el servidor.

### **Replicación**

Se utilizan propiedades replicadas para sincronizar entre servidor y clientes:

* Salud.  
* Munición y estado de recarga.  
* Equipo del jugador.  
* Kills y deaths.  
* Estado de las banderas.  
* Puntuaciones.  
* Tiempo restante.  
* Fase de la partida.  
* Equipo ganador.

Los RPC `Server` permiten solicitar acciones desde el cliente, mientras que los RPC `NetMulticast` comunican determinados efectos audiovisuales a los participantes.

Se utilizan funciones `OnRep` cuando es necesario reaccionar a cambios de propiedades replicadas.

## **5\. Multiplayer**

El juego dispone de dos modalidades de conexión.

**LAN:** permite alojar una partida local o conectarse mediante la dirección IP del anfitrión.

**Online:** utiliza Steam Online Subsystem, Steam Sockets y el plugin MultiplayerSessionOW para la creación, búsqueda y conexión a sesiones multijugador.

Las pruebas online se realizaron utilizando Steam App ID `480` (*Spacewar*), como identificador de desarrollo y pruebas.

Este identificador no representa una publicación comercial del juego.

## **6\. Cómo ejecutar el juego**

### **Build de Windows**

1. Descomprimir el archivo de la build completa.  
2. Ejecutar `RedVsBlue.exe`.  
3. Seleccionar Partida Online o Partida LAN desde el menú principal.  
4. Crear una partida o conectarse a una existente.  
5. Jugar hasta la finalización de la partida.

Para las pruebas online, iniciar sesión en Steam con una cuenta válida. Los participantes deben utilizar builds compatibles y tener Steam disponible.

### **Proyecto Unreal Engine**

1. Disponer de Unreal Engine 5.6.1 y las herramientas de compilación C++ requeridas.  
2. Abrir `RedVsBlue.uproject`.  
3. Verificar que los plugins necesarios estén disponibles y habilitados.  
4. Compilar los módulos C++ si el editor lo solicita.  
5. Abrir `MainLevel` para acceder al menú principal o `LV_BattleCreekUpdated` para probar el gameplay.

**Nota:** las sesiones Steam deben comprobarse en una build empaquetada; las pruebas multiplayer dentro de PIE se realizan mediante el entorno de networking del editor.

## **7\. Controles**

El juego utiliza Enhanced Input para movimiento, cámara, salto, disparo, recarga e interacción.

| Acción | Input |
| :---: | :---: |
| Movimiento | **WASD** |
| Salto | **Barra Espaciadora** |
| Disparo | **Click Izquierdo** |
| Recarga | **R** |
| Interacción | **E** |

## 

## **8\. Pruebas realizadas**

Durante el desarrollo se realizaron pruebas de funcionamiento y replicación.

| Prueba | Resultado |
| ----- | ----- |
| Multiplayer de cuatro jugadores en PIE | Satisfactorio |
| Distribución automática de equipos | Satisfactorio |
| Disparos, daño y recarga | Satisfactorio |
| Muerte y respawn | Satisfactorio |
| Captura y devolución de banderas | Satisfactorio |
| Sincronización de puntuaciones y tiempo | Satisfactorio |
| Victoria y finalización de partida | Satisfactorio |
| Sesión online de Steam con dos jugadores | Partida completa realizada |
| Regreso al menú desde partida online | Satisfactorio |

## **9\. Limitaciones conocidas**

El escenario Battle Creek contiene algunos materiales que no conservan su apariencia original tras la importación y el empaquetado.

Este problema es visual y no impidió completar las pruebas de gameplay multijugador realizadas.

La integración con Steam utiliza un identificador de aplicación de pruebas y no constituye una publicación oficial en Steam.

## **10\. Créditos**

**Desarrollo:** Capogrossi Agustín.

**Motor:** Unreal Engine 5.6.1 — Epic Games.

**Plugin de sesiones:** MultiplayerSessionOW 

**Escenario Battle Creek:** SketchFab

Autor: [**NobleKnightWolf**](https://sketchfab.com/solarwolfhd)

Recurso: [**BattleCreek**](https://sketchfab.com/3d-models/battlecreek-halo-ce-d3b9b2dcc5b54b0a8e6935a08cd33ddb)

Licencia: **CC** **Attribution** 

**Otros recursos gráficos y sonoros:** FreeSound.

[**Muerte**](https://freesound.org/people/MrFossy/sounds/547209/)

[**Hitmark**](https://freesound.org/people/aruscio28/sounds/322640/)

[**Captura**](https://freesound.org/people/GabrielAraujo/sounds/242501/)

[**Recarga**](https://freesound.org/people/HunterOfTheMist/sounds/846695/)

[**Disparo**](https://freesound.org/people/LeMudCrab/sounds/163456/)

---


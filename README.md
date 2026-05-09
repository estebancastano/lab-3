# Laboratorio # 3: Gestión de Memoria

## Nombres, correos y números de documento

* Esteban Andres Castaño Gallo, **esteban.castano1@udea.edu.co**, 1001967876
* Jorge Luis Rodriguez, **jorge.rodriguezj@udea.edu.co**, 1027941053

---

## Documentación del código

### Punto 1 — `mem_map.c`

Programa que muestra las direcciones virtuales de las distintas regiones de memoria de un proceso.

- **`int global_var`**: variable global inicializada, se almacena en el segmento de datos (.data) del ejecutable.
- **`int main()`**: función principal. Declara una variable local (almacenada en el stack), reserva memoria dinámica con `malloc` (almacenada en el heap) e imprime las direcciones de cada elemento junto con el PID del proceso.
- **`getchar()`**: pausa la ejecución del programa para permitir consultar `/proc/[pid]/maps` mientras el proceso sigue vivo.
- **`free(heap_var)`**: libera la memoria reservada en el heap antes de terminar.

### Punto 2 — `heap_demo.c`

Programa que demuestra el uso correcto de la API de memoria dinámica.

- **`malloc(n * sizeof(int))`**: reserva memoria para 10 enteros en el heap.
- **`realloc(arr, 20 * sizeof(int))`**: redimensiona el bloque previamente asignado para que pueda contener 20 enteros, conservando los valores originales.
- **`free(arr)`**: libera toda la memoria reservada antes de terminar el programa.

### Punto 2 — `buggy_mem.c`

Programa que contiene tres errores clásicos de memoria de manera intencional, para ser detectados con Valgrind.

- **Error 1 (buffer overflow)**: el bucle escribe en `p[5]` cuando solo hay 5 posiciones válidas (`p[0]` a `p[4]`).
- **Error 2 (memory leak)**: se reserva memoria con `malloc(100)` para `q` pero nunca se libera con `free`.
- **Error 3 (use-after-free)**: se accede a `p[0]` después de haber liberado el puntero con `free(p)`.

### Punto 2 — `buggy_mem_fixed.c`

Versión corregida del programa anterior con las tres correcciones aplicadas:

- Cambio de `i <= 5` a `i < 5` para evitar el buffer overflow.
- Adición de `free(q)` después de imprimir la cadena.
- Reordenamiento del `printf` para que se ejecute antes de `free(p)`.

### Punto 3 — `base_bounds.c`

Simulador del mecanismo de traducción de direcciones base & bounds.

- **`typedef struct { int base; int bounds; } Registro`**: estructura que representa los dos registros de hardware de un proceso: la dirección física base y el tamaño máximo permitido.
- **`int traducir(Registro r, int va)`**: aplica la fórmula `PA = VA + base` si la dirección virtual está dentro del rango válido (`0 <= VA < bounds`). Si no, imprime una excepción y retorna -1, simulando el comportamiento del hardware ante una violación de límites.
- **`int main()`**: define tres procesos (A, B y C) con distintos pares base/bounds y traduce un conjunto de direcciones virtuales de prueba para cada uno, mostrando los casos de éxito y las excepciones.

### Punto 5.2 — `paging_sim.c`

Simulador del mecanismo de traducción de direcciones mediante paginación de un solo nivel.

- `#define PAGE_BITS 4` y `PAGE_SIZE`: definen el tamaño de la página (16 bytes), estableciendo que los 4 bits bajos de la VA son el offset y los 4 bits altos el VPN.

- `int page_table[NUM_PAGES]`: arreglo que representa la tabla de páginas en memoria; el índice es el VPN y el contenido es el PFN (marco físico). El valor `-1` simula una página no presente o Page Fault.

- `void traducir(int va)`: realiza la descomposición de la dirección virtual mediante operaciones de bits (`>>` para el VPN y `&` para el offset).

- **Lógica de traducción**: si la entrada en la tabla es válida, calcula la dirección física (PA) desplazando el PFN y combinándolo con el offset ((`pfn << PAGE_BITS) | offset`); de lo contrario, notifica el fallo de página.

- `int main()`: ejecuta una serie de pruebas con direcciones hexadecimales para demostrar casos de traducción exitosa y accesos que generan excepciones por no estar mapeados en la tabla.

### Punto 6.2 — `fragmentation.c`

Programa de experimentación que demuestra el impacto de la fragmentación externa en el heap.

- `void * ptrs[N]` y `int sizes[]`: definen un arreglo de punteros y una lista de tamaños variados (desde 16B hasta 1KB) para simular diversas solicitudes de memoria de un proceso.

- **Ciclo de asignación (`malloc`)**: solicita bloques de memoria al sistema e imprime sus direcciones hexadecimales para observar la ubicación de los datos y el espacio adicional utilizado por los metadatos (headers) de `glibc`.

- **Estrategia de liberación selectiva (`free`)**: libera únicamente los bloques en índices pares (0, 2, 4...), creando intencionalmente "huecos" libres separados por bloques que aún están ocupados.

- **Prueba de fragmentación**: intenta realizar una asignación grande (`malloc(1500)`) para comprobar si el gestor de memoria puede reutilizar los huecos dispersos o si se ve obligado a solicitar más memoria al kernel debido a la falta de contigüidad física (fragmentación externa).

- `main()`: gestiona el ciclo de vida completo de la prueba, asegurando la limpieza final de los bloques impares para evitar fugas de memoria.


### Punto 7 — `tlb_locality.c`

Benchmark de rendimiento que mide el impacto de la localidad de referencia en la eficiencia del TLB y la traducción de direcciones.

- `double ms(struct timespec a, struct timespec b)`: función auxiliar que calcula la diferencia de tiempo entre dos instantes con precisión de nanosegundos, convirtiendo el resultado a milisegundos.

- **Acceso Secuencial (Alta Localidad)**: recorre un arreglo de 16 MB de forma lineal, lo que permite que una sola entrada del TLB sirva para múltiples accesos consecutivos (muchos hits), minimizando el costo de traducción.

- **Algoritmo Fisher-Yates**: se utiliza para desordenar aleatoriamente un arreglo de índices (`idx`), garantizando que el segundo acceso a memoria no tenga un patrón predecible.

- **Acceso Aleatorio (Baja Localidad)**: recorre el mismo arreglo utilizando los índices desordenados, lo que provoca constantes fallos de caché en el TLB (*TLB misses*) al saltar entre páginas distintas, obligando al hardware a consultar la tabla de páginas en RAM repetidamente.

- `main()`: compara los tiempos de ejecución de ambos patrones de acceso, demostrando empíricamente cómo la falta de localidad espacial degrada el rendimiento del sistema debido al costo de la traducción de direcciones.

---

## Problemas presentados durante el desarrollo de la práctica y sus soluciones

### Problema 1: Configuración del entorno Linux en Windows

Al inicio de la práctica no contábamos con un entorno Linux nativo, ya que trabajábamos en Windows.

**Solución:** Se instaló WSL (Windows Subsystem for Linux) con Ubuntu, lo que permitió ejecutar todos los comandos del laboratorio sin necesidad de una máquina virtual o un sistema dual. La instalación se hizo con el comando `wsl --install` desde PowerShell.

### Problema 2: El proceso terminaba antes de poder leer su mapa de memoria

En el Punto 1, al ejecutar `mem_map` y luego intentar leer `/proc/[pid]/maps` en otra terminal, en los primeros intentos el programa ya había terminado y `pgrep` no encontraba el proceso.

**Solución:** Se utilizó `getchar()` dentro del programa para pausarlo justo después de imprimir las direcciones. Mientras el programa esperaba el ENTER, se pudo abrir una segunda terminal y consultar `/proc/[pid]/maps` y `pmap` con el proceso aún vivo.

### Problema 3: Comparar dos procesos al mismo tiempo

Para la actividad 1.4 era necesario ejecutar dos instancias de `mem_map` simultáneamente y obtener el mapa de cada una.

**Solución:** Se abrieron tres terminales en VS Code: dos para mantener cada instancia esperando el ENTER, y una tercera para ejecutar `cat /proc/[pid]/maps` con los PIDs distintos de cada proceso, guardando las salidas en archivos separados (`procA_maps.txt` y `procB_maps.txt`).

### Problema 4: Advertencia del compilador en `buggy_mem.c`

Al compilar `buggy_mem.c` con `gcc -Wall`, el compilador detectó automáticamente uno de los errores intencionales (use-after-free) y mostró un warning, lo que inicialmente generó dudas sobre si el programa se compilaba correctamente.

**Solución:** Se confirmó que se trataba únicamente de una advertencia y no de un error: el ejecutable se generaba sin problemas. La advertencia incluso fue útil porque demostró que el compilador moderno también puede detectar ciertos errores de memoria de forma estática, complementando a Valgrind.

### Problema 5: Lectura inicial de la salida de Valgrind

La salida de Valgrind para `buggy_mem.c` era extensa y al principio resultaba difícil identificar a qué error correspondía cada mensaje.

**Solución:** Se compiló el programa con la opción `-g` para que Valgrind incluyera los números de línea del código fuente en sus reportes. Esto permitió asociar cada mensaje con la línea exacta del bug, facilitando la identificación de los tres errores clásicos.

### Problema 6: Optimización agresiva del compilador

Al ejecutar el código de `tlb_locality.c` , los tiempos de ejecución eran de **0.00 ms** o el programa terminaba instantáneamente sin realizar los cálculos. El compilador (gcc) detectó que los resultados de las sumas o las asignaciones no se utilizaban después del bucle, por lo que eliminó el código muerto para optimizar el rendimiento.

**Solución**: Se compiló utilizando la bandera `-O0` para desactivar todas las optimizaciones, asegurando que el hardware realmente recorra la memoria y se puedan medir los fallos del TLB.

### Problema 7: Variabilidad en las mediciones de tiempo
En el experimento de tlb_locality.c, los tiempos de ejecución variaban significativamente entre una prueba y otra, lo que dificultaba obtener un "factor de lentitud" constante. El sistema operativo estaba ejecutando otros procesos en segundo plano (navegador, actualizaciones, etc.), lo que generaba ruido en las mediciones y expulsaba datos del TLB/Caché de forma errática.

**Solución**: Se realizaron múltiples ejecuciones (más de las 3 solicitadas) y se cerraron aplicaciones innecesarias. Se utilizó el promedio de las pruebas más estables para obtener un resultado representativo del comportamiento del hardware.

---

## Pruebas realizadas a los programas que verificaron su funcionalidad
### 4. Segmentación:
#### 4.1 Traducción manual con tabla de segmentos:
#### - 4.1.1. cálculo paso a paso para cada VA:

| VA (hex) | Selector | Offset (hex) | Segmento | Cálculo                     | PA o Excepción |
|----------|----------|--------------|----------|-----------------------------|----------------|
| 0x03A0   | 00       | 0x3A0        | Code     | 0x4000 + 0x3A0              | 0x43A0         | 
| 0x1800   | 01       | 0x800        | Heap     | 0x6000 + 0x800              | 0x6800         |
| 0x3C00   | 11       | 0xC00        | Stack    | Ver paso a paso en 4.1.2    | 0x2400         |
| 0x0C00   | 00       | 0xC00        | Code     |Offset (3072) > Tamaño (2048)| Segment Fault  |
| 0x2200   | 10       |  —           | ???      |Selector no definido en tabla| Segment Fault  |


#### - 4.1.2. El Stack y el crecimiento negativo
**¿Por qué crece en dirección negativa?**

Se diseña así para permitir que el Heap y el Stack crezcan uno hacia el otro en el espacio de direcciones virtuales sin colisionar prematuramente, optimizando el uso del espacio lógico.
#### Ajuste especial para el PA:
Dado que el stack crece hacia abajo, el offset de la VA debe convertirse en un **offset negativo.**

La fórmula es:

**1. Offset Negativo** = $Offset - Tamaño Maximo del Segmento$

**2. PA** = $Base + Offset Negativo$

**Cálculo para 0x3C00:**

- **Offset:** 0xC00 (3072 en decimal).

- **Tamaño Máximo:** $2^{12} = 4096$ bytes.

- **Offset Negativo:** $3072 - 4096 = -1024$ bytes ($0xFFFFFC00$ en hex de 32 bits).

- **PA:** $0x2800 (10240) - 1024 = 9216$. En hex: **0x2400**.

#### -4.1.3. Ventaja frente a Base & Bounds:
La segmentación es mucho más eficiente porque evita el desperdicio de memoria física en los "huecos" del espacio de direcciones.
- En **Base & Bounds**, se debe asignar un bloque contiguo que incluya el espacio vacío entre el stack y el heap.
- En **Segmentación**, solo se mapean a la memoria física los segmentos que realmente contienen datos (Code, Heap, Stack), permitiendo que los huecos lógicos no ocupen RAM real.

#### -4.1.4. Fragmentación Externa
La fragmentación externa ocurre cuando la memoria física se llena de pequeños huecos libres dispersos entre segmentos asignados. El problema es que, aunque la suma total de memoria libre sea suficiente para un nuevo segmento, no hay un bloque contiguo lo suficientemente grande para albergarlo.Surge en la segmentación porque los segmentos tienen tamaños variables y se asignan/liberan en momentos distintos, dejando la memoria física "moteada".

Diagrama conceptual:
```plaintext
Memoria Física
+-------------------+
|   Segmento A      |
+-------------------+
|  Hueco (LIBRE)    | <--- Demasiado pequeño para el Segmento D
+-------------------+
|   Segmento B      |
+-------------------+
|  Hueco (LIBRE)    | <--- Demasiado pequeño para el Segmento D
+-------------------+
|   Segmento C      |
+-------------------+
```

---
### 5. Paginación:
#### 5.1 Actividad: Cálculo de la tabla de páginas:
#### - 5.1.1. Cálculo de bits para VPN y Offset:

Para descomponer una dirección virtual (VA) de 32 bits con págínas de 4 KB:

- **Offset:** El tamaño de página es de $4\text{ KB} = 2^{12}$ bytes. Por lo tanto, se necesitan 12 bits para el offset para poder direccionar cada byte dentro de la página.
- **VPN (Virtual Page Number)**: Los bits restantes de la dirección virtual corresponden al VPN.
  
    - $\text{VPN bits} = \text{Total bits VA} - \text{bits Offset}$
    - $\text{VPN bits} = 32 - 12 = \mathbf{20 \text{ bits}}$.
      
#### - 5.1.2. Cantidad de entradas en la tabla de páginas:
El número de entradas en la tabla de páginas (PTEs) es igual al número total de páginas virtuales posibles que el proceso puede direccionar.

- Como el VPN tiene 20 bits, hay $2^{20}$ combinaciones posibles.
- $\text{Entradas} = 2^{20} = \mathbf{1.048.576 \text{ entradas}}$.

#### - 5.1.3. Tamaño de la tabla de páginas:
Para calcular el espacio total en memoria que ocupa la tabla de un solo proceso, multiplicamos el número de entradas por el tamaño de cada PTE de 4 bytes :

- $\text{Tamaño} = \text{Número de entradas} \times \text{Tamaño de PTE}$
- $\text{Tamaño} = 1.048.576 \times 4 \text{ bytes} = 4.194.304 \text{ bytes} = \mathbf{4 \text{ MB}}$.

#### ¿Es razonable?
En sistemas modernos con gigabytes de RAM, 4 MB parece poco. Sin embargo, si tienes 100 procesos activos, solo las tablas de páginas ocuparían 400 MB de memoria física. Para procesos pequeños que solo usan unos pocos KB de memoria, tener una tabla de 4 MB es ineficiente y poco razonable, lo que motiva el uso de estructuras como tablas de páginas multinivel.

#### - 5.1.4. Bits del PFN y Bits de Control:
Dentro de una PTE de 4 bytes (32 bits), la información se divide entre la dirección física y metadatos:

- **Bits para el PFN**: Según la tabla dada, el espacio físico es de 20 bits y el tamaño de página/marco es de $2^{12}$ bytes.
       - $\text{PFN bits} = \text{Total bits físicos} - \text{bits Offset}$
       - $\text{PFN bits} = 20 - 12 = \mathbf{8 \text{ bits}}$. (Identifica uno de los 256 marcos físicos disponibles).
- **Bits restantes**: Si la PTE es de 32 bits y el PFN usa 8, quedan **24 bits** para control.
- **3 Bits de control comunes y su función**:
    - **Present Bit (P)**: Indica si la págína está actualmente en la memoria física o si ha sido movida al disco (swap).
    - **Read/Write Bit (R/W)**: Determina si el proceso tiene permiso solo para leer la página o también para escribir en ella.
    - **Dirty Bit (D)**: Se activa si la página ha sido modificada desde que se cargó en memoria; es crucial para saber si se debe escribir de vuelta al disco al desalojarla.
      
---
#### - 5.2.1. Actividad: Simulador de paginación:
Está en la carpeta de codes

---
#### - 5.3.1. Salida completa del simulador
Al ejecutar el código, la salida generada es la siguiente:

<img width="1174" height="880" alt="image" src="https://github.com/user-attachments/assets/318efe8a-39a3-417e-8b34-b9958e9db879" />

#### - 5.3.2. Análisis de VAs 0x10 y 0xA3 y el rol del SO
- **0x10**: El VPN es 1. Al consultar la tabla, `page_table[1]` es -1, lo que genera un Page Fault.
- **0xA3**: El VPN es 10 ($0xA$ en hexadecimal). El valor en `page_table[10]` es 4, por lo que se traduce exitosamente a la dirección física 0x43.

**¿Qué hace el SO ante un Page Fault?** 

En un sistema real, el hardware genera una excepción (trap) hacia el Sistema Operativo. El manejador de fallos de página del SO debe:

1. Verificar si la dirección es válida. Si no lo es, termina el proceso (Segfault).
2. Si es válida pero no está en RAM, busca la página en el almacenamiento secundario (disco/swap).
3. Busca un marco físico libre, carga la página y actualiza la PTE (Page Table Entry) con el nuevo PFN y el bit de presencia en 1.
4. Reinicia la instrucción que causó el fallo.

#### - 5.3.3. Accesos a memoria y costo

Para completar una instrucción simple como un `load`, se requieren 2 accesos a memoria física:
1. Primer acceso: Consultar la tabla de páginas en memoria para obtener el PFN (traducción).
2. Segundo acceso: Acceder al dato real en la dirección física calculada.

   **¿Por qué es costoso?**
   
   La memoria RAM es significativamente más lenta que el procesador. Duplicar los accesos reduce el rendimiento del sistema a la mitad.
   **Solución de hardware**: El TLB (Translation Lookaside Buffer). Es una memoria caché de alta velocidad dentro de la CPU que almacena las traducciones VPN $\rightarrow$ PFN recientes, permitiendo traducciones casi instantáneas sin ir a la RAM.

#### - 5.3.4. Paginación vs. Segmentación: Fragmentación
La gran ventaja de la paginación es que elimina la fragmentación externa.

- En la segmentación, los segmentos tienen tamaños variables, lo que deja huecos irregulares en la memoria física que pueden ser difíciles de reutilizar.

- En la paginación, tanto el espacio virtual como el físico se dividen en trozos de tamaño fijo (páginas y marcos). Cualquier página cabe en cualquier marco disponible, por lo que nunca quedan huecos inutilizables entre asignaciones.

---
### 6. Gestión de espacio libre:
#### 6.1 Actividad: Simulación de estrategias de asignación:
#### - 6.1.1. Simulación First Fit:

*Estrategia: Recorrer la lista y elegir el primer bloque donde quepa la solicitud.*

- **malloc(212)**: No cabe en 100. Cabe en 500 (Dir: 0x0200). Quedan 288 bytes libres en esa posición.

- **malloc(417)**: No cabe en 100, ni en 288, ni en 200, ni en 300. Cabe en 600 (Dir: 0x0700). Quedan 183 bytes libres.

- **malloc(98)**: Cabe en el primer bloque de 100 (Dir: 0x0100). Quedan 2 bytes libres.

- **malloc(426)**: Falla (NULL). No queda ningún bloque contiguo de al menos 426 bytes.

Lista libre final (First Fit):

`{2 (0x0162), 288 (0x02D4), 200 (0x0400), 300 (0x0500), 183 (0x08A1)}`


#### - 6.1.2. Simulación Best Fit:
Estrategia: Recorrer toda la lista y elegir el bloque que deje el menor remanente posible.

- **malloc(212)**: El más ajustado es 300 (Dir: 0x0500). Quedan 88 bytes.

- **malloc(417)**: El más ajustado es 500 (Dir: 0x0200). Quedan 83 bytes.

- **malloc(98)**: El más ajustado es 100 (Dir: 0x0100). Quedan 2 bytes.

- **malloc(426)**: Cabe en 600 (Dir: 0x0700). Quedan 174 bytes.

Resultado: ¡Con Best Fit todas las solicitudes tuvieron éxito!

#### - 6.1.3. Comparación de Estrategias:
- **Más fragmentación externa**: En este caso, First Fit. Al ser "descuidado" y usar bloques grandes para solicitudes pequeñas al principio de la lista, agota rápido las opciones para solicitudes grandes posteriores.

- **Minimiza fragmentación**: Best Fit. Al preservar los bloques grandes y usar los que mejor se ajustan, mantiene la capacidad de responder a solicitudes de mayor tamaño.

#### - 6.1.4. Coalescing (Coalescencia):
El **coalescing** es el proceso de unir dos bloques libres adyacentes en la memoria física para formar un único bloque más grande. Sin esto, la memoria se fragmenta en pedazos pequeños inútiles.

Caso de falla sin coalescing:
Supongamos que liberas dos bloques contiguos de 150 bytes cada uno.

- **Sin coalescing**: Tienes dos bloques de 150B. Si pides `malloc(250)`, el sistema dirá que no hay espacio, porque no hay ningún bloque contiguo de ese tamaño.

- **Con coalescing**: El sistema nota que están juntos, los une en un bloque de 300B y la solicitud de 250B tiene éxito.

#### - 6.1.5. Fragmentación Interna y Slab Allocator:
La fragmentación interna ocurre cuando se asigna un bloque de memoria que es ligeramente más grande de lo que el proceso pidió. El espacio sobrante dentro de ese bloque asignado se desperdicia porque el gestor no puede dárselo a nadie más.

Slab Allocator:
Es un gestor que crea "cachés" de objetos de tamaño fijo (por ejemplo, una caché solo para estructuras de procesos).

- **Cuándo aparece**: Aparece cuando el tamaño del objeto pedido no encaja perfectamente en las ranuras predefinidas del slab. Si el slab reserva espacios de 32 bytes y tú pides 20 bytes, los 12 bytes restantes son fragmentación interna dentro de ese "slot".

---

#### - 6.2.1. Actividad: Fragmentación
Está en la carpeta codes
<img width="657" height="427" alt="image" src="https://github.com/user-attachments/assets/df98991e-663d-4b5f-8299-d3f33ea5a042" />

---

#### 6.3 Actividad: Fragmentación en glibc — Análisis
#### - 6.3.1. Análisis de Direcciones y Metadatos
Al observar la salida del programa `fragmentation.c`, se determinó lo siguiente:

- **No contigüidad**: Las direcciones de memoria asignadas por `malloc()` no son consecutivas. Existe un patrón de separación (gap) entre el final de un bloque y el inicio del siguiente que es mayor al tamaño solicitado en el arreglo `sizes[]`.

- **Presencia de Headers**: Este espacio extra entre bloques confirma que el asignador de `glibc` inserta metadatos o cabeceras (headers) justo antes de la dirección que entrega al usuario. Estos headers son esenciales para que la función `free()` conozca el tamaño del bloque a liberar.

- **Alineación**: Además, se observa que las direcciones terminan frecuentemente en múltiplos de 8 o 16, lo que evidencia que el sistema aplica alineación de memoria para optimizar el rendimiento del procesador.

---

#### - 6.3.2. Resultado del Experimento de Fragmentación
Tras liberar los bloques en los índices pares (0, 2, 4, 6, 8) e intentar asignar un bloque grande de 1,500 bytes:

- **Estado del Heap**: Aunque se liberó un total de 1,616 bytes (suma de los bloques pares), el espacio quedó "agujereado" por los bloques de los índices impares que permanecieron ocupados.

- **Fragmentación Externa**: El experimento demuestra la existencia de fragmentación externa: hay suficiente memoria libre total, pero no existe un único bloque contiguo lo suficientemente grande para satisfacer la solicitud de 1,500 bytes.

- **Comportamiento de glibc**: Si el `malloc(1500)` tuvo éxito, no fue porque reutilizara los huecos pequeños, sino porque el asignador solicitó al Kernel expandir el heap para obtener un nuevo bloque de memoria limpia al final.

#### - 6.3.3. Niveles de Gestión: Usuario vs. Kernel
Es fundamental distinguir por qué existen dos niveles de asignación de memoria:

- **Asignador de Usuario (malloc/glibc)**: Trabaja a nivel de bytes. Su objetivo es la micro-gestión de solicitudes pequeñas y variadas de la aplicación, minimizando el desperdicio y maximizando la velocidad.

- **Asignador del Kernel (Buddy System/Slab)**: Trabaja a nivel de páginas (usualmente 4 KB). No gestiona bytes individuales, sino grandes bloques de RAM física que reparte entre los diferentes procesos.

- **Justificación de los dos niveles**:

  - **Rendimiento**: Las llamadas al sistema (syscalls) son lentas. malloc pide memoria "al por mayor" al Kernel y la reparte "al detal" internamente, evitando entrar al Kernel por cada pequeña variable.

  - **Aislamiento**: El Kernel se encarga de proteger la memoria entre procesos, mientras que la librería de usuario gestiona la estructura interna del heap del proceso.

---
### 7 TLBs — Translation Lookaside Buffer:
<img width="659" height="233" alt="image" src="https://github.com/user-attachments/assets/855f9957-fc4c-4528-a4dc-a877d1766a92" />

#### - 7.1 Actividad: Localidad y TLB — Análisis
#### - 7.1.1. Comparativa de rendimiento
Basado en las tres ejecuciones de tlb_locality, los tiempos son los siguientes:

| Ejecución   | Secuencial (ms) | Aleatorio (ms) | Factor de lentitud (Aleatorio / Secuencial) |
|-------------|-----------------|----------------|---------------------------------------------|
| 1           | 7.73 ms         | 40.94 ms       | ~5.29x                                      |
| 2           | 11.06 ms        | 38.97 ms       | ~3.52x                                      |
| 3           | 9.21 ms         | 41.92 ms       | ~4.55x                                      |
|**Promedio** | **9.33 ms**     | **40.61 ms**   | **4.35x veces más lento**                   |

El acceso aleatorio es, en promedio, **4.35 veces más lento** que el secuencial.

#### - 7.1.2. Explicación mediante el modelo del TLB
El TLB es una caché de hardware que almacena traducciones recientes de VPN a PFN para acelerar el acceso a memoria.

- **Acceso Secuencial**: Presenta una alta localidad espacial. Al acceder a un elemento, la traducción de la página se guarda en el TLB; como el siguiente elemento está en la misma página, ocurre un TLB hit. El hit rate es muy cercano al 100%, ya que solo hay un fallo (miss) cuando se cambia de página (cada 4 KB).

- **Acceso Aleatorio**: Presenta una baja localidad. Cada acceso apunta probablemente a una página distinta que no está en la caché. Esto provoca constantes TLB misses, obligando al hardware a consultar la tabla de páginas en la memoria principal (mucho más lenta) para cada traducción. El hit rate cae drásticamente.

#### - 7.1.3. Impacto de páginas de 64 KB frente a 4 KB
Desde el punto de vista del TLB: La situación mejoraría para el acceso aleatorio. Una página de 64 KB es 16 veces más grande que una de 4 KB, lo que significa que cada entrada del TLB "cubre" mucha más memoria física. Esto aumenta la probabilidad de que una dirección aleatoria caiga dentro de una página ya traducida en el TLB (mayor cobertura).

Desde el punto de vista del uso de memoria: La situación empeoraría debido a la fragmentación interna. Si un proceso solo necesita pequeños bloques de datos pero el sistema asigna páginas de 64 KB, se desperdicia mucha memoria RAM física que no puede ser utilizada por otros procesos.

#### - 7.1.4. No teniamos instalado `perf`

---

#### - 7.2 Actividad: Comportamiento de los TLB:
#### - 7.2.1:Cobertura del TLB y procesos modernos

- **Cálculo de cobertura**: Para determinar cuánta memoria puede "cubrir" (mapear) el TLB sin generar fallos, multiplicamos el número de entradas por el tamaño de la página: $64 \text{ entradas} \times 4 \text{ KB/página} = \mathbf{256 \text{ KB}}$.

- **¿Es suficiente?**: Para un proceso moderno típico, **no es suficiente**. Las aplicaciones actuales (navegadores, bases de datos, entornos de ejecución como Java o Python) manejan conjuntos de datos de megabytes o gigabytes. Con una cobertura de solo 256 KB, el proceso sufriría constantes **TLB misses**, lo que degradaría significativamente el rendimiento al tener que consultar la tabla de páginas en RAM con frecuencia.

#### - 7.2.2. TLB Shootdown en sistemas multiprocesador

- **Definición**: Un **TLB shootdown** es el mecanismo utilizado en sistemas con múltiples núcleos (multiprocesadores) para mantener la coherencia de las traducciones de memoria entre los diferentes TLB locales de cada núcleo.

- **Situación en la que ocurre**: Ocurre cuando el Sistema Operativo modifica una entrada en la tabla de páginas que es compartida por varios núcleos (por ejemplo, al liberar memoria o cambiar permisos). El núcleo que realiza el cambio debe "notificar" a los demás núcleos que sus copias locales de esa traducción en sus propios TLB ya no son válidas y deben ser eliminadas.

- **Por qué es costoso**: Es una operación de alto costo porque requiere enviar una **interrupción entre procesadores (IPI)**. Esto obliga a los otros núcleos a detener su ejecución actual, limpiar su TLB (o entradas específicas) y esperar una sincronización global antes de continuar, lo que genera una gran latencia en el sistema.

  #### - 7.2.3. Gestión por Hardware (CISC) vs. Software (RISC)

- **Hardware (CISC/x86)**: El hardware conoce la estructura exacta de la tabla de páginas. Ante un *miss*, el procesador "camina" por la tabla automáticamente (hardware page-table walk) para encontrar la traducción y cargarla en el TLB sin intervención del SO.

- **Software (RISC/MIPS)**: Ante un miss, el hardware simplemente genera una excepción (trap) y le entrega el control al Sistema Operativo. El manejador de excepciones del SO busca la traducción y utiliza instrucciones especiales para cargarla manualmente en el TLB.
- **Flexibilidad**: La gestión por software ofrece mayor flexibilidad al diseñador del SO. Esto se debe a que el SO puede implementar cualquier estructura de tabla de páginas (multinivel, invertida, hash, etc.) sin estar limitado a lo que el hardware espera, facilitando la innovación y el control total sobre la gestión de memoria.

---

## Enlace al vídeo

https://youtu.be/qbPEB0TFVw4


---

## Manifiesto de transparencia
El uso de la IA se limitó a las siguientes áreas:

- **Documentación de Código**: Apoyo en la redacción técnica y estructuración de los resúmenes de los programas paging_sim.c, fragmentation.c y tlb_locality.c.

- **Análisis Conceptual**: Clarificación de conceptos complejos de bajo nivel, como los mecanismos de TLB shootdown y las diferencias entre allocators de usuario y kernel.

- **Estructuración del Informe**: Formateo de datos experimentales y organización de hallazgos para mejorar la legibilidad.


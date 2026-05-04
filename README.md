# Laboratorio # 3: Gestión de Memoria

## Nombres, correos y números de documento

* Esteban Andres Castaño Gallo, **esteban.castano1@udea.edu.co**, 1001967876
* Jorge Luis Rodriguez, **jorge.rodriguezj@udea.edu.co**, 1027941053

---

## Documentación del código


---

## Problemas presentados durante el desarrollo de la práctica y sus soluciones


---

## Pruebas realizadas a los programas que verificaron su funcionalidad
### 4. Segmentación:
#### 4.1 Traducción manual con tabla de segmentos:
#### - 4.1.1 cálculo paso a paso para cada VA:

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
#### - 5.1.1 Cálculo de bits para VPN y Offset:

Para descomponer una dirección virtual (VA) de 32 bits con págínas de 4 KB:

- **Offset:** El tamaño de página es de $4\text{ KB} = 2^{12}$ bytes. Por lo tanto, se necesitan 12 bits para el offset para poder direccionar cada byte dentro de la página.
- **VPN (Virtual Page Number)**: Los bits restantes de la dirección virtual corresponden al VPN.
  
    - $\text{VPN bits} = \text{Total bits VA} - \text{bits Offset}$
    - $\text{VPN bits} = 32 - 12 = \mathbf{20 \text{ bits}}$.
      
#### - 5.1.2 Cantidad de entradas en la tabla de páginas:
El número de entradas en la tabla de páginas (PTEs) es igual al número total de páginas virtuales posibles que el proceso puede direccionar.

- Como el VPN tiene 20 bits, hay $2^{20}$ combinaciones posibles.
- $\text{Entradas} = 2^{20} = \mathbf{1.048.576 \text{ entradas}}$.

#### - 5.1.3 Tamaño de la tabla de páginas:
Para calcular el espacio total en memoria que ocupa la tabla de un solo proceso, multiplicamos el número de entradas por el tamaño de cada PTE de 4 bytes :

- $\text{Tamaño} = \text{Número de entradas} \times \text{Tamaño de PTE}$
- $\text{Tamaño} = 1.048.576 \times 4 \text{ bytes} = 4.194.304 \text{ bytes} = \mathbf{4 \text{ MB}}$.

#### ¿Es razonable?
En sistemas modernos con gigabytes de RAM, 4 MB parece poco. Sin embargo, si tienes 100 procesos activos, solo las tablas de páginas ocuparían 400 MB de memoria física. Para procesos pequeños que solo usan unos pocos KB de memoria, tener una tabla de 4 MB es ineficiente y poco razonable, lo que motiva el uso de estructuras como tablas de páginas multinivel.

#### - 5.1.4 Bits del PFN y Bits de Control:
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
#### - 5.2.1 Actividad: Simulador de paginación:
Está en la carpeta de codes

---
#### - 5.3.1 Salida completa del simulador
Al ejecutar el código, la salida generada es la siguiente:

<img width="1174" height="880" alt="image" src="https://github.com/user-attachments/assets/318efe8a-39a3-417e-8b34-b9958e9db879" />

#### - 5.3.2 Análisis de VAs 0x10 y 0xA3 y el rol del SO
- **0x10**: El VPN es 1. Al consultar la tabla, `page_table[1]` es -1, lo que genera un Page Fault.
- **0xA3**: El VPN es 10 ($0xA$ en hexadecimal). El valor en `page_table[10]` es 4, por lo que se traduce exitosamente a la dirección física 0x43.

**¿Qué hace el SO ante un Page Fault?** 

En un sistema real, el hardware genera una excepción (trap) hacia el Sistema Operativo. El manejador de fallos de página del SO debe:

1. Verificar si la dirección es válida. Si no lo es, termina el proceso (Segfault).
2. Si es válida pero no está en RAM, busca la página en el almacenamiento secundario (disco/swap).
3. Busca un marco físico libre, carga la página y actualiza la PTE (Page Table Entry) con el nuevo PFN y el bit de presencia en 1.
4. Reinicia la instrucción que causó el fallo.

#### - 5.3.3 Accesos a memoria y costo

Para completar una instrucción simple como un `load`, se requieren 2 accesos a memoria física:
1. Primer acceso: Consultar la tabla de páginas en memoria para obtener el PFN (traducción).
2. Segundo acceso: Acceder al dato real en la dirección física calculada.

   **¿Por qué es costoso?**
   
   La memoria RAM es significativamente más lenta que el procesador. Duplicar los accesos reduce el rendimiento del sistema a la mitad.
   **Solución de hardware**: El TLB (Translation Lookaside Buffer). Es una memoria caché de alta velocidad dentro de la CPU que almacena las traducciones VPN $\rightarrow$ PFN recientes, permitiendo traducciones casi instantáneas sin ir a la RAM.

#### - 5.3.4 Paginación vs. Segmentación: Fragmentación
La gran ventaja de la paginación es que elimina la fragmentación externa.

- En la segmentación, los segmentos tienen tamaños variables, lo que deja huecos irregulares en la memoria física que pueden ser difíciles de reutilizar.

- En la paginación, tanto el espacio virtual como el físico se dividen en trozos de tamaño fijo (páginas y marcos). Cualquier página cabe en cualquier marco disponible, por lo que nunca quedan huecos inutilizables entre asignaciones.

---
### 6. Gestión de espacio libre:
#### 6.1 Actividad: Simulación de estrategias de asignación:
#### - 6.1.1 Simulación First Fit:

*Estrategia: Recorrer la lista y elegir el primer bloque donde quepa la solicitud.*

- **malloc(212)**: No cabe en 100. Cabe en 500 (Dir: 0x0200). Quedan 288 bytes libres en esa posición.

- **malloc(417)**: No cabe en 100, ni en 288, ni en 200, ni en 300. Cabe en 600 (Dir: 0x0700). Quedan 183 bytes libres.

- **malloc(98)**: Cabe en el primer bloque de 100 (Dir: 0x0100). Quedan 2 bytes libres.

- **malloc(426)**: Falla (NULL). No queda ningún bloque contiguo de al menos 426 bytes.

Lista libre final (First Fit):

`{2 (0x0162), 288 (0x02D4), 200 (0x0400), 300 (0x0500), 183 (0x08A1)}`


#### - 6.1.2 Simulación Best Fit:
Estrategia: Recorrer toda la lista y elegir el bloque que deje el menor remanente posible.

- **malloc(212)**: El más ajustado es 300 (Dir: 0x0500). Quedan 88 bytes.

- **malloc(417)**: El más ajustado es 500 (Dir: 0x0200). Quedan 83 bytes.

- **malloc(98)**: El más ajustado es 100 (Dir: 0x0100). Quedan 2 bytes.

- **malloc(426)**: Cabe en 600 (Dir: 0x0700). Quedan 174 bytes.

Resultado: ¡Con Best Fit todas las solicitudes tuvieron éxito!

#### - 6.1.3 Comparación de Estrategias:
- **Más fragmentación externa**: En este caso, First Fit. Al ser "descuidado" y usar bloques grandes para solicitudes pequeñas al principio de la lista, agota rápido las opciones para solicitudes grandes posteriores.

- **Minimiza fragmentación**: Best Fit. Al preservar los bloques grandes y usar los que mejor se ajustan, mantiene la capacidad de responder a solicitudes de mayor tamaño.

#### - 6.1.4 Coalescing (Coalescencia):
El **coalescing** es el proceso de unir dos bloques libres adyacentes en la memoria física para formar un único bloque más grande. Sin esto, la memoria se fragmenta en pedazos pequeños inútiles.

Caso de falla sin coalescing:
Supongamos que liberas dos bloques contiguos de 150 bytes cada uno.

- **Sin coalescing**: Tienes dos bloques de 150B. Si pides `malloc(250)`, el sistema dirá que no hay espacio, porque no hay ningún bloque contiguo de ese tamaño.

- **Con coalescing**: El sistema nota que están juntos, los une en un bloque de 300B y la solicitud de 250B tiene éxito.

#### - 6.1.5 Fragmentación Interna y Slab Allocator:
La fragmentación interna ocurre cuando se asigna un bloque de memoria que es ligeramente más grande de lo que el proceso pidió. El espacio sobrante dentro de ese bloque asignado se desperdicia porque el gestor no puede dárselo a nadie más.

Slab Allocator:
Es un gestor que crea "cachés" de objetos de tamaño fijo (por ejemplo, una caché solo para estructuras de procesos).

- **Cuándo aparece**: Aparece cuando el tamaño del objeto pedido no encaja perfectamente en las ranuras predefinidas del slab. Si el slab reserva espacios de 32 bytes y tú pides 20 bytes, los 12 bytes restantes son fragmentación interna dentro de ese "slot".

#### - 6.2.1 Actividad: Fragmentación
Está en la carpeta codes
<img width="657" height="427" alt="image" src="https://github.com/user-attachments/assets/df98991e-663d-4b5f-8299-d3f33ea5a042" />


#### 6.3 Actividad: Fragmentación en glibc — Análisis
#### - 6.3.1 Análisis de Direcciones y Metadatos
Al observar la salida del programa `fragmentation.c`, se determinó lo siguiente:

- **No contigüidad**: Las direcciones de memoria asignadas por `malloc()` no son consecutivas. Existe un patrón de separación (gap) entre el final de un bloque y el inicio del siguiente que es mayor al tamaño solicitado en el arreglo `sizes[]`.

- **Presencia de Headers**: Este espacio extra entre bloques confirma que el asignador de `glibc` inserta metadatos o cabeceras (headers) justo antes de la dirección que entrega al usuario. Estos headers son esenciales para que la función `free()` conozca el tamaño del bloque a liberar.

- **Alineación**: Además, se observa que las direcciones terminan frecuentemente en múltiplos de 8 o 16, lo que evidencia que el sistema aplica alineación de memoria para optimizar el rendimiento del procesador.

#### - 6.3.2 Resultado del Experimento de Fragmentación
Tras liberar los bloques en los índices pares (0, 2, 4, 6, 8) e intentar asignar un bloque grande de 1,500 bytes:

- **Estado del Heap**: Aunque se liberó un total de 1,616 bytes (suma de los bloques pares), el espacio quedó "agujereado" por los bloques de los índices impares que permanecieron ocupados.

- **Fragmentación Externa**: El experimento demuestra la existencia de fragmentación externa: hay suficiente memoria libre total, pero no existe un único bloque contiguo lo suficientemente grande para satisfacer la solicitud de 1,500 bytes.

- **Comportamiento de glibc**: Si el `malloc(1500)` tuvo éxito, no fue porque reutilizara los huecos pequeños, sino porque el asignador solicitó al Kernel expandir el heap para obtener un nuevo bloque de memoria limpia al final.

#### - 6.3.3 Niveles de Gestión: Usuario vs. Kernel
Es fundamental distinguir por qué existen dos niveles de asignación de memoria:

- **Asignador de Usuario (malloc/glibc)**: Trabaja a nivel de bytes. Su objetivo es la micro-gestión de solicitudes pequeñas y variadas de la aplicación, minimizando el desperdicio y maximizando la velocidad.

- **Asignador del Kernel (Buddy System/Slab)**: Trabaja a nivel de páginas (usualmente 4 KB). No gestiona bytes individuales, sino grandes bloques de RAM física que reparte entre los diferentes procesos.

- **Justificación de los dos niveles**:

  - **Rendimiento**: Las llamadas al sistema (syscalls) son lentas. malloc pide memoria "al por mayor" al Kernel y la reparte "al detal" internamente, evitando entrar al Kernel por cada pequeña variable.

  - **Aislamiento**: El Kernel se encarga de proteger la memoria entre procesos, mientras que la librería de usuario gestiona la estructura interna del heap del proceso.

---
### 7 TLBs — Translation Lookaside Buffer:
<img width="659" height="233" alt="image" src="https://github.com/user-attachments/assets/855f9957-fc4c-4528-a4dc-a877d1766a92" />

#### 7.1 Actividad: Localidad y TLB — Análisis
#### 7.1.1 Comparativa de rendimiento
Basado en las tres ejecuciones de tlb_locality, los tiempos son los siguientes:

| Ejecución   | Secuencial (ms) | Aleatorio (ms) | Factor de lentitud (Aleatorio / Secuencial) |
|-------------|-----------------|----------------|---------------------------------------------|
| 1           | 7.73 ms         | 40.94 ms       | ~5.29x                                      |
| 2           | 11.06 ms        | 38.97 ms       | ~3.52x                                      |
| 3           | 9.21 ms         | 41.92 ms       | ~4.55x                                      |
|**Promedio** | **9.33 ms**     | **40.61 ms**   | **4.35x veces más lento**                   |

El acceso aleatorio es, en promedio, **4.35 veces más lento** que el secuencial.

#### 7.1.2 Explicación mediante el modelo del TLB
El TLB es una caché de hardware que almacena traducciones recientes de VPN a PFN para acelerar el acceso a memoria.

- **Acceso Secuencial**: Presenta una alta localidad espacial. Al acceder a un elemento, la traducción de la página se guarda en el TLB; como el siguiente elemento está en la misma página, ocurre un TLB hit. El hit rate es muy cercano al 100%, ya que solo hay un fallo (miss) cuando se cambia de página (cada 4 KB).

- **Acceso Aleatorio**: Presenta una baja localidad. Cada acceso apunta probablemente a una página distinta que no está en la caché. Esto provoca constantes TLB misses, obligando al hardware a consultar la tabla de páginas en la memoria principal (mucho más lenta) para cada traducción. El hit rate cae drásticamente.

3. Impacto de páginas de 64 KB frente a 4 KB
Desde el punto de vista del TLB: La situación mejoraría para el acceso aleatorio. Una página de 64 KB es 16 veces más grande que una de 4 KB, lo que significa que cada entrada del TLB "cubre" mucha más memoria física. Esto aumenta la probabilidad de que una dirección aleatoria caiga dentro de una página ya traducida en el TLB (mayor cobertura).

Desde el punto de vista del uso de memoria: La situación empeoraría debido a la fragmentación interna. Si un proceso solo necesita pequeños bloques de datos pero el sistema asigna páginas de 64 KB, se desperdicia mucha memoria RAM física que no puede ser utilizada por otros procesos.

4. No teniamos instalado `perf`

---

## Enlace al vídeo



---

## Manifiesto de transparencia



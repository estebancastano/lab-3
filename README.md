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

## Enlace al vídeo



---

## Manifiesto de transparencia



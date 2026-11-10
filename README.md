# Práctica 4.1 – Simulador de oferta y demanda

| Dato | Valor |
|---|---|
| Estudiante | Roberto Carlos Robles Campos |
| Escuela | Tecnológico Nacional de México – Instituto Tecnológico de Mazatlán |
| Materia | Desarrollo Sustentable (ACD-0908) |
| Práctica | 4.1-P2 Simulador de oferta y demanda con potenciómetros |

**Código:** [`codigo/Mercado.ino`](codigo/Mercado.ino) · **Escenario de Make:** [`codigo/blueprint.json`](codigo/blueprint.json) · **Resultados (PDF):** [`resultados/Resultados.pdf`](resultados/Resultados.pdf) · **Video:** [`video/enlace.txt`](video/enlace.txt)

## 1. Objetivo
Explicar cómo la oferta y la demanda fijan los precios, relacionar la escasez de un recurso natural (el camarón) con su precio, y construir un sistema IoT que convierta un fenómeno económico en datos y alertas automáticas.

## 2. Investigación previa

**Oferta, demanda y precio de equilibrio.** Según Banco de México (Educa Banxico), la oferta es la cantidad de bienes y servicios que se ponen a la venta, y la demanda es la cantidad que desean comprar los interesados [1]. El precio es el punto de acuerdo entre compradores y vendedores, y es donde coinciden la oferta y la demanda [2]: eso es el **precio de equilibrio**. La propia página da un ejemplo: si una sequía arruina las cosechas, la oferta baja y el precio sube [1]. (Banxico no define "precio de equilibrio" con ese nombre; la definición anterior es la idea de su capítulo "Mercados".)

**Qué es una veda y por qué se aplica al camarón.** Una veda es un periodo en el que se prohíbe pescar una especie. En el Pacífico mexicano, el Acuerdo de la SADER/CONAPESCA publicado en el Diario Oficial de la Federación (firmado el 11 de septiembre de 2025) establece para 2026 una veda de **todas las especies de camarón** en los sistemas lagunarios, marismas y bahías de Sonora y del centro y norte de Sinaloa, y en las aguas marinas federales del Pacífico (incluido el Golfo de California), **a partir de las 00:00 horas del 3 de marzo de 2026** [3]. Su finalidad es "resguardar los procesos de reproducción y reclutamiento" del camarón, con base en una opinión técnica del IMIPAS [3]. Otra nota explica que su objetivo principal es proteger los procesos reproductivos y de crecimiento del camarón silvestre [4]. El mismo acuerdo había levantado la veda anterior el 19 de septiembre de 2025 en lagunas y bahías, y el 29 de septiembre en aguas marinas [3][5]. En la temporada que cerró, Sinaloa capturó 7,861 toneladas de camarón en 115 días (hasta el 27 de enero) [6].

**Dos ejemplos reales en Sinaloa de escasez que subió el precio**

1. **Jitomate (2026).** La sequía redujo el agua de riego en la siembra de octubre y noviembre en Sinaloa, y las lluvias fuera de temporada de enero causaron tizón temprano; a la vez, heladas en Florida redujeron la producción de Estados Unidos. Con menos oferta, el precio del jitomate en Estados Unidos subió cerca de 40 % entre enero y abril, el mayor aumento trimestral desde 2006, y algunos estados de México tuvieron aumentos superiores a 100 % frente al año anterior [7]. Otra nota confirma que las principales zonas productoras de Sinaloa y Sonora enfrentaron ciclos de agua restringidos [8].
2. **Maíz (2021).** *Ojo: esta nota es de mayo de 2021.* Un representante del sector dijo que Sinaloa, que produce 22 % del maíz del país, traería 4.8 millones de toneladas en lugar de 6 millones "porque no hay los riegos suficientes" por la sequía. El precio de la tonelada pasó de 3,800–4,100 pesos un año antes a 7,800–8,000 pesos. La nota no aclara si ese precio es nacional o solo de Sinaloa [9].

> **No se verificó** cuánto sube el precio del camarón al consumidor durante la veda en Mazatlán: no se encontró una fuente que se pudiera abrir y leer, así que no se afirma ninguna cifra.

## 3. Cómo funciona

Flujo: **Potenciómetros → Arduino UNO R4 WiFi (calcula y muestra el precio) → Webhook de Make → Telegram (alerta de escasez)**.

![Escenario Mercado69 en Make: webhook y Telegram](imagenes/escenario_completo.jpg)

### Circuito
| Elemento | Conexión |
|---|---|
| Potenciómetro de **demanda** (10 kΩ) | pata izquierda a GND, pata central a **A0**, pata derecha a 5V |
| Potenciómetro de **oferta** (10 kΩ) | pata izquierda a GND, pata central a **A1**, pata derecha a 5V |

### El código ([`codigo/Mercado.ino`](codigo/Mercado.ino))
1. Se conecta al WiFi (2.4 GHz) y muestra "WiFi" y luego "OK" en la matriz LED.
2. Cada 0.5 s lee los dos potenciómetros y convierte cada lectura (0–1023) a una escala de 10 a 90 con `map()`: eso es la **demanda** y la **oferta**.
3. Calcula el precio de equilibrio con un modelo lineal: la demanda es `P = a − Q` con `a = 100 + demanda`, y la oferta es `P = Q − b` con `b = oferta − 50`. Donde se cruzan, `P = (a − b) / 2`, que se multiplica por 4 para dar pesos por kilo: `precio = ((150 + demanda − oferta) / 2) × 4`. Con la demanda y la oferta iguales el precio es $300; si la demanda sube o la oferta baja, el precio sube.
4. Muestra el precio en la matriz LED y en el Monitor Serie (9600 baudios).
5. Hay **escasez** cuando `demanda − oferta ≥ 25`. En ese momento envía una petición HTTPS (GET) al webhook de Make con `demanda`, `oferta`, `precio` y `estado=escasez`. Solo avisa **cuando el mercado pasa a escasez** y como máximo **una vez por minuto**, para no gastar créditos de Make en cada lectura.

> El WiFi y la ruta del webhook están sin datos reales (`NOMBRE_DE_TU_RED`, `CLAVE_DE_TU_RED`, `/PEGA_AQUI_EL_ID`). El host del webhook depende de la cuenta (`us1`, `us2`, `eu1`, `eu2`).

### El escenario de Make ([`codigo/blueprint.json`](codigo/blueprint.json))
| # | Módulo | Configuración |
|---|---|---|
| 1 | Webhooks → Custom webhook | Recibe `demanda`, `oferta`, `precio` y `estado` |
| 2 | Telegram Bot → Send a Text Message or a Reply | Conexión del bot, Chat ID del usuario, texto: *ALERTA DE ESCASEZ · Demanda · Oferta · Precio de equilibrio · La demanda supera a la oferta: el precio sube.* |

Se ejecuta **Immediately** (cada vez que llega una alerta). El Chat ID del blueprint está reemplazado por `TU_CHAT_ID`, y el token del bot no se incluye en ningún archivo.

## 4. Experimentos de mercado

La demanda y la oferta van de 10 a 90 y el precio sale de `((150 + demanda − oferta) / 2) × 4`.

| Escenario | Demanda | Oferta | Precio ($/kg) | ¿Alerta? | Explicación económica |
|---|---|---|---|---|---|
| Temporada normal | 50 | 50 | 300* | No | La oferta iguala a la demanda: el precio queda en su nivel base. |
| Veda de camarón | 57 | 10 | 392 | Sí | La veda prohíbe pescar y la oferta cae al mínimo; la demanda sigue, así que el precio sube. |
| Semana Santa (alta demanda) | 90 | 20 | 440 | Sí | Muchos más compradores para la misma cantidad de camarón: el precio sube. |
| Sobreproducción de granjas | 30 | 85 | 188* | No | Hay más camarón del que se quiere comprar: el precio baja y no hay escasez. |
| Huracán que cierra la pesca | 80 | 10 | 440 | Sí | Nadie puede salir a pescar, la oferta se desploma y el precio se dispara. |

\*Valores **calculados con la fórmula del código** (sin alerta, porque no hay escasez); los demás son lecturas del Arduino que generaron la alerta de Telegram.

Otras alertas recibidas durante las pruebas: demanda 80 / oferta 20 → $420; demanda 80 / oferta 41 → $376; demanda 90 / oferta 20 → $440; demanda 58 / oferta 22 → $372. En el Monitor Serie con demanda 90 y oferta 10 el precio llegó a $460, que es el máximo del modelo.

## 5. Video de la prueba en vivo
[Ver la prueba en YouTube](https://youtu.be/fHW2rqMyLG8)

## 6. Preguntas de reflexión

**1. ¿Por qué el precio sube cuando la oferta baja, aunque el producto sea el mismo?**
El producto es el mismo, pero hay menos kilos disponibles para la misma cantidad de personas que lo quieren comprar. Cuando la cantidad que desean comprar los consumidores es mayor a la que se pone a la venta, los compradores compiten entre sí y algunos están dispuestos a pagar más para no quedarse sin producto. Los vendedores lo notan, porque el camarón se agota rápido, y suben el precio hasta que solo compran quienes más lo valoran. Así se llega a un nuevo precio de equilibrio, más alto. Banxico da el mismo ejemplo con una sequía que arruina las cosechas: baja la oferta y sube el precio [1]. En el simulador se ve claro: dejo la demanda en 80 y bajo la oferta de 41 a 10, y el precio pasa de $376 a $440.

**2. Una veda reduce la oferta y sube el precio. ¿Por qué aun así conviene para la economía de largo plazo?**
Porque el camarón es un recurso que se renueva solo si se le deja reproducirse. La veda protege justo la etapa de reproducción y reclutamiento, es decir, cuando nacen y crecen los camarones jóvenes [3][4]. Si se pescara todo el año, cada temporada habría menos camarón adulto, las capturas bajarían y el precio subiría de forma permanente, sin recuperación. Con la veda se acepta un precio más alto unos meses, a cambio de que la población se mantenga y haya pesca en las siguientes temporadas, y con ella empleos para pescadores, plantas y comercios. Es una pérdida pequeña y temporal contra una ganancia grande y duradera. Además, ordena el mercado: todos dejan de pescar a la vez y nadie obtiene ventaja pescando en la época prohibida.

**3. ¿Qué factores reales de la oferta de camarón no considera su modelo (clima, costos de combustible, importaciones)?**
Mi modelo es muy sencillo: solo usa dos números que yo muevo con los potenciómetros, y el precio es una fórmula lineal. No considera el clima real: huracanes, sequías o cambios de temperatura del mar, que alteran los ciclos del camarón. Tampoco considera los costos de producir y pescar, como el diésel de los barcos, el hielo, la mano de obra y el alimento de las granjas. No incluye las importaciones, que pueden bajar el precio nacional si entra camarón de otros países a un precio menor, ni las exportaciones, que sacan producto del mercado local. Tampoco distingue entre camarón de pesca y de granja, ni entre tallas, ni modela la competencia, los intermediarios, los impuestos o la inflación. Por eso sirve para entender la idea, pero no para predecir el precio real.

**4. ¿Qué ventaja tiene automatizar una alerta de escasez para un productor o una cooperativa?**
Les avisa en el momento en que cambia el mercado, sin tener que estar revisando precios o inventarios todo el día. Con la alerta pueden decidir a tiempo: guardar producto para venderlo cuando el precio esté más alto, renegociar con compradores, programar la siembra o la cosecha, o comprar insumos antes de que suban. Una cooperativa tiene varios socios, y una alerta en Telegram les llega a todos a la vez y deja un registro de cuándo ocurrió cada escasez, de manera que todos tengan la misma información. Además, con Make no necesitan programar ni pagar un sistema caro: es un flujo sencillo que se puede cambiar. En mi simulador, la alerta solo se manda cuando el mercado *pasa* a escasez y como máximo una vez por minuto, así no se llenan de mensajes ni gastan créditos de más.

**5. ¿El precio de mercado incluye el costo ambiental de producir el camarón? Expliquen.**
Normalmente no por completo. El precio que paga el consumidor refleja lo que cuesta pescar o criar el camarón (combustible, mano de obra, insumos) y lo que la gente quiere comprarlo, pero casi nunca refleja el daño que se hace al ambiente. Por ejemplo, las granjas camaroneras pueden cambiar o contaminar esteros y manglares, y la pesca de arrastre captura otras especies que se desperdician; ese daño lo paga la sociedad, no quien produce ni quien compra. Los economistas llaman a esto un costo que el mercado no cobra. La veda es una forma de corregirlo: una regla que obliga a cuidar el recurso aunque el precio por sí solo no lo haría. Para que el precio incluyera el costo ambiental harían falta reglas, impuestos o certificaciones que obliguen a pagarlo.

## 7. Fuentes

Las fuentes se abrieron y leyeron el 10 de noviembre de 2026.

1. Banco de México, Educa Banxico, *Oferta y demanda*: <https://educa.banxico.org.mx/yo-y-la-economia/oferta-y-demanda/oferta-demanda.html>
2. Banco de México, Educa Banxico, *Mercados*: <https://educa.banxico.org.mx/yo-y-la-economia/oferta-y-demanda/1/mercados.html>
3. SADER/CONAPESCA, Acuerdo que da a conocer el levantamiento en 2025 de la veda temporal de camarón y establece la de 2026 (Diario Oficial de la Federación, vía SIDOF): <https://sidof.segob.gob.mx/notas/docFuente/5768065>
4. Luz Noticias, 07/04/2025, *Veda de camarón en Sinaloa: cuáles son las reglas*: <https://www.luznoticias.mx/2025-04-07/mexico/veda-de-camaron-en-sinaloa-cuales-son-las-reglas-para-pescadores-y-comercializadoras/240438>
5. Luz Noticias, 10/09/2025, *Estas serán las fechas para el levantamiento de la veda al camarón en el Pacífico*: <https://www.luznoticias.mx/2025-09-10/agro/estas-seran-las-fechas-para-el-levantamiento-de-la-veda-al-camaron-en-el-pacifico-todos-a-la-pesca/259099>
6. Luz Noticias, 02/03/2026, *Arranca la veda del camarón este 3 de marzo*: <https://www.luznoticias.mx/2026-03-02/agro/arranca-la-veda-del-camaron-este-3-de-marzo/280139>
7. El Financiero (Bloomberg), 11/06/2026, *Precio del jitomate se dispara nuevamente*: <https://www.elfinanciero.com.mx/economia/2026/06/11/precio-del-jitomate-se-dispara-nuevamente-y-no-por-trump-por-que-motivo-es-mas-caro-en-mexico-y-eu/>
8. El Imparcial, 27/03/2026, *México se consolida como el mayor exportador de tomate… el precio local sube por la sequía extrema en Sinaloa*: <https://www.elimparcial.com/dinero/2026/03/27/mexico-se-consolida-como-el-mayor-exportador-de-tomate-del-mundo-en-2026-pero-el-precio-local-sube-por-la-sequia-extrema-en-sinaloa-y-el-aumento-en-costos-de-produccion-que-disparan-el-costo-al-consumidor-final/>
9. Eje Central, *Sequía extrema, la otra embestida a la canasta básica* (nota de mayo de 2021): <https://www.ejecentral.com.mx/sequia-extrema-la-otra-embestida-a-la-canasta-basica>

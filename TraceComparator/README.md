# TraceComparator


Este módulo forma parte del Proyecto Fin de Grado: *Captura y Sincronización de Datos Biométricos con EmotiBit y RoboTEA*.

## Descripción

Herramienta desarrollada para comparar gráficas y señales biométricas obtenidas desde un ismo dispositivo EmotiBit, pero capturadas desde diferentes disposisitivos, ya que las señales pueden grabarse la tarjeta SD de EmotiBit o ser obtenidas a través de comunicación UDP. Permite evaluar la sincronización, similitud o desajuste entre señales, facilitando el análisis postcaptura.

## Contenido

- Código fuente (.cpp, .h, .ui)
- Archivo de proyecto Qt (`ComparadorGraficas.pro`)
- Documentación Doxygen en la carpeta `doc/`

## Cómo compilar

Abre el archivo `ComparadorGraficas.pro` con Qt Creator y compílalo en modo **Release** o **Debug**.

### Requisitos de compilación

- **Qt versión**: 6.7.2  
- **Compilador**: MSVC 2019 (Visual Studio 16.11)  
- **Sistema operativo**: Windows 10/11

> Puedes instalar Qt con MSVC desde el instalador oficial: [https://www.qt.io/download](https://www.qt.io/download)


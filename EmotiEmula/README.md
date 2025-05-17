# EmotiEmula

Este módulo forma parte del Proyecto Fin de Grado: *Captura y Sincronización de Datos Biométricos con EmotiBit y RoboTEA*.

## Descripción

Herramienta que emula el comportamiento del dispositivo EmotiBit para permitir pruebas sin hardware físico. Soporta lectura desde archivos CSV y simula el envío de datos por red utilizando sockets UDP y TCP.

## Contenido

- Código fuente (.cpp, .h, .ui)
- Archivo de proyecto Qt (`EmotiEmula.pro`)
- Documentación Doxygen en la carpeta `doc/`

## Cómo compilar

Abre el archivo `EmotiEmula.pro` con Qt Creator y compílalo en modo **Release** o **Debug**.

### Requisitos de compilación

- **Qt versión**: 6.7.2
- **Compilador**: MSVC 2019 (Visual Studio 16.11)
- **Sistema operativo**: Windows 10/11

> Puedes instalar Qt con MSVC desde el instalador oficial: [https://www.qt.io/download](https://www.qt.io/download)
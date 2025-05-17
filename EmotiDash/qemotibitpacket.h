/*
MIT License

Copyright (c) 2018 Connected Future Labs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

--------------------------------------------------------------------------------
Este archivo ha sido modificado por Enrique Fuentes en el contexto de su Trabajo Fin de Grado.
El código se ha adaptado para integrarse en la herramienta RoboTEA, utilizando el
entorno Qt, extendiendo su funcionalidad y reorganizando su estructura.
*/

/**
* @file QEmotiBitPacket
* @date Mayo 2025
*
* @details
* Proyecto desarrollado en C++ con Qt 6.7.2 para la UNED.
*
* - Framework: Qt (módulos: Network, Widgets, Core, etc.)
* - Compilador: MSVC / MinGW (según configuración)
* - Entorno: Windows 10/11
* - Herramientas de documentación: Doxygen, Graphviz
* - Licencia: MIT (si corresponde)
*/


#ifndef QEMOTIBITPACKET_H
#define QEMOTIBITPACKET_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDateTime>
#include <unordered_map>

/*!
 * \file QEmotiBitPacket.h
 * \brief Proporciona funcionalidades para la creación, análisis y gestión de paquetes específicos de EmotiBit.
 *
 * Gestiona la estructura y contenido de los paquetes de datos intercambiados con dispositivos EmotiBit,
 * facilitando la comunicación estructurada y el análisis detallado de los datos transmitidos y recibidos.
 *
 * \see EmotiBitWiFiRoboTEA, EmotiBitController
 * \date 2025-05-08
 */

class qEmotiBitPacket {
public:


    static const qint8 FAIL = -1;
    static const qint16 NO_PACKET_DATA = -2;
    static const qint16 MALFORMED_HEADER = -3;


    // Definición de TypeTags
    class TypeTag {
    public:
        static const QString EDA;
        static const QString EDL;
        static const QString EDR;
        static const QString PPG_INFRARED;
        static const QString PPG_RED;
        static const QString PPG_GREEN;
        static const QString SPO2;
        static const QString TEMPERATURE_0;
        static const QString TEMPERATURE_1;
        static const QString THERMOPILE;
        static const QString HUMIDITY_0;
        static const QString ACCELEROMETER_X;
        static const QString ACCELEROMETER_Y;
        static const QString ACCELEROMETER_Z;
        static const QString GYROSCOPE_X;
        static const QString GYROSCOPE_Y;
        static const QString GYROSCOPE_Z;
        static const QString MAGNETOMETER_X;
        static const QString MAGNETOMETER_Y;
        static const QString MAGNETOMETER_Z;
        static const QString BATTERY_VOLTAGE;
        static const QString BATTERY_PERCENT;
        static const QString BUTTON_PRESS_SHORT;
        static const QString BUTTON_PRESS_LONG;
        static const QString DATA_CLIPPING;
        static const QString DATA_OVERFLOW;
        static const QString SD_CARD_PERCENT;
        static const QString RESET;
        static const QString EMOTIBIT_DEBUG;
        static const QString ACK;
        static const QString NACK;
        static const QString REQUEST_DATA;
        static const QString TIMESTAMP_EMOTIBIT;
        static const QString TIMESTAMP_LOCAL;
        static const QString TIMESTAMP_UTC;
        static const QString TIMESTAMP_CROSS_TIME;
        static const QString EMOTIBIT_MODE;
        static const QString EMOTIBIT_INFO;
        static const QString HEART_RATE;
        static const QString INTER_BEAT_INTERVAL;
        static const QString SKIN_CONDUCTANCE_RESPONSE_AMPLITUDE;
        static const QString SKIN_CONDUCTANCE_RESPONSE_FREQ;
        static const QString SKIN_CONDUCTANCE_RESPONSE_RISE_TIME;


//--------------------------------------------------
        static const QString RECORD_BEGIN;
        static const QString RECORD_END;
        static const QString USER_NOTE;
        static const QString MODE_NORMAL_POWER;
        static const QString MODE_LOW_POWER;
        static const QString MODE_MAX_LOW_POWER;
        static const QString MODE_WIRELESS_OFF;
        static const QString MODE_HIBERNATE;
        static const QString HELLO_EMOTIBIT;
        static const QString HELLO_HOST;
        static const QString EMOTIBIT_CONNECT;
        static const QString EMOTIBIT_DISCONNECT;
        static const QString SERIAL_DATA_ON;
        static const QString SERIAL_DATA_OFF;
        static const QString WIFI_ADD;
        static const QString WIFI_DELETE;
        static const QString LIST;
//______________________________________________________

        static const QString PING;
        static const QString PONG;


        // Quizas falten PayloadLabels
    };

    // Definición de PayloadLabels
    class PayloadLabel {
    public:
        static const QString CONTROL_PORT;
        static const QString DATA_PORT;
        static const QString DEVICE_ID;
        static const QString RECORDING_STATUS;
        static const QString POWER_STATUS;
        static const QString LSL_MARKER_RX_TIMESTAMP;
        static const QString LSL_MARKER_SRC_TIMESTAMP;
        static const QString LSL_LOCAL_CLOCK_TIMESTAMP;
        static const QString LSL_MARKER_DATA;

    };

    // Delimitadores y constantes
    static const QChar PAYLOAD_DELIMITER;
    static const QChar PACKET_DELIMITER_CSV;
    static const int HEADER_LENGTH = 6; // Número de elementos en el encabezado


    static const QString TIMESTAMP_STRING_FORMAT;


    // Estructura para el encabezado del paquete
    struct Header {
        QString typeTag;
        quint64 timestamp;
        quint16 packetNumber;
        quint16 dataLength;
        quint8 protocolVersion;
        quint8 dataReliability;
    };

    // Métodos para crear y parsear paquetes
    static QString createPacket(const QString &typeTag, quint16 packetNumber, const QString &data, quint16 dataLength, quint8 protocolVersion = 1, quint8 dataReliability = 100);
    static QString createPacket(const QString &typeTag, quint16 packetNumber, const QVector<QString> &data, quint8 protocolVersion=1, quint8 dataReliability=100);
    static bool getHeader(const QStringList &packetElements, Header &packetHeader);
    static qint16 getHeader(const QString &packet, Header &packetHeader);
    //static int getPacketElement(const QStringList &packet, QString &element, int index);
    static qint16 getPacketKeyedValue(const QStringList &packet, const QString &key, QString &value, int startIndex = 0);
    static qint16 getPacketKeyedValue(const QString &packet, const QString &key, QString &value, int startChar);
    static qint16 getPacketElement(const QString &packet, QString &element, qint16 startChar);


    // Estructura para la configuración
    struct WifiHostSettings {
        qint64 checkAdvertisingInterval = 1000; // Intervalo en milisegundos        
    };


        // Definición de EmotibitInfo
    struct EmotibitInfo {
        QString ip;
        bool isAvailable;
        qint64 lastSeen;  // Nueva variable para el tiempo de detección
        EmotibitInfo(const QString &ipAddress, bool available, qint64 lastSeenTime = 0)  : ip(ipAddress), isAvailable(available), lastSeen(lastSeenTime) {}
    };

    std::unordered_map<QString, EmotibitInfo> _discoveredEmotibits;
    QString connectedEmotibitIp;

private:
    // Constructor privado para evitar instancias; todos los métodos son estáticos
    qEmotiBitPacket() {}
};

#endif

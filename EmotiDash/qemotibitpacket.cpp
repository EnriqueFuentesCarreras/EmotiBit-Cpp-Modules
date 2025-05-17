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

/****************************************************************************
 * qEmotiBitPacket.cpp
 *
 * @brief Implementación de funciones para la construcción, análisis y gestión
 *        de paquetes de comunicación utilizados por dispositivos EmotiBit.
 *
 * Este archivo proporciona:
 * - Creación de paquetes con encabezado y payload.
 * - Extracción de encabezados y elementos de datos.
 * - Utilidades para búsqueda de valores clave dentro de paquetes.
 * - Definiciones de constantes como etiquetas de tipo (TypeTags) y labels de payload (PayloadLabels). *
 * @date 2025-04-29
 ****************************************************************************/


#include "qEmotiBitPacket.h"

// Inicialización de TypeTags
const QString qEmotiBitPacket::TypeTag::EDA = "EA";
const QString qEmotiBitPacket::TypeTag::EDL = "EL";
const QString qEmotiBitPacket::TypeTag::EDR = "ER";
const QString qEmotiBitPacket::TypeTag::PPG_INFRARED = "PI";
const QString qEmotiBitPacket::TypeTag::PPG_RED = "PR";
const QString qEmotiBitPacket::TypeTag::PPG_GREEN = "PG";
const QString qEmotiBitPacket::TypeTag::SPO2 = "O2";
const QString qEmotiBitPacket::TypeTag::TEMPERATURE_0 = "T0";
const QString qEmotiBitPacket::TypeTag::TEMPERATURE_1 = "T1";
const QString qEmotiBitPacket::TypeTag::THERMOPILE = "TH";
const QString qEmotiBitPacket::TypeTag::HUMIDITY_0 = "H0";
const QString qEmotiBitPacket::TypeTag::ACCELEROMETER_X = "AX";
const QString qEmotiBitPacket::TypeTag::ACCELEROMETER_Y = "AY";
const QString qEmotiBitPacket::TypeTag::ACCELEROMETER_Z = "AZ";
const QString qEmotiBitPacket::TypeTag::GYROSCOPE_X = "GX";
const QString qEmotiBitPacket::TypeTag::GYROSCOPE_Y = "GY";
const QString qEmotiBitPacket::TypeTag::GYROSCOPE_Z = "GZ";
const QString qEmotiBitPacket::TypeTag::MAGNETOMETER_X = "MX";
const QString qEmotiBitPacket::TypeTag::MAGNETOMETER_Y = "MY";
const QString qEmotiBitPacket::TypeTag::MAGNETOMETER_Z = "MZ";
const QString qEmotiBitPacket::TypeTag::BATTERY_VOLTAGE = "BV";
const QString qEmotiBitPacket::TypeTag::BATTERY_PERCENT = "B%";
const QString qEmotiBitPacket::TypeTag::BUTTON_PRESS_SHORT = "BS";
const QString qEmotiBitPacket::TypeTag::BUTTON_PRESS_LONG = "BL";
const QString qEmotiBitPacket::TypeTag::DATA_CLIPPING = "DC";
const QString qEmotiBitPacket::TypeTag::DATA_OVERFLOW = "DO";
const QString qEmotiBitPacket::TypeTag::SD_CARD_PERCENT = "SD";
const QString qEmotiBitPacket::TypeTag::RESET = "RS";
const QString qEmotiBitPacket::TypeTag::EMOTIBIT_DEBUG = "DB";
const QString qEmotiBitPacket::TypeTag::ACK = "AK";
const QString qEmotiBitPacket::TypeTag::NACK = "NK";
const QString qEmotiBitPacket::TypeTag::REQUEST_DATA = "RD";
const QString qEmotiBitPacket::TypeTag::TIMESTAMP_EMOTIBIT = "TE";
const QString qEmotiBitPacket::TypeTag::TIMESTAMP_LOCAL = "TL";
const QString qEmotiBitPacket::TypeTag::TIMESTAMP_UTC = "TU";
const QString qEmotiBitPacket::TypeTag::TIMESTAMP_CROSS_TIME = "TX";
const QString qEmotiBitPacket::TypeTag::EMOTIBIT_MODE = "EM";
const QString qEmotiBitPacket::TypeTag::EMOTIBIT_INFO = "EI";
const QString qEmotiBitPacket::TypeTag::HEART_RATE = "HR";
const QString qEmotiBitPacket::TypeTag::INTER_BEAT_INTERVAL = "BI";
const QString qEmotiBitPacket::TypeTag::SKIN_CONDUCTANCE_RESPONSE_AMPLITUDE = "SA";
const QString qEmotiBitPacket::TypeTag::SKIN_CONDUCTANCE_RESPONSE_FREQ = "SF";
const QString qEmotiBitPacket::TypeTag::SKIN_CONDUCTANCE_RESPONSE_RISE_TIME = "SR";



const QString qEmotiBitPacket::TypeTag::RECORD_BEGIN = "RB";
const QString qEmotiBitPacket::TypeTag::RECORD_END = "RE";


const QString qEmotiBitPacket::TypeTag::MODE_NORMAL_POWER = "MN";				// la sincronización de los tiempos debe ser precisa.
const QString qEmotiBitPacket::TypeTag::MODE_LOW_POWER = "ML";				// la sincronización de los tiempos debe ser precisa.
const QString qEmotiBitPacket::TypeTag::MODE_MAX_LOW_POWER = "MM";			//  la precisión de la sincronización de los tiempos disminuye.
const QString qEmotiBitPacket::TypeTag::MODE_WIRELESS_OFF = "MO";			//  la sincronización de los tiempos debe ser precisa.
const QString qEmotiBitPacket::TypeTag::MODE_HIBERNATE = "MH";				// Apagado completo de todas las operaciones. request


const QString qEmotiBitPacket::TypeTag::EMOTIBIT_DISCONNECT = "ED";
const QString qEmotiBitPacket::TypeTag::SERIAL_DATA_ON = "S+";
const QString qEmotiBitPacket::TypeTag::SERIAL_DATA_OFF = "S-";
// Advertising TypeTags
const QString qEmotiBitPacket::TypeTag::PING = "PN";
const QString qEmotiBitPacket::TypeTag::PONG = "PO";
const QString qEmotiBitPacket::TypeTag::HELLO_EMOTIBIT = "HE";
const QString qEmotiBitPacket::TypeTag::HELLO_HOST = "HH";
const QString qEmotiBitPacket::TypeTag::EMOTIBIT_CONNECT = "EC";
// WiFi Credential management TypeTags
const QString qEmotiBitPacket::TypeTag::WIFI_ADD = "WA";
const QString qEmotiBitPacket::TypeTag::WIFI_DELETE = "WD";

//Information Exchange TypeTags
const QString qEmotiBitPacket::TypeTag::LIST = "LS";



const QString qEmotiBitPacket::TypeTag::USER_NOTE = "UN";



// Inicialización de PayloadLabels
const QString qEmotiBitPacket::PayloadLabel::CONTROL_PORT = "CP";
const QString qEmotiBitPacket::PayloadLabel::DATA_PORT = "DP";
const QString qEmotiBitPacket::PayloadLabel::DEVICE_ID = "DI";
const QString qEmotiBitPacket::PayloadLabel::RECORDING_STATUS = "RS";
const QString qEmotiBitPacket::PayloadLabel::POWER_STATUS = "PS";
const QString qEmotiBitPacket::PayloadLabel::LSL_MARKER_RX_TIMESTAMP = "LR";
const QString qEmotiBitPacket::PayloadLabel::LSL_MARKER_SRC_TIMESTAMP = "LM";
const QString qEmotiBitPacket::PayloadLabel::LSL_LOCAL_CLOCK_TIMESTAMP = "LC";
const QString qEmotiBitPacket::PayloadLabel::LSL_MARKER_DATA = "LD";


// Delimitadores y constantes
const QChar qEmotiBitPacket::PAYLOAD_DELIMITER = ',';
const QChar qEmotiBitPacket::PACKET_DELIMITER_CSV = '\n';    //Importante


const QString qEmotiBitPacket::TIMESTAMP_STRING_FORMAT = "yyyy-MM-dd_hh-mm-ss-%f";


/**
 * @brief Crea un paquete EmotiBit a partir de datos en formato QString.
 *
 * @param typeTag Tipo de paquete (ej: "EA", "HR", "AX").
 * @param packetNumber Número de secuencia del paquete.
 * @param data Datos del payload (separados por comas).
 * @param numElements Número de elementos en el payload.
 * @param protocolVersion Versión del protocolo utilizado.
 * @param dataReliability Nivel de confiabilidad de los datos.
 * @return QString Representación textual del paquete completo.
 */
QString qEmotiBitPacket::createPacket(const QString &typeTag, quint16 packetNumber, const QString &data, quint16 numElements, quint8 protocolVersion, quint8 dataReliability) {
    // Crear el encabezado
    qEmotiBitPacket::Header header;
    header.typeTag = typeTag;
    header.timestamp = QDateTime::currentMSecsSinceEpoch();
    header.packetNumber = packetNumber;
    header.dataLength = numElements; // Cambiar aquí para reflejar el número de elementos
    header.protocolVersion = protocolVersion;
    header.dataReliability = dataReliability;

    // Construir el paquete como una cadena
    QStringList packetElements;
    packetElements << QString::number(header.timestamp)
                   << QString::number(header.packetNumber)
                   << QString::number(header.dataLength)  // Usar el número de elementos
                   << header.typeTag
                   << QString::number(header.protocolVersion)
                   << QString::number(header.dataReliability);

    // Añadir los datos si existen
    if (!data.isEmpty()) {
        packetElements << data.split(PAYLOAD_DELIMITER);
    }

    // Unir todos los elementos con el delimitador
    QString packet = packetElements.join(PAYLOAD_DELIMITER);

    // Añadir el delimitador de paquete
    packet += PACKET_DELIMITER_CSV;

    return packet;
}
//_________________


/**
 * @brief Crea un paquete EmotiBit a partir de un QVector de datos.
 *
 * @param typeTag Tipo de paquete (ej: "EA", "HR", "AX").
 * @param packetNumber Número de secuencia del paquete.
 * @param data Vector de elementos de datos.
 * @param protocolVersion Versión del protocolo utilizado.
 * @param dataReliability Nivel de confiabilidad de los datos.
 * @return QString Representación textual del paquete completo.
 */

QString qEmotiBitPacket::createPacket(const QString &typeTag, quint16 packetNumber, const QVector<QString> &data, quint8 protocolVersion, quint8 dataReliability) {

    quint16 numElements = data.size();  // Calcula el número de elementos en el payload

    QString concatenatedData = data.join(QString(PAYLOAD_DELIMITER));

    return createPacket(typeTag, packetNumber, concatenatedData, numElements, protocolVersion, dataReliability);
}
//___________




/**
 * @brief Extrae el encabezado de un paquete a partir de su representación en texto completo.
 *
 * @param packet Cadena completa del paquete recibido.
 * @param packetHeader Estructura donde se almacenarán los datos extraídos del encabezado.
 * @return qint16 Posición inicial donde comienzan los datos (payload) o MALFORMED_HEADER si hay error.
 */
qint16 qEmotiBitPacket::getHeader(const QString &packet, Header &packetHeader)
{
    qint16 dataStartChar = 0;
    qint16 commaN = 0, commaN1;

    // timestamp
    commaN1 = packet.indexOf(PAYLOAD_DELIMITER, commaN);
    //qDebug() << "1 dataStartChar commaN1 " << commaN1;
    if (commaN1 == -1 || commaN1 <= commaN) return MALFORMED_HEADER;
    packetHeader.timestamp = packet.mid(commaN, commaN1 - commaN).toLongLong();

    // packetNumber
    commaN = commaN1 + 1;
    //qDebug() << "2 dataStartChar commaN1 " << commaN1;
    commaN1 = packet.indexOf(PAYLOAD_DELIMITER, commaN);
    if (commaN1 == -1 || commaN1 <= commaN) return MALFORMED_HEADER;
    packetHeader.packetNumber = packet.mid(commaN, commaN1 - commaN).toInt();

    // dataLength
    commaN = commaN1 + 1;
    //qDebug() << "3 dataStartChar commaN1 " << commaN1;
    commaN1 = packet.indexOf(PAYLOAD_DELIMITER, commaN);
    if (commaN1 == -1 || commaN1 <= commaN) return MALFORMED_HEADER;
    packetHeader.dataLength = packet.mid(commaN, commaN1 - commaN).toInt();

    // typeTag
    commaN = commaN1 + 1;
    //qDebug() << "4 dataStartChar commaN1 " << commaN1;
    commaN1 = packet.indexOf(PAYLOAD_DELIMITER, commaN);
    if (commaN1 == -1 || commaN1 <= commaN) return MALFORMED_HEADER;
    packetHeader.typeTag = packet.mid(commaN, commaN1 - commaN);

    // protocolVersion
    commaN = commaN1 + 1;
    //qDebug() << "5 dataStartChar commaN1 " << commaN1;
    commaN1 = packet.indexOf(PAYLOAD_DELIMITER, commaN);
    //qDebug() << "6 dataStartChar commaN1 " << commaN1;
    if (commaN1 == -1 || commaN1 <= commaN) return MALFORMED_HEADER;
    packetHeader.protocolVersion = packet.mid(commaN, commaN1 - commaN).toInt();

    // dataReliability
    commaN = commaN1 + 1;
    //qDebug() << "7 dataStartChar commaN1 " << commaN1;
    commaN1 = packet.indexOf(PAYLOAD_DELIMITER, commaN);
    //qDebug() << "8 dataStartChar commaN1 " << commaN1;
    if (commaN1 == -1) {
        // No hay más delimitadores; estamos al final del paquete
        commaN1 = packet.size();
        //qDebug() << "9 dataStartChar commaN1 " << commaN1;
        dataStartChar = NO_PACKET_DATA; // No hay datos adicionales
    } else {
        dataStartChar = commaN1 + 1;
        //qDebug() << "1 dataStartChar  " << dataStartChar;
    }

    if (commaN1 <= commaN) return MALFORMED_HEADER;
    packetHeader.dataReliability = packet.mid(commaN, commaN1 - commaN).toInt();
   // qDebug() << "PAQUETE DataStartChar  " << dataStartChar;
    return dataStartChar;
}









/**
 * @brief Extrae el encabezado de un paquete a partir de su representación en texto completo.
 *
 * @param packet Cadena completa del paquete recibido.
 * @param packetHeader Estructura donde se almacenarán los datos extraídos del encabezado.
 * @return qint16 Posición inicial donde comienzan los datos (payload) o MALFORMED_HEADER si hay error.
 */
bool qEmotiBitPacket::getHeader(const QStringList &packetElements, Header &packetHeader) {
    if (packetElements.size() < HEADER_LENGTH) {
        return false; // El paquete es demasiado corto
    }

    bool ok;
    packetHeader.timestamp = packetElements[0].toULongLong(&ok);
    if (!ok) return false;

    packetHeader.packetNumber = packetElements[1].toUShort(&ok);
    if (!ok) return false;

    packetHeader.dataLength = packetElements[2].toUShort(&ok);
    if (!ok) return false;

    packetHeader.typeTag = packetElements[3];

    packetHeader.protocolVersion = packetElements[4].toUShort(&ok);
    if (!ok) return false;

    packetHeader.dataReliability = packetElements[5].toUShort(&ok);
    if (!ok) return false;

    return true;
}



/**
 * @brief Extrae un único elemento (campo) del paquete a partir de una posición dada.
 *
 * @param packet Paquete completo como cadena de texto.
 * @param element Variable donde se almacenará el elemento extraído.
 * @param startChar Posición inicial para comenzar la extracción.
 * @return qint16 Nueva posición para la siguiente extracción o NO_PACKET_DATA si no hay más datos.
 */
qint16 qEmotiBitPacket::getPacketElement(const QString &packet, QString &element, qint16 startChar){
    int nextStartChar = -1;
    int commaN1 = packet.indexOf(PAYLOAD_DELIMITER, startChar);
    // qDebug() << "getPacketElement(packet " << packet << ",element " << element << ", startChar " << startChar;
    if (commaN1 != -1) {
        // Se encontró una coma, extraer elemento
        element = packet.mid(startChar, commaN1 - startChar);
        if (packet.length() > commaN1 + 1)        {
            nextStartChar = commaN1 + 1;
        }
    }
    else if (packet.length() > startChar) {
        // No se encontró una coma, retornar el último elemento
        element = packet.mid(startChar);
        nextStartChar = NO_PACKET_DATA;
    }
    //qDebug() << "return nextStartChar " << nextStartChar;
    return nextStartChar;
}




/**
 * @brief Busca y extrae el valor asociado a una clave (key) en un paquete representado como QStringList.
 *
 * @param packet Lista de elementos separados del paquete.
 * @param key Clave a buscar.
 * @param value Variable donde se almacenará el valor asociado a la clave.
 * @param startIndex Índice inicial donde comenzar la búsqueda.
 * @return qint16 Índice donde se encontró el valor o -1 si no se encontró.
 */
qint16 qEmotiBitPacket::getPacketKeyedValue(const QStringList &packet, const QString &key, QString &value, int startIndex){
    //qDebug() << "getPacketKeyedValue(packet " << packet << ",key " << key << ", value, startIndex " << startIndex;
    for (int i = startIndex; i < packet.size(); i++)    {
        if (packet.at(i).compare(key, Qt::CaseInsensitive) == 0)        {
            if (i + 1 < packet.size())    {
                value = packet.at(i + 1);
                return i + 1;
            }
            return -1; // No se encontró un valor después de la clave
        }
    }
    return -1; //
}



/**
 * @brief Busca y extrae el valor asociado a una clave (key) en un paquete representado como QString.
 *
 * @param packet Paquete completo como cadena de texto.
 * @param key Clave a buscar.
 * @param value Variable donde se almacenará el valor asociado a la clave.
 * @param startChar Posición inicial para comenzar la búsqueda.
 * @return qint16 Nueva posición tras encontrar el valor o -1 si no se encontró.
 */
qint16 qEmotiBitPacket::getPacketKeyedValue(const QString &packet, const QString &key, QString &value, int startChar){
    //qDebug() << "getPacketKeyedValue(packet " << packet << ",key " << key << ", value, startChar__ " << startChar;
    QString element;
    do  {
        startChar = qEmotiBitPacket::getPacketElement(packet, element, startChar);
        if (element.compare(key, Qt::CaseInsensitive) == 0)    {
            //startChar = qEmotiBitPacket::getPacketElement(packet, value, startChar);
            qEmotiBitPacket::getPacketElement(packet, value, startChar);
            if (!value.isEmpty()) {
               // qDebug() << "startChar " << startChar;
                return startChar;
            }
            return -1;	// Retorna -1 si se alcanza el final del paquete antes de encontrar un valor
        }
    } while (startChar > -1);
    return -1;	// Retorna -1 si se alcanza el final del paquete antes de encontrar la clave
}




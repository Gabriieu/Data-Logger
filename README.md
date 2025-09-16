# Data-Logger
Data Logger EC5
# 🌡️💡 Data Logger Arduino – Monitoramento Ambiental

Este projeto consiste em um **data logger** dedicado ao monitoramento de **temperatura**, **umidade relativa do ar** e **luminosidade** em ambientes controlados.  
Os dados são exibidos em um **LCD 16x2 com interface I2C**, armazenados em **memória EEPROM** e marcados com **timestamp** utilizando um módulo RTC.

---

## 🎯 Objetivos
- Criar um dispositivo de registro de dados (*data logger*) para monitoramento ambiental.  
- Monitorar:
  - **Temperatura** (°C ou °F)
  - **Umidade relativa do ar (%)**
  - **Luminosidade (%)**
- Registrar automaticamente as leituras em EEPROM com **data e hora**.
- Emitir **alertas visuais e sonoros** quando as leituras ultrapassarem limites pré-definidos.

---

## ⚙️ Especificações do Sistema
| Componente            | Função                                                                 |
|------------------------|-------------------------------------------------------------------------|
| **Microcontrolador**   | ATmega328P (Arduino Uno R3)                                            |
| **Armazenamento**      | Memória EEPROM para gravação dos dados com data/hora                   |
| **Relógio de Tempo Real (RTC)** | Fornece timestamps precisos para cada registro                 |
| **Display LCD 16x2 I2C** | Exibição de dados e status do sistema                                 |
| **Controles**          | Botões e joystick para navegação e configuração                        |
| **Indicadores Visuais** | LEDs para status operacional e alertas                                 |
| **Alerta Sonoro**      | Buzzer para avisos audíveis                                            |
| **Sensores**           | DHT11 (Temperatura/Umidade) e LDR (Luminosidade)                       |

---

## 🚨 Limites de Alerta (Triggers)
| Variável       | Faixa Segura         | Ação quando fora da faixa        |
|----------------|----------------------|-----------------------------------|
| Temperatura    | **15 a 25 °C**       | Acende LED vermelho e buzzer     |
| Luminosidade   | **0 a 30%**          | Acende LED vermelho e buzzer     |
| Umidade        | **30 a 50%**         | Acende LED vermelho e buzzer     |

Se qualquer leitura sair da faixa, os dados são **gravados na EEPROM** junto com **data e hora**.

---

## 🧩 Lista de Materiais
| Quantidade | Componente                  |
|-----------:|------------------------------|
| 1 | Arduino Uno R3 (ATmega328P)          |
| 1 | Sensor DHT11 (Temperatura/Umidade)   |
| 1 | LDR + Resistor 10 kΩ                 |
| 1 | Módulo RTC (Real Time Clock)         |
| 1 | LCD 16x2 com interface I2C           |
| 1 | Buzzer                                |
| 1 | Bateria 9V + suporte                 |
| – | Protoboard, jumpers, LEDs e resistores diversos |

---

## 🖼️ Esquema de Ligações (Principais Pinos)

| Componente        | Pino Arduino |
|-------------------|--------------|
| **DHT11**         | **2**       |
| **Joystick (eixo X)** | A0      |
| **Joystick botão** | 3          |
| **LDR**           | A2         |
| **LCD I2C**       | SDA → A4 / SCL → A5 |
| **RTC**           | SDA → A4 / SCL → A5 |
| **LED Vermelho**  | 7          |
| **LED Verde**     | 8          |
| **Buzzer**        | 6          |

---

## 💻 Bibliotecas Utilizadas
Certifique-se de instalar as seguintes bibliotecas na IDE Arduino:

- [`LiquidCrystal_I2C`](https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library)
- [`RTClib`](https://github.com/adafruit/RTClib)
- [`DHT sensor library`](https://github.com/adafruit/DHT-sensor-library)
- Biblioteca padrão `EEPROM` (já incluída na IDE)

---

## 🔧 Como Utilizar
1. **Montagem:**  
   Conecte os componentes conforme a tabela de pinos acima.
2. **Configuração:**  
   - Abra o código na IDE Arduino.  
   - Instale as bibliotecas listadas.
3. **Upload:**  
   Envie o código para o Arduino Uno.
4. **Operação:**  
   - O LCD exibirá data/hora, leituras dos sensores e menus de configuração.  
   - Use o **joystick/botões** para alternar entre telas e configurar unidades (°C/°F) e modo automático/manual.  
   - LEDs e buzzer indicarão alertas em tempo real.

---

## 📦 Estrutura de Armazenamento na EEPROM
Cada registro salvo contém:
- **Tipo**: `T` (Temperatura), `U` (Umidade), `L` (Luminosidade)
- **Valor**: Medição correspondente
- **Timestamp**: Data e hora exata da leitura

---

## 🖋️ Licença
Este projeto pode ser distribuído e modificado sob os termos da **MIT License**.

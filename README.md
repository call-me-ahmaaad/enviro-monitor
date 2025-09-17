# 🌡️ Enviro Monitor 💧

Enviro Monitor is a system designed to monitor room temperature and humidity. The system consists of two main components: hardware and software.

The hardware component is responsible for environmental data acquisition. It utilizes the XY-MD02 sensor, which communicates via the Modbus RTU protocol over an RS485 interface. The collected data is processed locally on an ESP8266-based edge device and subsequently transmitted to specific topics on an MQTT broker, implemented locally using Mosquitto.

The software component handles data visualization through a locally hosted web application. The interface is developed using HTML, CSS, and JavaScript, and is structured in a grid-based layout inspired by simple Human-Machine Interface (HMI) dashboards. This design provides an intuitive and organized representation of the environmental conditions, facilitating real-time monitoring and user interaction.

## ⚡ Motivation

This project represents my first serious attempt at combining IoT and web development. The primary motivation behind creating Enviro Monitor is to learn and practice these technologies hands-on. By building both the hardware and software components from scratch, I aim to gain practical experience in sensor integration, data communication, and interactive web interfaces.

# 🗝️ Key Features

- Real-time Temperature & Humidity Monitoring – Continuously monitors the room's temperature and humidity.
- XY-MD02 Sensor Integration – Uses the XY-MD02 sensor with the Modbus RTU protocol over RS485 for high-accuracy measurements.
- Edge Processing with ESP8266 – Sensor data is processed locally on an ESP8266-based edge device before being transmitted to the server.
- MQTT Communication – Publishes data to MQTT topics via a locally running Mosquitto broker, enabling IoT integration.
- Web-based Visualization – Displays collected data in a locally hosted web application built with HTML, CSS, and JavaScript.
- HMI-inspired Grid Interface – Features a structured, panel-like layout inspired by simple HMI dashboards for an intuitive user experience.
- Lightweight & Local Deployment – Can be fully deployed locally without requiring cloud connectivity, ideal for learning and experimentation with IoT systems.

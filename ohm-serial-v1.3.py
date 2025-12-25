'''obter informacoes atraves do OpenHardwareMonitor, diretamente no DLL
requer instalacao das bibliotecas pythonnet, pyserial, pyinstaller

a comunicao SERIAL funcionou bem rodando IDLE como ADM ou comum

>>> 1.3 mod para tela ST7789 com Arduino Nano (problemas com buffer size)

o que estou buscando???

Intel X99 / Fan #2 (RPM da fan)

Intel Xeon E5-2650 v3 / Temperatures / CPU Package (Temperatura do CPU)
Intel Xeon E5-2650 v3 / Load / CPU Total (% de uso de CPU)

Generic Memory / Load / Memory (% de uso de memoria)

NVIDIA NVIDIA GeForce RTX 2060 / Temperatures / GPU Core (Temperatura da GPU)
NVIDIA NVIDIA GeForce RTX 2060 / Load / GPU Core (% uso da GPU)
NVIDIA NVIDIA GeForce RTX 2060 / Fans / GPU (RPM da fan)


'''

import clr
import time
import serial
import serial.tools.list_ports
import os

# Carrega a DLL
clr.AddReference(r'C:\OpenHardwareMonitor\OpenHardwareMonitorLib.dll')

from OpenHardwareMonitor import Hardware
from OpenHardwareMonitor.Hardware import SensorType

# Inicializa o monitor
computer = Hardware.Computer()
computer.MainboardEnabled = True
computer.CPUEnabled = True
computer.GPUEnabled = True
computer.FanControllerEnabled = True
computer.ChipsetEnabled = True
computer.HDDEnabled = True
computer.Open()

print('arquivo obrigatorio:  C:\OpenHardwareMonitor\OpenHardwareMonitorLib.dll')
print('Executando comunicação com DLL OpenHardwareMonitor...\n\n')
import os
import time

def enviar_msg(texto):
    os.system('cls')
    print(texto)
    texto += '\n'
    encoded = texto.encode('utf-8')    # Divide o texto em blocos de até 62 bytes
    max_chunk_size = 62
    ser.write('@\n'.encode('utf-8'))     # limpa tela
    time.sleep(0.1)
    for i in range(0, len(encoded), max_chunk_size):
        chunk = encoded[i:i + max_chunk_size]
        ser.write(chunk)
        time.sleep(0.4)
    ser.write('\n'.encode('utf-8'))    # Envia terminador

def encontrar_esp8266():
    portas = serial.tools.list_ports.comports()
    for porta in portas:
        desc = porta.description.lower()
        if "usb" in desc or "esp" in desc or "cp210" in desc or "ch340" in desc:
            print(f"Dispositivo encontrado em: {porta.device}")
            return porta.device
    print("Dispositivo não encontrado.")
    return None

def get_values():
    cpu_fan = None
    cpu_temp = None
    cpu_load = None
    mem_load = None
    gpu_temp = None
    gpu_load = None
    gpu_fan = None

    for hardware in computer.Hardware:
        hardware.Update()
        for sensor in hardware.Sensors:
            if ('Intel X99' in hardware.Name):
                if ('Fan #2' in sensor.Name):
                    cpu_fan = sensor.Value
            elif ('Intel Xeon' in hardware.Name):
                if ('CPU Package' in sensor.Name) and (cpu_temp is None) and (sensor.SensorType == SensorType.Temperature):
                    cpu_temp = sensor.Value
                elif ('CPU Total' in sensor.Name) and (cpu_load is None) and (sensor.SensorType == SensorType.Load):
                    cpu_load = sensor.Value
            elif ('Generic Memory' in hardware.Name):
                if ('Memory' in sensor.Name) and (mem_load is None):
                    mem_load = sensor.Value
            elif ('NVIDIA' in hardware.Name):
                if ('GPU Core' in sensor.Name) and (gpu_temp is None) and (sensor.SensorType == SensorType.Temperature):
                    gpu_temp = sensor.Value
                elif ('GPU Core' in sensor.Name) and (gpu_load is None) and (sensor.SensorType == SensorType.Load):
                    gpu_load = sensor.Value
                elif ('GPU' in sensor.Name) and (gpu_fan is None) and (sensor.SensorType == SensorType.Fan):
                    gpu_fan = sensor.Value
                    
#    return cpu_fan, cpu_temp, cpu_load, mem_load, gpu_temp, gpu_load, gpu_fan
#    print('resultados:', cpu_fan, cpu_temp, cpu_load, mem_load, gpu_temp, gpu_load, gpu_fan)

    output_lines = []

    if gpu_temp > 74:
        output_lines.append("\n GPU TEMP ALERT!\n")

    if gpu_temp is not None:
        output_lines.append(f"GPU Temp:  {gpu_temp:.0f} C")
#    else:
#        output_lines.append("GPU Temp:  N/A")

    if gpu_load is not None:
        output_lines.append(f"GPU Load:  {gpu_load:.0f} % ")
#    else:
#        output_lines.append("GPU Load:  N/A")

    if gpu_fan is not None:
        output_lines.append(f"GPU Fan:  {gpu_fan:.0f}RPM")
#    else:
#        output_lines.append("GPU Fan    N/A")

    if cpu_temp is not None:
        output_lines.append(f"CPU Temp:  {cpu_temp:.0f} C")
#    else:
#        output_lines.append("CPU Temp:  N/A")

    if cpu_load is not None:
        output_lines.append(f"CPU Load:  {cpu_load:.0f} % ")
#    else:
#        output_lines.append("CPU Load:  N/A")

    if mem_load is not None:
        output_lines.append(f"RAM Load:  {mem_load:.0f} % ")
#    else:
#        output_lines.append("RAM Load:  N/A")

    if cpu_fan is not None:
        output_lines.append(f"CPU Fan: {cpu_fan:.0f}RPM")
#    else:
#        output_lines.append("CPU Fan:   N/A")

    # Junta tudo em uma única string
    output = "\n".join(output_lines)
#    output = ">".join(output_lines)
    if (output == None): output = 'Nenhum sensor compativel'
    return output
#    print(output)



# Inicializa a porta serial
ser = serial.Serial(encontrar_esp8266(), 115200)  # Ajuste a porta e baud rate conforme necessário
time.sleep(1.5)
enviar_msg(' inicializando > comunicacao...')

# Inicializa o monitor
computer = Hardware.Computer()
computer.MainboardEnabled = True
computer.CPUEnabled = True
computer.GPUEnabled = True
computer.RAMEnabled = True
computer.HDDEnabled = True
computer.Open()

# Atualiza e lista sensores
log_lines = []  # lista para acumular as linhas

for hardware in computer.Hardware:
    hardware.Update()
    header = f"\nHardware: {hardware.Name}"
    print(header)
    log_lines.append(header)

    for sensor in hardware.Sensors:
        line = f"  Sensor: {sensor.Name} | Tipo: {sensor.SensorType} | Valor: {sensor.Value}"
        print(line)
        log_lines.append(line)

# Salva no arquivo 'sensors.txt'
with open("sensors.txt", "w", encoding="utf-8") as f:
    f.write("\n".join(log_lines) + "\n")

print('\nMantenha a janela aberta pra enviar continuamente via Serial.')

while True:
    output = get_values()
    enviar_msg(output)
    time.sleep(5)

x = input()

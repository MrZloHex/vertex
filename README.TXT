███╗   ███╗ ██████╗ ███╗   ██╗ ██████╗ ██╗     ██╗████████╗██╗  ██╗
████╗ ████║██╔═══██╗████╗  ██║██╔═══██╗██║     ██║╚══██╔══╝██║  ██║
██╔████╔██║██║   ██║██╔██╗ ██║██║   ██║██║     ██║   ██║   ███████║
██║╚██╔╝██║██║   ██║██║╚██╗██║██║   ██║██║     ██║   ██║   ██╔══██║
██║ ╚═╝ ██║╚██████╔╝██║ ╚████║╚██████╔╝███████╗██║   ██║   ██║  ██║
╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝╚═╝   ╚═╝   ╚═╝  ╚═╝


  ░▒▓█ _vertex_ █▓▒░  
  The beating heart of **СоюзПечаль**—a night lamp that does more than shine.  

  ───────────────────────────────────────────────────────────────  
  ▓ OVERVIEW  
  **_vertex_** is a firmware running on an **ATmega328P**, controlling an  
  LED strip and a buzzer inside **СоюзПечаль**.  
  It communicates with an external command hub via **USART**,  
  allowing dynamic lighting modes and interactions.  
  Simple. Flexible. Cool as hell.  

  ───────────────────────────────────────────────────────────────  
  ▓ HARDWARE  
  ▪ **MCU**: ATmega328P (Currently Arduino Nano, later standalone)  
  ▪ **OUTPUTS**: LED Strip (WS2812B / other) + Buzzer  
  ▪ **INTERFACE**: USART for external command control  
  ▪ **POWER**: USB / External 5V  

  ───────────────────────────────────────────────────────────────  
  ▓ FEATURES  
  ▪ Custom lighting modes controlled via USART  
  ▪ Reactive buzzer for audio cues  
  ▪ Low-power idle mode  
  ▪ Designed for easy flashing & standalone ATmega328P use  

  ───────────────────────────────────────────────────────────────  
  ▓ BUILD & FLASH  
  Compile with **AVR-GCC** or Arduino IDE.  
  Flash using **AVRDUDE** or Arduino bootloader.  

  ```sh  
  make flash  # If using a Makefile  
  avrdude -c arduino -p m328p -P /dev/ttyUSB0 -b 115200 -U flash:w:vertex.hex:i  
  ```

  ───────────────────────────────────────────────────────────────  
  ▓ USART COMMANDS  
  _vertex_ listens for commands over USART to control lighting & sound.  
  Sample command format:  

  ```
  CMD:SET_MODE:1      # Switch to mode 1  
  CMD:BUZZER:ON       # Enable buzzer  
  CMD:SET_COLOR:255,0,0  # Set LED color to red  
  CMD:POWER_OFF       # Go to low-power mode  
  ```

  ───────────────────────────────────────────────────────────────  
  ▓ USAGE  
  Example communication using Python:  
  ```python  
  import serial  

  ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)  
  ser.write(b'CMD:SET_MODE:2
')  # Switch to mode 2  
  response = ser.readline().decode().strip()  # Read response (if any)  
  print("Response:", response)  
  ser.close()
  ```

  ───────────────────────────────────────────────────────────────  
  ▓ LICENSE  
  Open-source, because why not?  

  ───────────────────────────────────────────────────────────────  
  ▓ AUTHOR & CONTACT  
  Developed by [Your Name / Handle]  
  Questions? Bugs? Hit me up at [your contact info]  

  ───────────────────────────────────────────────────────────────  
  ▓ FINAL WORDS  
  This is not just a lamp.  
  This is **СоюзПечаль**.  

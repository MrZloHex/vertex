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
  It communicates with an _obelisk_ via USART,  
  allowing dynamic lighting modes and interactions.  
  Simple. Flexible. Cool as hell.  

  ───────────────────────────────────────────────────────────────  
  ▓ HARDWARE  
  ▪ **MCU**: ATmega328P (Currently Arduino Nano, later standalone)  
  ▪ **OUTPUTS**: LED Strip + Buzzer  
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
  Compile with **AVR-GCC**
  Flash using **AVRDUDE**

  ```sh  
  make &&
  make PORT=/dev/ttyUSB0 flash
  ```

  ───────────────────────────────────────────────────────────────  
  ▓ USART COMMANDS  
  _vertex_ listens for commands over USART to control lighting & sound.  
  Sample command format:  

  ```
  CMD:SET_MODE:1      # Switch to mode 1  
  CMD:BUZZER:ON       # Enable buzzer  
  CMD:POWER_OFF       # Go to low-power mode  
  ```

  ───────────────────────────────────────────────────────────────  
  ▓ FINAL WORDS  
  This is not just a lamp.  
  This is **СоюзПечаль**.  

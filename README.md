# **AccessGuard - Sistema de Controle de Acesso com FreeRTOS**  

[![GitHub](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-v10.4.3-green)
![Platform](https://img.shields.io/badge/Platform-Raspberry%20Pi%20Pico-red)
[![Google Drive](https://img.shields.io/badge/Demo-Google%20Drive-blue?logo=google-drive)](https://drive.google.com/file/d/1g9TPC9uLPvH81IrUvNcO_eSAk6k5pv5d/view?usp=sharing)

Um sistema de controle de acesso baseado em FreeRTOS para monitorar a ocupação de ambientes limitados, com feedback visual (OLED + LED RGB) e sonoro (buzzer).  

---

## **📁 Estrutura do Projeto**  

```
AccessGuard/
├── lib
|   ├── FreeRTOSConfig.h        # Configurações do FreeRTOS
|   ├── buzzer.c                # Implementação do controle do buzzer
|   ├── buzzer.h                # Definições do buzzer
|   ├── font.h                  # Fontes para o display OLED
|   ├── ssd1306.c               # Driver do display SSD1306 (OLED)
|   └── ssd1306.h               # Cabeçalho do driver OLED
├── CMakeLists.txt          # Configuração de compilação CMake
├── main.c                  # Código principal (inicialização e tarefas)
└── .gitignore              # Arquivos ignorados pelo Git
```

---

## **🚀 Funcionalidades**  

✔ **Controle de acesso** com contagem de usuários (entrada/saída)  
✔ **Limite configurável** (até 10 usuários por padrão)  
✔ **Feedback visual** via:  
   - **OLED** (status em tempo real)  
   - **LED RGB** (cores indicando ocupação)  
✔ **Alerta sonoro** (buzzer) em caso de lotação  
✔ **Reset manual** via botão do joystick  
✔ **Multi-tarefa** com FreeRTOS para operação concorrente  

---

## **🛠️ Hardware Necessário**  

- **Placa:** Raspberry Pi Pico  
- **Periféricos (BitDogLab):**  
  - Botões (A, B, Joystick)  
  - Display OLED (SSD1306, 128x64)  
  - LED RGB  
  - Buzzer  

---

## **⚙️ Configuração e Compilação**  

1. **Clone o repositório:**  
   ```bash
   git clone https://github.com/EderRenato/Embartech_Sistema_Acesso.git
   cd AccessGuard
   ```

2. **Configure o ambiente:**  
   - Certifique-se de ter o [SDK da Raspberry Pi Pico](https://github.com/raspberrypi/pico-sdk) instalado.  
   - Configure o `CMakeLists.txt` conforme necessário.  

3. **Compile e flashe:**  
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```  
   - Carregue o arquivo `.uf2` gerado na Pico.  

---

## **🎯 Uso**  

1. **Botão A (Entrada):** Incrementa o contador de usuários (se houver vagas).  
2. **Botão B (Saída):** Decrementa o contador.  
3. **Botão do Joystick (Reset):** Zera a contagem e toca um alarme.  

---

## **📜 Licença**  

MIT License. Consulte o arquivo [LICENSE](LICENSE) para detalhes.  

---


**Desenvolvido por:** Eder Renato 

--- 

🔹 **Contribuições são bem-vindas!** Abra uma *issue* ou envie um *pull request*.

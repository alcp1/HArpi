# HArpi
HAPCAN Home automation for Raspberry Pi!

# Scope
The HAPCAN Home automation for Raspberry Pi - (or **HArpi** for short) is intended to be a central for processing home automation scenes with HAPCAN Systems:
 * Send and Receive messages from/to the CAN Bus (HAPCAN frames) to define scenes;

# Hardware
Same interface used for HMSG. See below:
 * https://github.com/alcp1/HMSG?tab=readme-ov-file#hardware

# Raspberry Pi Setup and Configuration
Follow the same steps found on **HMSG** below. **HArpi** should ideally run on a raspberry pi thar already runs **HMSG**.
 * https://github.com/alcp1/HMSG?tab=readme-ov-file#raspberry-pi-setup-and-configuration

## Compile the HMSG code and run it:
* STEP 1: Go to user folder:
    ```
    cd /home/pi
    ```
    **REMARK:** It is considered that the user is "pi". If not, the folder should be /home/_username_.
* STEP 2: Get files from github
    ```
    git clone --recurse-submodules https://github.com/alcp1/HArpi
    ```
* STEP 3: Compile and Install
    ```
    cd /home/pi/HArpi/SW/
    sudo make all
    ```
* STEP 4: Update the configuration file /home/pi/HArpi/SW/configHArpi.json 
    
    **REMARK:** See the section "Configuration File" for HMSG (https://github.com/alcp1/HMSG?tab=readme-ov-file#configuration-file) for details.
    
* STEP 5: Run the program
    ```
    sudo /home/pi/HArpi/SW/out/HArpi
    ```
## Running the HMSG program at startup and automatically restarting it
* STEP 1: Create a service:
    ```
    sudo nano /lib/systemd/system/harpi.service
    ```
* STEP 2: Edit the service by adding the the lines below to the file:
    ```
    [Unit]
    Description=HArpi Service
    After=multi-user.target
    
    [Service]
    Type=idle
    
    User=pi
    ExecStart= /home/pi/HArpi/SW/out/HArpi
    WorkingDirectory=/home/pi/HArpi/SW/out
    Restart=always
    RestartSec=40
    
    [Install]
    WantedBy=multi-user.target
    ```
**REMARK:** If the user is not pi, this file has to be updated with the correct user and folder.
* STEP 3: Enable the Service:
    ```
    sudo systemctl daemon-reload
    sudo systemctl enable harpi.service
    ```
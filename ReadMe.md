
# Distance Alarm system with UWB & GPS
This repository is distance alarm system using UWB and GPS sensor.  
Check the distance between UWB anchor and UWB tag, Generate Alarm in server.  
Anchor and Tag always transfer thier information to server,  
then server generate alarm using these datas and send it to both.

&nbsp;

## Environment
---
| Name | Content | Model | Number |
|:---------: |:-------:|:-----:|:------:|
| Ubuntu 18.04 | Server | - | 1 |
| Raspberry PI 4 | Anchor & Tag | - | 2 |
| LTE stick | Communication | HUAWEI 4G Wingle E8372 | 2 |
| PI camera | Record Video in Anchor | - | 1 |
| GPS Module | Coordinate | EZ-0048 | 2 |
| UWB sensor | Measure Distance (Anchor & Tag) | DWM 1001-dev | 2 |

&nbsp;

<!-- * GPS sensor * 2 (EZ-0048)  
  ![EZ-0048](https://user-images.githubusercontent.com/62336850/179450284-dd48c133-bc71-4491-add4-de474efa5d80.PNG)
* UWB sensor * 2 (DWM 1001-dev)
  
  ![DWM 1001-dev](https://user-images.githubusercontent.com/62336850/179450432-9a3848ca-dd8e-4f1a-a3a8-55afd3f18df6.jpg)
____
System Model
------------
![System model](https://user-images.githubusercontent.com/62336850/179450396-a6f6b709-835f-4ef4-857f-c6b523a074f3.PNG) -->
_______

&nbsp;

# Requirments
## GPS (EZ-0048)
> Reference : https://wiki.52pi.com/index.php?title=EZ-0048

* Dependencies
  ```bash
    sudo apt-get update
    sudo apt-get -y install gpsd gpsd-clients libgps-dev
  ```
* Activate GPS
  ```bash
    sudo systemctl enable gpsd.socket
    sudo systemctl start gpsd.socket
    sudo cgps -s
  ```

## UWB (DWM 1001-dev)
> Reference : https://www.decawave.com/dwm1001/firmware/


* UWB  

  Initially, the firmware is already installed.  
  how to reintall firmware or install other firmware at Window OS
  > We use modified firmware. If you want to use, check the folder "how to modify UWB firmware"   
  1. install **j-Link**  
      > Site : https://www.segger.com/downloads/jlink/

      ![download](https://user-images.githubusercontent.com/62336850/179666429-779eabd3-1249-48ba-92cf-7e71af5725c8.PNG)
      ![install2](https://user-images.githubusercontent.com/62336850/179667131-b8b6d568-0125-4c91-b4a2-aa6038b35529.PNG)
      ![install2](https://user-images.githubusercontent.com/62336850/179477563-af898bf2-5c0f-4277-8c97-8086ac8b89b4.PNG)
      ![installfinish](https://user-images.githubusercontent.com/62336850/179477684-fe7c39ac-8f29-4a7b-8f47-617467094ff2.PNG)

  2. connect UWB to window OS using USB to 5-pin cable
  
  3. run **jFlashLite**
    ![install](https://user-images.githubusercontent.com/62336850/179666457-2ccc5407-7102-4bfd-8421-32a012b46a68.PNG)
  4. init setup

  ![set chip](https://user-images.githubusercontent.com/62336850/179666469-74efed4d-fe6b-438c-8748-d3f6b2872df0.PNG)
  ![set chip2](https://user-images.githubusercontent.com/62336850/179666478-11f5a287-cb5f-4a4f-9206-74826d0ed922.PNG)
  ![set chip3](https://user-images.githubusercontent.com/62336850/179666487-e9665bec-5fe3-40f9-b2aa-0c7e628db4dd.PNG)

  5. erase firmware
    
      press 'erase chip'

      ![erase chip](https://user-images.githubusercontent.com/62336850/179666497-6f50d24b-6c98-4327-bf12-a4597e1d5a81.PNG)

  6. flash firmware 

      set up the data file(UWB firmware)

          Data File : DWM10001_dwm-simple.hex

      press 'Program Device'


      
  ![flash chip](https://user-images.githubusercontent.com/62336850/179790047-1b62a640-6253-4cc8-ad09-5d303512a482.PNG)

  * set **'Anchor'** and **'Tag'**
    
    * You have to make at least 1 anchor and 1 tag.  

    1. install **tera term VT**
        
       [id]: URL "Optional Title here" 
        Link: [Tera Term VT download][Teralink]

        [Teralink]: https://tera-term.softonic.kr/download "Go tera" 
    2. run **Tera Term VT** and Set **connection method**
        
       ![connection](https://user-images.githubusercontent.com/62336850/179793938-25db3c08-8333-4212-9510-3312bf039ef1.PNG) 
    3. press **Setup/Serial port**

        ![speed](https://user-images.githubusercontent.com/62336850/179794034-a9e980b0-1563-4e41-9bec-62c3fb87f0b8.PNG)
    4. change **Speed: 115200**

       ![speed2](https://user-images.githubusercontent.com/62336850/179794043-19bd38e9-82a8-4a40-bd6f-3b8db0f7085b.PNG) 
    5. set mode('Anchor' or 'Tag')
        
        ### **nmi**: set anchor init(first anchor)

        ### **nma**: set anchor(second anchor, third, ~)

        ### **nmt**: set tag

        ### **?**: you can see other command

        ![set anchor](https://user-images.githubusercontent.com/62336850/179794054-8feafc13-51cf-4281-8f9d-85b53996c106.PNG)
  

&nbsp;

# Usage
## 1. Server (Laptop - Ubuntu 18.04)
```bash
  python3 serv.py ${SERVER_IP} ${ANCHOR_PORT} ${TAG_PORT}

  # ex. python3 serv.py 192.168.0.1 4000 5000
```

## 2. Anchor (Raspberry Pi)
```bash
  ## Terminal 1
  # Install Dependencies
  sudo apt-get -y install gpsd gpsd-clients libgps-dev

  # Activate GPS
  sudo systemctl enable gpsd.socket
  sudo systemctl start gpsd.socket
  sudo cgps -s

  ## Termianl 2
  # Build
  gcc -o clnt_anchor clnt_anchor.c -lm -lgps
  
  # Run
  ./gcc clnt_anchor ${ANCHOR_PORT}

  # ex. ./gcc clnt_anchor 4000
```

## 3. Tag (Raspberry Pi) 
```bash
  ## Terminal 1
  # Install Dependencies
  sudo apt-get -y install gpsd gpsd-clients libgps-dev

  # Activate GPS
  sudo systemctl enable gpsd.socket
  sudo systemctl start gpsd.socket
  sudo cgps -s

  ## Terminal 2
  # Build
  gcc -o clnt_tag clnt_tag.c -lm -lgps

  # Run
  ./gcc clnt_tag ${TAG_PORT}

  # ex. ./gcc clnt_tag 5000
```

## Run Image
![uwb_serv](https://user-images.githubusercontent.com/96454461/179469411-b345cded-7fe6-4ce7-9b80-3df9d436dd81.PNG) 

# Author
Kumoh National Institute of Technology  
- Kwon DaeHyeon : dh0708@kumoh.ac.kr  
- Kim JungMin : 20216029@kumoh.ac.kr
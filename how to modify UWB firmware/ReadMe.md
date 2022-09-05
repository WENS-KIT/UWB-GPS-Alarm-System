How to modify the firmware
===========
if you want to modify the firmware, you have to use the Embedded Studio for ARM.

## overview

1. install Embedded Studio for ARM
   
   A. install Embedded Studio for ARM
   
   B. install j-link
   
   C. install GNU Arm Embedded Toolchain
2. upload firmware
3. modify to .hex file from .bin file
_______

## 1. install Embedded Studio for ARM

   ### A. install Embedded Studio for ARM
  
   Link: [install address][ESAlink]

   [ESAlink]: https://www.segger.com/downloads/jlink/ "Go ESA"
      
   A-1. change the version to V5.42a
   ![ESA install1](https://user-images.githubusercontent.com/62336850/180369345-98379ba9-0bf8-4af5-9163-ca5d94a13241.PNG)

   A-2. press next, finish

   ![ESA install2](https://user-images.githubusercontent.com/62336850/180374594-8fd90105-0b61-4ac0-8b05-2903fc882059.PNG)
   ![ESA install3](https://user-images.githubusercontent.com/62336850/180374600-03765b54-1039-4a6c-874d-437722810998.PNG)
   ![esa install4](https://user-images.githubusercontent.com/62336850/180374603-7bd6d89a-a197-4213-97d1-585a8aa1d149.PNG)
     
   ### B. install **j-Link**
   
   [id]: URL "Optional Title here" 
   Link: [J-link download][jlinklink]

   [jlinklink]: https://www.segger.com/downloads/jlink/ "Go Jlink"

   ![download](https://user-images.githubusercontent.com/62336850/179666429-779eabd3-1249-48ba-92cf-7e71af5725c8.PNG)
   ![install2](https://user-images.githubusercontent.com/62336850/179667131-b8b6d568-0125-4c91-b4a2-aa6038b35529.PNG)
   ![install2](https://user-images.githubusercontent.com/62336850/179477563-af898bf2-5c0f-4277-8c97-8086ac8b89b4.PNG)
   ![installfinish](https://user-images.githubusercontent.com/62336850/179477684-fe7c39ac-8f29-4a7b-8f47-617467094ff2.PNG)

   ### C. install GNU Arm Embedded Toolchain(cross compiler)

   Link: [install address][link]

   [link]: https://launchpad.net/gcc-arm-embedded/+download "Go tool chain"

   ![toolchaingcc](https://user-images.githubusercontent.com/62336850/185858833-70d01b45-ed8c-49e5-a9f2-5830d15bc909.PNG)

   gcc-arm-none-eabi

   [compiler classification]-[Process name executed as a result of compilation]-[OS]-[abi]

   press 'ok', 'install', 'finish'

   ![gcc1](https://user-images.githubusercontent.com/62336850/180402804-7309b86a-51c3-437b-a603-17df80a2b236.PNG)
   ![gcc2](https://user-images.githubusercontent.com/62336850/180402814-13a8d957-e9f8-42f4-b922-ec2071c5db0a.PNG)
   ![gcc3](https://user-images.githubusercontent.com/62336850/180402828-fdd6e6ae-99b1-4aba-b14e-1b03312a6a52.PNG)

   close the editor and terminal

   ![gcc4](https://user-images.githubusercontent.com/62336850/180402835-9aa8a734-1039-4ad6-bd44-8a8b8708390e.PNG)
   ![gcc4-1](https://user-images.githubusercontent.com/62336850/180402842-177a535a-df1a-41d2-89ad-a0ec42f27bc6.PNG)

## 2. upload firmware

   2-1. press 'dwm-simple' in the origin file, then you can see this screen
   ![캡처](https://user-images.githubusercontent.com/62336850/185859401-6f35ec0e-444c-4058-9ed3-d6667248cde1.PNG)

   2-2. press tab Target/connect j-Link

   ![캡처](https://user-images.githubusercontent.com/62336850/185860437-0dc93ee1-3098-4ed7-a15d-2ddced50a700.PNG)
   ![캡처](https://user-images.githubusercontent.com/62336850/185861563-37db6c81-aa43-4321-8ab4-01b79df19c6a.PNG)

   2-3. press tab Target/download dwm-simple

   ![캡처](https://user-images.githubusercontent.com/62336850/185862522-b758be84-4d69-46c7-9b35-09ff16a520e6.PNG)

   2-4. check the Output

   ![캡처](https://user-images.githubusercontent.com/62336850/185863016-68a67c62-bd18-4688-b6ad-f427b8167c1c.PNG)

   The compilation output can be found in the examplesd/dwm-simple/Output directory. If the compilation is successful, then SES will create a compiled image: /dwm-simple/Output/linker/dwm-simple_fw2.bin
   ![캡처](https://user-images.githubusercontent.com/62336850/185865180-54716dd9-e60b-4441-af2f-ea01967bb0cc.PNG)

## 3. modify to .hex file from .bin file
In order to simplify this task and generate a unique hex file, a batch script is provided in the /utilities directory.

![image](https://user-images.githubusercontent.com/62336850/185865861-30e97ce4-b17b-4bea-b9d6-702f36582014.png)

press the "generate_example_hex". It make the bin->hex

![image](https://user-images.githubusercontent.com/62336850/185866220-61d7119c-71ab-43da-9dea-f259c0f07c50.png)

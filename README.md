# wakeduino-server
WakeDuino Server Sketch for Arduino - Sends WOL Magic Packets to devices in your LAN. It is triggered by messages received on a TCP socket.
I needed a lightweight low-energy system to wake devices in my LAN from outside. Didn't want to use my RaspBerry because I wanted to use it as a RetroPie. :D
It has been also very fun to try to make it work considering the limitations of my Arduino UNO.
I used an old Arduino UNO and the official Ethernet Shield v2.
Check also the app made in Flutter that connects to this server: https://github.com/banorz/wakeduino-client

The biggest difficulty I had to overcome was the dynamic memory, which easily filled up and made the code unstable. <br/>
I implemented a JSON config file, but I had to move to more simple solutions, because the JSON parser library I was using was using too much memory making the Arduino crash.<br/>
I implemented a Log system on SD, but I had to disable it because it was filling up the memory and the Arduino was crashing.<br/>
I think that maybe on more powerful Arduinos they can be run, but it needs to be tested and then it can be inserted using conditional code.<br/>

Another issue I had was the use of TCP and UDP contemporarily:<br/>
it seems to be solved, but I am not sure if I solved just by closing the TCP socket before sending the UDP broadcast or using a modified version of the Ethernet library:<br/>
(https://github.com/PaulStoffregen/Ethernet) this one from PaulStoffregen which is fixing a well-known bug in the Ethernet library.<br/>

# Config
The config file should be saved on a SD card, named init.cfg<br/>
In the source code you can find an example
The file structure is the following:<br/>
<br/>
0.0.0.0 <- Ip  \
0 <- Port  \
00:00:00:00:00:00 <- Arduino Ethernet Mac Address  \
SERVERKEYLONGERISBETTER <- Server Password  \
remoteOnly <- Lan Devices Only  \
device1|00:00:00:00:00:00 <- Lan Devices (Optional)  \
device2|00:00:00:00:00:00  \
deviceN|00:00:00:00:00:00 ||| put a new line here (IMPORTANT)\  
<- your caret should be here
<br/>
- <b>Ip:</b> Socket will be opened on this address<br/>
- <b>Port:</b> Arduino will listen on the following port<br/>
- <b>Arduino Ethernet Mac Address:</b> Mac address of the Arduino Ethernet Device (Often is written on a label of the eth shield)<br/>
- <b>Lan Devices Only:</b> if this line contains "remoteOnly" the Arduino will be triggered only if the client sends the devices name stored in Lan Devices<br/>
  if this line is empty (or contains any other text) the Arduino will be triggered also from mac addresses (check Trigger Message)<br/>
- <b>Lan Devices:</b> if Lan Devices Only is true the arduino will load the list inserted here. [deviceName|macAddress]. There is a limitation on how much devices you can load.<br/>
<br/>
If the SD is not inserted the Arduino will load the default settings (you can configure directly in the Sketch)<br/>
<br/>

# Trigger Messages
There are two kinds of trigger messages, one for device, one for mac address.  \
Device trigger: [Server Password]||[Device name as in Lan Devices]  \
MAC trigger: [Server Password]|[Mac Address]  \
  \
If Lan Devices Only is enabled the Arduino will listen only for device triggers.  \
If Lan Devices Only is disabled the Arduino will listen for both types of messages.  \
The Arduino will respond OK to the client only if the message is sent, hiding any operation (from the client perspective) if any error occurs.  \
  \
You can test using a Telnet client connected to the Arduino using PASSIVE mode, just send the message, example:
SERVERPASSWORD||DesktopPC
SERVERPASSWORD|00:11:22:33:44:55

# ToDo
- Test on more Arduinos / Ethernet Shields. Add Conditional Code with more / less functions for more performant / less performant Arduinos.
- Better config file format (JSON? XML? INI?)<br/>
- LED when trigger message is received<br/>
- Alert module (warn if there are too many trigger messages on the socket / DDOS protection)<br/>
- Log module<br/>
- Secure communication<br/>

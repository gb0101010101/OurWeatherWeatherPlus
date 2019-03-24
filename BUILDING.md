1. Download the Arduino IDE (henceforth called the AIDE) from  <https://www.arduino.cc/en/Main/Software>, version 1.6.8 at the time of writing. Be careful to not download from www.arduino.org! See <http://arduino.stackexchange.com/questions/11824/difference-between-arduino-cc-and-arduino-org> for the soppy details

1. Install the ESP8266 addon in the AIDE   <https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon>.   
<http://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/>.  
Open up AIDE, then go to the Preferences (File > Preferences). Then, towards the bottom of the window, copy this URL into the 'Additional Board Manager URLs' text box:

    http://arduino.esp8266.com/stable/package_esp8266com_index.json

    Click 'OK' to save changes and go back to main IDE screen.
    Click 'Tools' -> 'Board' -> 'Board manager' (very top of board list)
    Type 'ESP8266' into filter box, Select version '2.5.0' and click 'Install'.

    Click on 'Tools' -> Manage Libraries'
    Type 'Blynk' into filter and install version 0.6.1 by Volodymyr Shymanskyy.
    Type 'PubSubClient' into filter and install version 2.7.0 by Nick O'leary.

    Here are some introductory tips on using the AIDE: <https://www.arduino.cc/en/Guide/Environment>

1. Clone the forked version of OurWeatherWeatherPlus from the github repo to your local PC.

1. Open the file `SDL_ESP8266_WeatherPlus.ino` in the AIDE.

1. In the AIDE menu system select the 'Adafruit Feather Huzzah ESP8266' in the Tools/Board menu. SDL advises that this board setting best fit the design of the WeatherPlus board.

1. In the AIDE menu system select 'Sketch' -> 'Verify/compile' to compile the code.

1. If it compliles without errors then hook up the board to USB. Select the USB port in 'Tools' -> 'Port' and upload using 'Sketch' -> 'Upload'. Make sure to put board in flash mode! 

1. The binary can be exported using the AIDE menu system select 'Sketch / Export Compiled Binary'.

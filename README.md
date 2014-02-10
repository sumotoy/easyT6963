easyT6963
=========

Library for T6963 displays with a GPIO (only 4 wires)<br>
![image](http://i1189.photobucket.com/albums/z437/theamra/libraries/CIMG5833.jpg "T6963C_1")<br>

Now Beta and fully working, lot of speedup, mixed (text&Graphic) environment working. Check the examples...<br>

http://www.youtube.com/watch?v=JvwkEgbJaDo

<b>ToDo:</b> 
 - Graphic Font rendering can be faster. Next release will not render blank pixels but a routine will fast erase the string placeholder and render routine will fill just needed pixels, this will fast a lot rendering.
 - Some graphic low level routine can be a bit faster.


<b>T6963</b> it's a graphic controller from Toshiba mounted usually on few large graphic displays. Unfortunatly it needs a lot of wires, from 14 to 16, and it's not well supported for MCU like Arduino, Teensy, DUE, etc.<br>
I google it and find some library but mainly only 2 was (almost) works:<br>
<b>U8glib</b> https://code.google.com/p/u8glib/<br>
<b>T6963_Lib</b> http://code.google.com/p/arduino-t6963c/<br>
The first one it's really basic and have no DUE or Teensy3 support, second one was pretty interesting but it's quite old and uses hardware mapped pins, plus have some bug.<br>
Since I have some of those displays (mainly 240x128) I decided to wrote a library with these goals:<br>

 - Uses just 4 wires.
 - Working all my MCU's without changes.
 - Must able to use the features of this chip.

Since I need to uses only 4 wires, I'm enforced to use a GPIO chip. I choosed my preferred one, <b>MCP23s17</b> from Microchip and since I wroted already a library for it (here in my GITHUB,https://github.com/sumotoy/gpio_expander) a part of the job it's done, yo!<br>
<b>T6963</b> it's quite strange chip, not particular fast and Toshiba's datasheet it's a confused messy of informations in bad english, pure Japanese attempt to give as less infos as possible, so was not easy but thanks a guy called Radosław Kwiecień that spent a lot of time writing his T6963_Lib I was able to grab part of his cede for this one so thanks a lot Radosław, nice you shared!<br>
One good thing of this chip is the ability to uses graphic and text separately in different memory pages an some un-usual command give the ability to switch by adding reverse or blink, it has some other features that give some speed in visualization and only Radosław was able to findout (U8glib it's really raw) but unfortunatly he never finish the library and left some unworking parts of code, so I need to write many part from scratch and replace completely the text routines.<br>
Another problem is the negative voltage needed from almost all those displays (take a look in the display datasheet first!) that need to be from -9v to -15v, this is not easy when you only have 5V for your MCU!<br>
I fixed the negative voltage problem with a microchip IC MC34063 that uses a couple of Jaf but you can use any step-up negative supply since the current needed it's really tiny!<br>
My displays was using High Voltage for backlight so I need to build up another stepUp supply for that! Take a look in LCD datasheet and check<br>
The Pro and Cons of this chip are so far:<br>

 <b>PRO:</b><br>
 - Ability to drive large displays thanks byte addressing and internal shadow ram.
 - Ability to works with mixed Graphics and Text, it's a mix of Char LCD and Graphics one.
 - Hardware drived paged display with some effects.
 - Ability to perform animations thanks to fast hardware drived paging (kinda)
 - Very fast when byte transfer used (only)

 <b>CONS:</b><br>
 - Not particular fast. Single pixel addressing it's one of the slowest in graphic LCD's market.
 - It need to read chip state before each command/data transmit.
 - Commands are clumsy and documentation not help at all.
 - Initialization it's slow.
 - Clear graphic screen slow, in contrast clear text screen it's fast.


Here's the pin connection with GPIO:<br>
<table>
<tr>
<td><b>LCD Pin</b></td><td><b>MCP23S17 PIN</b></td>
</tr>
<tr>
<td><b>D0</b></td><td>I/O:0</td>
</tr>
<tr>
<td><b>D1</b></td><td>I/O:1</td>
</tr>
<tr>
<td><b>D2</b></td><td>I/O:2</td>
</tr>
<tr>
<td><b>D3</b></td><td>I/O:3</td>
</tr>
<tr>
<td><b>D4</b></td><td>I/O:4</td>
</tr>
<tr>
<td><b>D5</b></td><td>I/O:5</td>
</tr>
<tr>
<td><b>D6</b></td><td>I/O:6</td>
</tr>
<tr>
<td><b>D7</b></td><td>I/O:7</td>
</tr>
<tr>
<td><b>CE</b></td><td>I/O:8</td>
</tr>
<tr>
<td><b>CD</b></td><td>I/O:9</td>
</tr>
<tr>
<td><b>WR</b></td><td>I/O:10</td>
</tr>
<tr>
<td><b>RD</b></td><td>I/O:11</td>
</tr>
<tr>
<td><b>FS</b></td><td>I/O:12 (see note 1)</td>
</tr>
<tr>
<td><b>RV</b></td><td>I/O:13 (see note 2)</td>
</tr>
<tr>
<td><b>RST</b></td><td>I/O:14</td>
</tr>
<tr>
<td><b>Backlight</b></td><td>I/O:15 (see note 3)</td>
</tr>
</table>

 - <b>Note 1</b>: FS pin is not present in all LCD's. If not, ignore it.
 - <b>Note 2</b>: RV pin is not present in all LCD's. If not, ignore it.
 - <b>Note 3</b>: Backlight pin should be careful used since latge displays use a lot of current and MCP23s17 cannot handle more that a couple of leds. I will include a simple schematic for an universal background driver that use a reed relay, this allow you to use almost every LCD background (even mine that uses High Voltage), for the experiment ignore this pin and connect your Background as your datasheet suggest.<br>

The remaining MCP23s17 pins are of course GND, VDD, the MOSI,MISO,SCK and CS pins and 3 address pin and a reset pin (should be connected to VDD).<br>
The address pins are really important since you can use the HAEN feature of this chip that let you share 8 of these GPIOs by sharing the same SPI lines, included CS!<br>
For a quick test, tie all address to GND that means address 0x20!<br>
I would raccomand attention using large displays on small memory MCU like UNO, I tried to use the internal CGRAM as much I can but if you plan to use external fonts or bitmaps you can reach the limits of those small cpu's!<br>
Library it's modular, this mean that low hardware routines have been splitted to simply use any GPIO you like or even direct connections!<br>
This is an experiment, it works (i made a video that proof it) but it's far to be finished, I did quite optimizations to get as fast I can but if you plan to use a I2C GPIO, simply forget it!<br>
![image](http://i1189.photobucket.com/albums/z437/theamra/libraries/CIMG5832.jpg "T6963C_1")<br>

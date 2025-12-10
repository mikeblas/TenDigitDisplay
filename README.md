Here is a project I made in the summer of 2025.

On a driving trip from Chicago back home to Seattle, I discovered [American Science and Surplus](https://sciplus.com/). Their store in Geneva, Illinois has lots of fun stuff, including some two-digit LED displays that I thought I could use to make a neat display.

The circuit in this project multiplexes the displays so they can be driven by the ESP-32. After a few iterations on the breadboard, I got boards done at [OSH Park](https://oshpark.com/) and soldered them up with through-hole parts that I mostly had in my inventory.

The web server on the ESP-32 has two endpoints: one takes a ten-digit hex string, which is a bitmap of the segments to illuminate on the display. The other takes a string of numbers to display -- they're converted from ASCII to their seven-segment representation.

I want to use the project to display some stock prices as they change. Since there's just seven segments, it's a bit crude, but something like "183.78d 1.19" means that the issue I'm tracking is at $183.78 per share, down (for "d") $1.19 per share since the last close.

Right now, all the smarts are in a Python scripts that run on a desktop computer. That script pokes at those endpoints and the ESP-32 just scans the multiplexed displays to show the last thing it was told. Eventually, the device could be standalone, though that comes with a few problems that I'll need to sort out.

Things to do:

* Make the Python script support multiple devices
* Show the DHCP-assigned IP address of the ESP-32 on the LEDs at startup
* Have the ESP-32 beacon a UDP message for automatic discovery by client scripts
* Smarter endpoints: accept characters and decimal points, for example
* Use the on-board LED to indicate stale data -- if no client has made an update in some time

And probably a few more things. But I'm also thinking about 16-segment display with 2 rows of 20 characters for a bit more fidelity ...

This repo contains these directories:

* `assets`: some pictures for the readme
* `TenDigitPCB`: a KiCAD project (including shape and footprint files) for the PCB and its components
* `TenDigitsIDF`: Visual Studio Code project for the ESP-32 IDF to implement the web server and multiplexing
* `TenDigitQuoter`: A Python project (for PyCharm) that shows quotes for a stock from the Yahoo! Finance API

Here are some pictures:

![Front view of populated PCB](assets/PopulatedPCBObverse.png?raw=true)

![Reverse view of bare PCB](assets/BarePCBReverse.png?raw=true)

![Quote being displayed](assets/DisplayQuote.png?raw=true)

![PCB in KiCAD designer](assets/PCBDesigner.png?raw=true)

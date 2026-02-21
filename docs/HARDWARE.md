[The case](#the-case)

[The controller](#the-controller)

[The display](#the-display)

[The battery](#the-battery)

[The reading light](#the-reading-light)

[Other hardware and mounting](#other-hardware-and-mounting)

## The case:
As mentioned elsewhere, this case started life as a display case for a pen and a calculator. It is a felt covered stamped steel case, with a spring-loaded hinge which holds it in either closed or approx. 120° open position. It is about 18cm long, 11 cm wide, and 2.2cm thick. I fitted a microswitch/limit switch that cuts battery power when the case is closed, eliminating any battery drain. While a case exactly like it may be hard to find, I think the basic layout has several advantages:
1. The size and shape is approximately that of a paperback. It is small enough that it can be held at a comfortable reading distance for an extended time, while allowing a useful screen size.
2. The cover means that it opens up like a book.
3. The cover protects the screen while not in use.
4. The cover provides a place to mount a reading light. E-paper is low power and sunlight readable, but in dim light it requires a light source.
5. It provides the means to automatically shut off battery power when not in use, as described above. You could of course fit a separate power switch to do the same, but that means always remembering to turn off when you stop reading.

The material it is made of doesn't really matter. In fact a plastic case would be slightly easier to work with as you wouldn't have to worry about shorts through the case. For anyone with a 3D printer, it might be worth considering a 3D printed case, incorporating the points mentioned above. I would be very glad to hear of any case solutions anybody comes up with.

## The controller:
The Adafruit RP2040 feather ThinkInk dev board ([Adafruit product# 5727](https://www.adafruit.com/product/5727) is at the heart of the reader. It features the same RP2040 microcontroller as in Raspberry's own Pi Pico, a charging circuit for a LiPo battery, and an interface for a 24-pin e-paper display.

## The display:
The e-paper display is also supplied by Adafruit, the 5.83" monochrome e-paper, [product# 6397](https://www.adafruit.com/product/6397). This is the first time I work with an e-paper display, and I was somewhat disappointed at first, due to the almost 5 seconds update time and flashing black and white while doing so, but on further review this seems to be par for the course with this class of e-paper displays. I have found displays of this class with claimed update time of 3.5 seconds. There are also displays that use a different connection that claim less than one second update time, but that would require a different type of interfacee and maybe a more powerful controller. I should say though that the reader is IMO quite usable with this display, just that the experience could be smoother. Another option might be a TFT display, but that would sacrifice sunlight readability and, due to the backlight, some battery life.

## The battery:
The battery was sourced locally, what with transport restrictions. It's a 2000mAh LiPo pouch cell, with protection circuitry. That last bit is important, lest your battery is quickly damaged or even becomes a fire hazard! The capacity is more than is needed, but does provide a nice long runtime, even with the reading light on. It's worth noting that the charge current provided by the controller is, at 200mA, targeted at smaller batteries. With this one, over 10 hours may be expected for fully charging from flat. Mind you, I've set a fairly high low voltage cutoff point (3.4V), so it should never be run completely flat. What with that and the low charge current, the battery is treated quite gently.

## The reading light:
As mentioned before, the e-paper display does not have or need a backlight, but it does need a decent ambient illumination. When that is not present, a reading light is required. With this design, an almost perfect placement for such a light is inside the lip of the cover of the reader. My first thought was a section of LED strip, and that is still an option. However, these are generally intended for 12V operating voltage and would thus need a boost regulator. Not an unsurmountable obstacle, but then i came across another Adafruit product, the LED sequin ([product# 1758](https://www.adafruit.com/product/1758), warm white version). These are simply a surface mount LED and resistor on a tiny PCB. They are intended to be sewn with conductive thread to textiles for decoration, but there is nothing that prevents you lining them up and soldering to a couple of wires for a makeshift low voltage LED strip. I wasn't sure how bright they would be so I used ten of them, which is overkill. I am running them at 38% with PWM and that's plenty bright, so four of them at 100% should work. Theyre quoted as drawing about 5mA each from a 3.3V source. That would mean 50mA for the ten of them, which is a tall order for a microcontroller io pin. I am therefore using a simple BJT transistor drive circuit to run them directly from the battery.

## Other hardware and mounting:
I'm using an Adafruit Perma-proto ([product# 1606](https://www.adafruit.com/product/1606)) as a carrier board. This is a high quality perfboard laid out like a solderless breadboard. It carries the controller, a micro-SD breakout board ([Adafruit product# 4682](https://www.adafruit.com/product/4682)), a set of eight tactile buttons ([Adafruit product# 1490](https://www.adafruit.com/product/1490)) for operating the reader, the reading light drive circuit, and a voltage divider for battery monitoring.

This in turn is mounted to a faceplate made from diffusion acrylic ([Adafruit product 4749](https://www.adafruit.com/product/4749)). This is a special type of acrylic intended to let LED indicators shine through, while being dark enough to hide one's messy wiring.

The display is fixed on top of the faceplate with double sided tape, while the carrier board is fixed underneath the faceplate with screws and standoffs, and located so that the buttons come through holes drilled in the faceplate, directly below the display, and so that the LED indicators on the controller are visible through the faceplate. The display cable is far too short to reach the controller when located like this, so an extention cable ([Adafruit product# 4230](https://www.adafruit.com/product/4230)) and interconnect ([Adafruit product# 4524](https://www.adafruit.com/product/4524)) are needed.

The faceplate is in turn fixed to the case with screws and standoffs. The height of the faceplate is sufficient for the USB connector of the controller and the micro-SD slot to be accessible above the bottom half of the case. To make this work I had to remove the battery and Stemma QT connectors from the controller. The battery can be connected through the header pins, and the Stemma connection is unused.

The reading light is fixed to the lip of the case lid with double-sided tape.

The battery is fixed to the bottom half of the case with double-sided tape, underneath the faceplate.

Also underneath the faceplate is the power switch ([Adafruit product# 819](https://www.adafruit.com/product/819), mounted with screws and a small wooden block to the bottom half of the case, and located so that one of the springs that keep the case shut/open operates it.

# Mimisbrunnur
For some years now I've been considering joining the 21'st century and get an e-reader. One day when I was sorting through some old junk at my family home, I came across this presentation case (from the early 80's I think) that used to hold a Parker ball point pen and a small calculator. With the pen lost and the calculator broken, I was about to toss it when a thought struck me; this would make a rather nice case for an e-reader. Thus was Mimisbrunnur born.

Needless to say, this is a very back-to-front approach to any project. A reasonable person, well, a reasonable person would just have bought any of the number of commercial offerings out there. That would have brought a slick and polished user interface, and with a bit of forethought wouldn't have locked one into any particular ecosystem.

A slightly less reasonable person would have considered system requirements, existing software and best hardware choices, with the actual case probably being the list item on the list.

And then there is me. Actually I did give a cursory glance at the above, but I was rather besotted with that case. Anyway, I don't expect anyone will mimic this project in every detail (the case is not an off-the-shelf item today, that I know of), but I hope some parts of it may be of some use to others.

[The case](#the-case)
[The controller](#the-controller)


## The case:
As mentioned above, this case started life as a display case for a pen and a calculator. It is a felt covered stamped steel case, with a spring-loaded hinge which holds it in either closed or approx. 120° open position. It is about 18cm long, 11 cm wide, and 2.2cm thick. I fitted a microswitch/limit switch that cuts battery power when the case is closed, eliminating any battery drain. While a case exactly like it may be hard to find, I think the basic layout has several advantages:
1. The size and shape is approximately that of a paperback. It is small enough that it can be held at a comfortable reading distance for an extended time, while allowing a useful screen size.
2. The cover means that it opens up like a book.
3. The cover protects the screen while not in use.
4. The cover provides a place to mount a reading light. E-paper is low power and sunlight readable, but in dim light it requires a light source.
5. It provides the means to automatically shut off battery power when not in use, as described above. You could of course fit a separate power switch to do the same, but that means always remembering to turn off when you stop reading.

The material it is made of doesn't really matter. In fact a plastic case would be slightly easier to work with as you wouldn't have to worry about shorts through the case. For anyone with a 3D printer, it might be worth considering a 3D printed case, incorporating the points mentioned above. I would be very glad to hear of any case solutions anybody comes up with.

## The controller:
The Adafruit RP2040 feather ThinkInk dev board is at the heart of the device. It features the same RP2040 microcontroller as in Raspberry's own Pi Pico, a charging circuit for a LiPo battery, and an interface for a 24-pin e-paper display.

## The display:
The e-paper display is also supplied by Adafruit, the 5.83" monochrome e-paper, product# 6397. I was somewhat disappointed with this, but as this is the first time I work with e-paper displays, maybe my expectations were just too high. I was aware that e-paper takes a fairly long time to update, and f.x. that one should forget about video on them. Nevertheless, almost 5 seconds to update the display, and it flashing black and white while doing so, was a disappointment. I should say though that it is IMO quite usable, just that the experience could be smoother. Possibly other e-paper displays are more responsive. It might also be worth considering a TFT display instead. That would though sacrifice sunlight readability and, due to the backlight, battery life.

## The battery:
The battery was sourced locally, what with transport restrictions. It's a 2000mAh LiPo pouch cell, with protection circuitry. That last bit is important, lest your battery is quickly damaged or even becomes a fire hazard. The capacity is more than is needed, but does provide a nice long runtime, even with the reading light on. It's worth noting that the charge current provided by the controller is, at 200mA, targeted at smaller batteries. With this one, over 10 hours may be expected for fully charging from flat. Mind you, I've set a fairly high low voltage cutoff point (3.4V), so it is never run completely flat. What with that and the low charge current, the battery is treated quite gently.

## The reading light:
As mentioned before, the e-paper display does not have or need a backlight, but it does need a decent ambient illumination. In cases where that is not present, a reading light is what you need. With this design, an almost perfect placement for such a light is inside the lip of the cover of the device. My first thought was a section of LED strip, and that is still an option. However, these are generally intended for 12V operating voltage and would thus need a boost regulator. Not an unsurmountable obstacle, but then i came across another Adafruit product, the LED sequin (product# 1758, warm white version). These are simply a surface mount LED and resistor on a tiny PCB. They are intended to be sewn with conductive thread to textiles for decoration, but there is nothing that prevents you lining them up and soldering to a couple of wires for a makeshift low voltage LED strip. I wasn't sure how bright they would be so I used ten of them, which is overkill. I am running them at 38% with PWM and that's plenty bright. Theyre quoted as drawing about 5mA each from a 3.3V source, which means more like 10mA from the ca. 4V battery voltage which I use to drive them, so 100mA+ for all ten of them. That's more than any microcontroller can be expected to handle, so I'm using a simple BJT transistor circuit to drive them.

## Other hardware and mounting:
I'm using an Adafruit Perma-proto board as a carrier board. This is a high quality perfboard laid out like a solderless breadboard. This carries the controller, a micro-SD breakout board, a set of eight tactile buttons for operating the device, the reading light drive circuit, and a voltage divider for battery monitoring.

This in turn is mounted to a baseplate made from diffusion acrylic. This is a special type of acrylic intended to let LED indicators shine through, while being dark enough to hide one's messy wiring.

The display is fixed on top of the baseplate with double sided tape, while the carrier board is fixed underneath the baseplate with screws and standoffs, and located so that the buttons come directly below the display, and the charge and general indicators are visible through the baseplate. The display cable is far too short to reach the controller when located like this, so an extention cable and interconnect are needed.

The baseplate is also fixed to the case with screws and standoffs. The height that the baseplate is fixed at is sufficient for the USB connector of the controller and the micro-SD slot to be accessible above the bottom half of the case. To make this work I had to remove the battery and Stemma QT connectors from the controller. The battery is connected through the header pins, and the Stemma connection is unused.

The reading light is fixed to the lip of the case lid with double-sided tape.

The battery is fixed to the bottom half of the case, also with double-sided tape, underneath the baseplate.

Also underneath the baseplate is the power switch, mounted with screws to the bottom half of the case, and located such that one of the springs that keep the case shut/open operates it.

## The name:
In Ásatrú, the old norse mythology, Mímisbrunnur is a well or fount of wisdom located at the roots of the world tree Yggdrasill. An e-reader can, in a good light and with a following wind, be considered, if not a fount then a vessel of knowledge, hence the name.



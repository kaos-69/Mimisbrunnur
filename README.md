# Mimisbrunnur
Yet another e-reader

For some years now I've been considering joining the 21'st century and get an e-reader. One day I was sorting through some old junk at my family home, I came across this presentation case (from the early 80's I think) that used to hold a Parker ball point pen and a miniature calculator. With the pen lost and the calculator broken, I was about to toss it when a thought struck me; this would make a rather nice case for an e-reader. Thus was Mimisbrunnur born.

Needless to say, this is a very back-to-front approach to any project. A reasonable person, well, a reasonable person would just have bought any of the number of commercial offerings out there. That would have brought a slick and polished user interface, and with a bit of forethought wouldn't have locked one into any particular ecosystem.

A slightly less reasonable person would have considered system requirements, existing software and best hardware choices, with the actual case probably being the list item on the list.

And then there is me. Actually I did give a cursory glance at the above, but I was rather besotted with that case. Anyway, I don't expect anyone will mimic this project in every detail (the case is not an off-the-shelf item today, that I know of), but I hope some parts of it may be of some use to others.

The case:

As mentioned above, this case started life as a display case for a pen and a miniature calculator. It is a felt covered stamped steel case, with a spring loaded hinge wich holds it in either closed or approx. 120° open position. It is about 18cm long, 11 cm wide, and 2.2cm thick (external measurements, excluding slight padding on the lid). I fitted a microswitch/limit switch that cuts battery power when the case is closed, eliminating any battery drain. While a case exactly like it may be hard to find, I think the basic layout has several advantages:
1. The size and shape is approximately that of a common paperback. It is small enough that it can be held at a comfortable reading distance for an extended time, while allowing a useful screen size.
2. The cover means that it opens up like a book.
3. The cover protects the screen while not in use.
4. The cover provides a place to mount a reading light. E-paper is low power and sunlight readable, but in dim light it requires a light source.
5. It provides the means to automatically shut off battery power when not in use, as described above. You could of course fit a separate power switch to do the same, but that means always remembering to turn off when you stop reading.

The material it is made of doesn't really matter. In fact a plastic case would be slightly easier to work with as you wouldn't have to worry about shorts through the case. For anyone with a 3D printer, it might be worth considering a 3D printed case, incorporating the points mentioned above. I would be very glad to hear of any case solutions anybody comes up with.

The controller:

The Adafruit RP2040 feather ThinkInk dev board is at the heart of the device. It features the same RP2040 microcontroller as in Raspberry's own Pi Pico, a charging circuit for a LiPo battery, and an interface for a 24-pin e-paper display.

The display:

The e-paper display is also supplied by Adafruit, the 5.83" monochrome e-paper, product# 6397. I was somewhat disappointed with this, but as this is the first time I work with e-paper displays, maybe my expectations were just too high. I was aware that e-paper takes a fairly long time to update, and f.x. had read that one should forget about video on them. Nevertheless, almost 5 seconds to update the display, and it flashing black and white while doing so, was a disappointment. I should say though that it is IMO quite usable, just that the experience could be smoother. Possibly other e-paper displays are more responsive. It might also be worth considering a TFT display instead. That would though sacrifice sunlight readability and, due to the backlight, battery life.

The battery:

The battery was sourced locally, what with transport restrictions. It's a 2000mAh LiPo pouch cell, with protection circuitry. That last bit is important, lest your battery is quickly damaged or even becomes a fire hazard. The capacity is more than is needed, but does provide a nice long runtime, even with the reading light on. It's worth noting that the charge current provided by the dev board is, at 100mA, targeted at smaller batteries. With this one, over 20 hours may be expected for fully charging from flat. Mind you, I've set a fairly high low voltage cutoff point (3.4V), so it is never run completely flat. What with that and the low charge current, the battery is treated quite gently.

The reading light:



Other hardware:

Perma proto. Sd card breakout. Buttons. Microswitch. Reading light drive. Base plate.

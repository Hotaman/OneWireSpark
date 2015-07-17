Well, I finally got two Photons (7/16/2015), still not sure where the other two I ordered are and no word yet from Particle on them. All 4 of my dev Cores have been having connectivity issues (no love). Now on with the development! I'll post some more samples for the devices I have and I can now test out the DS1820 example on the Photon (I have some in stock now). I've personally only tested this code with the 1-wire battery monitor noted in the example. The hardware interface works fine but you have to handle the specifics for your 1-wire device of choice for your application. My current project uses this lib so it will be getting some love RSN.


OneWireSpark
============

Provides support for the Dallas One Wire protocol on the Spark core.

I have made minor tweeks to the work of many others and collected it
here for use within the web build tool.

Simply click the libs button in the web build tool at spark.io
and add this lib to your project.

Refer to the ReadTemp.ino example code for basic use of the lib.

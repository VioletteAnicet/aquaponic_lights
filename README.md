# aquaponic_lights

# THE AUTONOMOUS AQUAPONIC PLATFORM

## Implementation of a light sensor and processing of the collected data with InfluxDB

Because the aquaponic platform is a system that aim to be replicated in any indoor environment, controlling the ambient light and being able to adjust it so that the plants can grow at their best in any environment (place with little sun, place subject to weather variations, etc.), is an essential element of research

You can find here **how to implement an arduino code that allows to get the ambient brightness perceived by a photoresistor and send to the leds a light power to compensate. But also how to retrieve these data in the InfluxDB databse in order to have a graphical representation.**

You can find the importation of the libraries needed in the platformio.ini file

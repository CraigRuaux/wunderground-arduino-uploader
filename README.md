wunderground-arduino-uploader
=============================

Inspired by the Sparkfun WIMP weather station project, this code is for an Arduino application that receives weather data from the now-defunct Sparkfun Weather Board (USB Weather Board V3) via XBee, validates the data and then uploads it to Weather Underground 

=======


Unlike the Sparkfun WIMP weather station, this implementation calculates dewpoint on the instruments, and uses time from the NTP servers to determine UTC and do midnight resets.

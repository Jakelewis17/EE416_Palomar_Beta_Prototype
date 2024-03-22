# EE416_Palomar_Beta_Prototype
This patient monitor was built as a 2023/24 CAPSTONE project at Washington State University (WSU) in conjunction with Philips by Team Palomar:
- [Jake Lewis](https://github.com/Jakelewis17)
- [Zack Harrington](https://github.com/ZackHarrington)
- [Matthew Stavig](https://github.com/matthewstavig7)
- [Nick Gerth](https://github.com/Sythianis)

## Functionality
This device was developed as a patient monitor for telehealth applications. It consists of ECG, SpO2, and blood pressure circuitry, with internet connection capabilities. Therefore the device can safely and accurately measure a patient's heartrate, heartbeat waveforms, blood oxygen content, and blood pressure, from any location with power and internet access. 

## Hardware
The hardware for this product consists of:
- An assembled printed circuit board (PCB)
- A blood pressure pump, release valve, and connector for an external cuff
- A 3D printed enclosure
- SpO2 and motor driver breakout boards
- A connection for external ECG electrodes

The specifications this product meets through hardware as required by Philips include:
- 1500V electrical isolation for the ECG electrodes
- 0.5-100Hz bandwidth for measuring ECG signals
- 20dB rejection of 60Hz line power
- Continuous 2.4Ghz Wi-Fi connection
- Device control via an integrated mobile application

Images of the design can be seen below

## Software
The software for this product consists of:
- A webserver
- A mobile application for patient control over the device and measurements
- Digital signal processing code
- Control over measurements

The specifications this product meets through software as required by Philips include:
- A robust webserver storing measurement data for practitioners to access
- Device control via an integrated mobile application
- Digital signal processing of ECG, SpO2, and blood pressure measurements

## License
This product is not licensed, however it was developed in an educational setting in partnership with Philips. Any use of this product is thus subject to the policies of [Washington State University](https://policies.wsu.edu/) and [Philips](https://www.philips.com/a-w/about/investor-relations/governance/business-principles).

## Acknowledgments
Thanks to Scott Schweizer from Philips for the guidance and insight while developing this project.
Additional credits to:
- Dr. Mohammad Torabi, Ph.D. Electrical Engineering
- Keaton Schweizer from Philips
- Sparkfun tutorials

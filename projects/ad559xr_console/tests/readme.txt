This document is to be used to set up the AD5592R/AD5593R hardware for testing.

Connect I/O 5 and 6 on the AD5592R/93R.	
A test is run to estabish if I/O5 can be set as GPO and I/O6 as a GPI. 
The test sets GPO on I/O5 high and I/O6 should read high. 
To perform this test, test points I/O 5 and I/O 6 on the AD5592R/93R should be connected using jumper cables.	


AD5592R:
The AD5592R should be connected to the SDP-K1 using the 120 pin connector and requires no additional jumper cables.
It will receive power from the SDP-K1 provided the jumper on the K1 is set to 3.3V IO output.	
Please ensure the links are configured as follows:
Links:
LK11: A
LK1: B


AD5593R:
The AD5593R uses jumper cables the arduino headers to connect to the SDP-K1. 
The AD5593R can be powered by connecting 5V and from the Arduino header to J2. 
Please ensure the links are configured as follows:
Links:
LK11: C/B	
LK1: B	

For more details and photos for connecting power on the AD5592R/93R the wiki page:	
https://wiki.analog.com/resources/tools-software/product-support-software/ad5592r_mbed_example	






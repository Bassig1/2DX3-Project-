/*  Time of Flight for 2DX4 -- Studio W8-0
                Code written to support data collection from VL53L1X using the Ultra Light Driver.
                I2C methods written based upon MSP432E4 Reference Manual Chapter 19.
                Specific implementation was based upon format specified in VL53L1X.pdf pg19-21
                Code organized according to en.STSW-IMG009\Example\Src\main.c
                
                The VL53L1X is run with default firmware settings.


           Gurjas Bassi, 400062217 Final Project Code using studio 8 as its template
					 Assigned Bus Speed = 80 MHz
					 Measurement Status PN0, Additional Status PN1 This corresponds to LED D2 and D1 respecitivley,
					 PN0 is for the UART measurements and PN1 is the status that the motor has done 45 degree turn since the last blink of this LED

*/
#include <stdint.h>
#include "PLL.h"  //The clockspeed was changed in this .h file 
#include "SysTick.h"  //in systick.c the 10ms delay was changed due to the assigned speed being 80 MHz
#include "uart.h"
#include "onboardLEDs.h"
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"
#include <stdbool.h>   // this is included for the bool condition for the motor to be running 
#include <string.h>
#include <stdio.h>


#define I2C_MCS_ACK             0x00000008  // Data Acknowledge Enable
#define I2C_MCS_DATACK          0x00000008  // Acknowledge Data
#define I2C_MCS_ADRACK          0x00000004  // Acknowledge Address
#define I2C_MCS_STOP            0x00000004  // Generate STOP
#define I2C_MCS_START           0x00000002  // Generate START
#define I2C_MCS_ERROR           0x00000002  // Error
#define I2C_MCS_RUN             0x00000001  // I2C Master Enable
#define I2C_MCS_BUSY            0x00000001  // I2C Busy
#define I2C_MCR_MFE             0x00000010  // I2C Master Function Enable

#define MAXRETRIES              5           // number of receive attempts before giving up
void I2C_Init(void){
  SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;           													// activate I2C0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;          												// activate port B
  while((SYSCTL_PRGPIO_R&0x0002) == 0){};																		// ready?

    GPIO_PORTB_AFSEL_R |= 0x0C;           																	// 3) enable alt funct on PB2,3       0b00001100
    GPIO_PORTB_ODR_R |= 0x08;             																	// 4) enable open drain on PB3 only

    GPIO_PORTB_DEN_R |= 0x0C;             																	// 5) enable digital I/O on PB2,3
//    GPIO_PORTB_AMSEL_R &= ~0x0C;          																// 7) disable analog functionality on PB2,3

                                                                            // 6) configure PB2,3 as I2C
//  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00003300;
  GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0xFFFF00FF)+0x00002200;    //TED
    I2C0_MCR_R = I2C_MCR_MFE;                      													// 9) master function enable
    I2C0_MTPR_R = 0b0000000000000101000000000111011;                       	// 8) configure for 100 kbps clock (added 8 clocks of glitch suppression ~50ns)
//    I2C0_MTPR_R = 0x3B;                                        						// 8) configure for 100 kbps clock
        
}

//The VL53L1X needs to be reset using XSHUT.  We will use PG0
void PortG_Init(void){
    //Use PortG0
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;                // activate clock for Port N
    while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R6) == 0){};    // allow time for clock to stabilize
    GPIO_PORTG_DIR_R &= 0x00;                                        // make PG0 in (HiZ)
  GPIO_PORTG_AFSEL_R &= ~0x01;                                     // disable alt funct on PG0
  GPIO_PORTG_DEN_R |= 0x01;                                        // enable digital I/O on PG0
                                                                                                    // configure PG0 as GPIO
  //GPIO_PORTN_PCTL_R = (GPIO_PORTN_PCTL_R&0xFFFFFF00)+0x00000000;
  GPIO_PORTG_AMSEL_R &= ~0x01;                                     // disable analog functionality on PN0

    return;
}
void PortM0_Init(void){ //This is for both pushbuttons which were physical 
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R11;                 // Activate the clock for Port M
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R11) == 0){};        // Allow time for clock to stabilize 
	GPIO_PORTM_DIR_R = 0b00000000;       								      // Make PM0 and PM1 inputs 
  GPIO_PORTM_DEN_R = 0b00000011;   // this is for the pushbutton required to start the motor and data 
	return;
}


void PortE0_Init(void){	 //this is for the polling method of buttons to determine if a button was pressed
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;		              // Activate the clock for Port E
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R4) == 0){};	        // Allow time for clock to stabilize
  
	GPIO_PORTE_DIR_R = 0b00000011;
	GPIO_PORTE_DEN_R = 0b00000011;                        		// Enable PE0 and PE1 as digital outputs
	return;  // This pe0 is also for the pushbutton using studio 5 as its refrence although onboard buttons could have been used
	}


//XSHUT     This pin is an active-low shutdown input; 
//					the board pulls it up to VDD to enable the sensor by default. 
//					Driving this pin low puts the sensor into hardware standby. This input is not level-shifted.
void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R |= 0x01;                                        // make PG0 out
    GPIO_PORTG_DATA_R &= 0b11111110;                                 //PG0 = 0
    FlashAllLEDs();
    SysTick_Wait10ms(10);
    GPIO_PORTG_DIR_R &= ~0x01;                                            // make PG0 input (HiZ)
    
}
void LEDOFF(void) {  //this is to turn the PORTN LEDs off
  GPIO_PORTN_DATA_R = 0b00000000; // Turn off LED on PN0 as measurement LED
}
void PortH_Init(void){   // port for the stepper motor which is For Port H, port H3:H0 will be used for the motor. //
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R7;
	while((SYSCTL_PRGPIO_R&SYSCTL_PRGPIO_R7) == 0){};	
	GPIO_PORTH_DIR_R |= 0xF;												
	GPIO_PORTH_AFSEL_R &= ~0xF;		 								
	GPIO_PORTH_DEN_R |= 0xF;											
																								
	GPIO_PORTH_AMSEL_R &= ~0xF;		 								
	
	return;
}
// Leds are already initalized via onboardLEDs.c
bool flag = false; //variable to exit the for loop when button is pressed
bool data = true; // this is for the UART to keep giving data 



//*********************************************************************************************************
//*********************************************************************************************************
//***********					MAIN Function				*****************************************************************
//*********************************************************************************************************
//*********************************************************************************************************
uint16_t	dev = 0x29;			//address of the ToF sensor as an I2C slave peripheral
int status=0;

int main(void) {
  uint8_t byteData, sensorState=0, myByteArray[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} , i=0;
  uint16_t wordData;
  uint16_t Distance;
  uint8_t RangeStatus;
  uint8_t dataReady;
// important code for the UART and motor spinning starts at line 242
	//initialize
	PLL_Init();	
	SysTick_Init();
	onboardLEDs_Init();
	I2C_Init();
	UART_Init();
	PortE0_Init();
	PortM0_Init();
	PortH_Init();
	bool motor = true; //again just defined in here 
	
	
	// hello world! //this is not needed for the prorgram the flashing of LEDS can be shown for booting ToF this would only clog the data sent through UART
	//UART_printf("Program Begins\r\n");  //This UART_printfs which are not needed have been commented out 
	int mynumber = 400062217; //student number 
	//sprintf(printf_buffer,"2DX ToF Program Studio Code %d\r\n",mynumber);
	//UART_printf(printf_buffer);


/* Those basic I2C read functions can be used to check your own I2C functions */
	status = VL53L1X_GetSensorId(dev, &wordData);

	sprintf(printf_buffer,"(Model_ID, Module_Type)=0x%x\r\n",wordData);
	//UART_printf(printf_buffer);

	// Booting ToF chip
	while(sensorState==0){
		status = VL53L1X_BootState(dev, &sensorState);
		SysTick_Wait10ms(10);
  }
	FlashAllLEDs();
	//UART_printf("ToF Chip Booted!\r\n Please Wait...\r\n"); // this is just initializing and booting the chip 
	
	status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/
	
  /* This function must to be called to initialize the sensor with the default setting  */
  status = VL53L1X_SensorInit(dev);
	Status_Check("SensorInit", status);

	
  /* Optional functions to be used to change the main ranging parameters according the application requirements to get the best ranging performances */
 //status = VL53L1X_SetDistanceMode(dev, 2); /* 1=short, 2=long */
//  status = VL53L1X_SetTimingBudgetInMs(dev, 100); /* in ms possible values [20, 50, 100, 200, 500] */
//  status = VL53L1X_SetInterMeasurementInMs(dev, 200); /* in ms, IM must be > = TB */
// although short mode was not used it could have been used and long distance mode would have no purpose in this project 
  status = VL53L1X_StartRanging(dev);   // This function has to be called to enable the ranging
// Starting from here is where the main function starts and detects if the pushbutons are pressed 
while(1) {
        // Drive Low PE0 (Row 0) for scanning
GPIO_PORTE_DATA_R = 0b11111110;
        
        // Check if button is pressed
  if((GPIO_PORTM_DATA_R & 0b00000001) == 0) { // This is just for the motor being pressed by itself
		// this whole if statement is just to show the motor being able to spin one whole rotation and if button is pressed again stops
		SysTick_Wait10ms(10); //debouncing period or delay which is needed not to have a false press
				
        if(motor == true) {
                // If motor is not running, start it in clockwise 
				flag = false; 
				for(int i=0; i<512; i++) {  
        if(flag) {
            break;
        }
        GPIO_PORTH_DATA_R = 0b00001100;  // This is for CW rotation 
        SysTick_Wait10ms(1); // delay which is required 
        GPIO_PORTH_DATA_R = 0b00000110;
        SysTick_Wait10ms(1);
        GPIO_PORTH_DATA_R = 0b00000011;
        SysTick_Wait10ms(1);
        GPIO_PORTH_DATA_R = 0b00001001;
        SysTick_Wait10ms(1);
						//wait until the ToF sensor's data is ready
	 
				
        if(i%64 == 0 && i != 0) {
            FlashLED1(1); // flashes led every 45 degrees exce[t at starting which is 0 degrees
					// in this case addtional status LED was used to show that 45 degrees had occured so PN1 or user LED 1
					//while for measuremnt PN0 or USER LED 2 would be used otherwise 
        }
        if(i == 0) {
            LEDOFF();  // Turn off LED on PN0 as measurement LED
        }
        if(i == 511) {  // this is for 360 degrees as it's a multiple of 45
            FlashLED1(1);
					
        }
				if (i > 10){ //i>10 was chosen so at least 10/512 of the rotation has to be done before stopping motor again to prevent accidental button presses
					 if((GPIO_PORTM_DATA_R & 0b00000001) == 0){
						 flag = !flag;}} // this toggles the flag value if button is pressed once motor is running so would turn flag into true 
    }
								motor = !motor;} //motor here toggles if was true now becomes false then next time button is pressed goes to the elsr statement 
								
								
								// now motor is false so when button is pressed again motor is turned off at port h 
								
					else{
						GPIO_PORTH_DATA_R = 0b00000000;
						SysTick_Wait10ms(10);
							motor = !motor;} //here motor is toggled again for the next button press
            
								
            }
int i = 0;
	// Get the distance measurement until the button is pressed again which should be for one rotation of the stepper motor 
	if((GPIO_PORTM_DATA_R & 0b00000010) == 0) {  //This is the important code as UART button has to be pressed first for proper functionality of project 
		SysTick_Wait10ms(100); //debouncing period or delay which is needed not to have a false press
		while(data == true) // this is for UART 
		{
		if((GPIO_PORTM_DATA_R & 0b00000001) == 0) { //everything in this if statement is for the motor while the UART is transmitting data 
		SysTick_Wait10ms(10); //debouncing period or delay which is needed not to have a false press
				
        if(motor == true) {
                // If motor is not running, start it in clockwise 
				flag = false; 
				while(flag == false) { // for deliverable 1 this was a for loop for one rotation here the motor spins until button is pressed again
        if(flag) { // if the button is pressed again the loop breaks and the motor stops spinning 
            break;
        }
				i++; 
        GPIO_PORTH_DATA_R = 0b00001100;  // This is for CW rotation 
        SysTick_Wait10ms(1); // delay which is required 
        GPIO_PORTH_DATA_R = 0b00000110;
        SysTick_Wait10ms(1);
        GPIO_PORTH_DATA_R = 0b00000011;
        SysTick_Wait10ms(1);
        GPIO_PORTH_DATA_R = 0b00001001;
        SysTick_Wait10ms(1);
						//wait until the ToF sensor's data is ready
	  while (dataReady == 0){
		  status = VL53L1X_CheckForDataReady(dev, &dataReady);
         
          VL53L1_WaitMs(dev, 5);
	  }
		dataReady = 0;
	  
		//read the data values from ToF sensor
	  status = VL53L1X_GetDistance(dev, &Distance);					//The Measured Distance value only thing we care about
	
    
		FlashLED2(1); //PN0 is flashed which is userD2 everytime a measurement takes place 

	  status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/  
		
		// print the resulted readings to UART
		sprintf(printf_buffer,"%u\r\n", Distance);  // This gives the distance which can be used to calucate the 
		UART_printf(printf_buffer);
				
        if(i%64 == 0 && i != 0) {
            FlashLED1(1); // flashes led every 45 degrees exce[t at starting which is 0 degrees
					// in this case addtional status LED was used to show that 45 degrees had occured so PN1 or user LED 1
					//while for measuremnt PN0 or USER LED 2 would be used otherwise 
        }
        if(i == 0) {
            LEDOFF();  // Turn off LED on PN0 as measurement LED
        }
				if (i > 10){
					 if((GPIO_PORTM_DATA_R & 0b00000001) == 0){ // conditions for the motor to stop spinning if motor button is pressed while running 
						 flag = !flag;}} // this toggles the flag value if button is pressed once motor is running so would turn flag into true 
    }
								motor = !motor;} //motor here toggles if was true now becomes false then next time button is pressed goes to the elsr statement 
								
								
								// now motor is false so when button is pressed again motor is turned off at port h 
								
					else{
						GPIO_PORTH_DATA_R = 0b00000000;
						SysTick_Wait10ms(10);
							motor = !motor;} //here motor is toggled again for the next button press
            
								
            }
		//wait until the ToF sensor's data is ready
	  while (dataReady == 0){
		  status = VL53L1X_CheckForDataReady(dev, &dataReady);
         
          VL53L1_WaitMs(dev, 5);
	  }
		dataReady = 0;
	  
		//read the data values from ToF sensor
	  status = VL53L1X_GetDistance(dev, &Distance);					//The Measured Distance value only thing we care abou t
	
    
		FlashLED2(1); //PN0 is flashed which is userD2 everytime a measurement takes place 

	  status = VL53L1X_ClearInterrupt(dev); /* clear interrupt has to be called to enable next interrupt*/  
		
		// print the resulted readings to UART
		sprintf(printf_buffer,"%u\r\n", Distance);  // distance gives the x and y with z being the known displacement. 
		UART_printf(printf_buffer); //to the UART we only care about distance
		if((GPIO_PORTM_DATA_R & 0b00000010) == 0){  //if the button is pressed again bool variable data toggles and the ToF ceases this is used to stop UART data being sent when needed 
			//to send distance data as per project requirements
			SysTick_Wait10ms(10);
			data = !data;}
  }
  
	VL53L1X_StopRanging(dev);}}  
  while(1) {}

}

//The majority of the code was combinations of studio 5 for the buttons , deliverable 1 and studio 8 with deliverable 1 the motor spinning code
//being in a function but for this purpose the function wouldnt work so motor code is directly put in the main while we are 
// waiting for a button press to initialize UART sending data and the motor to spin pressing these buttons again also causes these tasks
// to cease operation 
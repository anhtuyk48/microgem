/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F45K22
        Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"
#include<string.h>


#define MAX_MENU_ITEMS 5

void initializeLCD();
void changeStartupScreen(const char *message);
void setCursor(uint8_t column, uint8_t row);
void sendStringToLCD(const char *str);
char readKeypad(void);
void blinkingBlockCursorOn();
void pirSensor();
void tempratureReadings();
void setSecurityCode();
void promptAndCheckPasswordA();
void promptAndCheckPasswordD();
void printSysStatus();
void masterSettingsMenu();
int promptForMasterCode();


char key;
char keyValue;
char confirmedCode[7];
const char keys[4][4] =
    {
    {'1','4','7','*'},
    {'2','5','8','0'},
    {'3','6','9','#'},
    {'A','B','C','D'}
    };


char readKeypad(void)
    {
    // Make RD0-RD3 as outputs and RD4-RD7 as inputs
    TRISA = 0xF0;  // Set RA0-RA3 as outputs (0), RA4-RA7 as inputs (1)

    // Initial PORTA state: all rows high, columns are inputs with pull-ups
    PORTA = 0xFF;

    for (uint8_t row = 0; row < 4; row++)
        {
        // Ground one row at a time
        PORTA &= ~(1 << row);

        for (uint8_t col = 0; col < 4; col++)
            {
            // Check each column for a grounded key (active low)
            if (!(PORTA & (0x10 << col)))
                {
                // Debounce delay
                __delay_ms(20);
                // Check if the button is still pressed
                if (!(PORTA & (0x10 << col)))
                    {
                    PORTA = 0xFF; // Reset PORTA
                    return keys[row][col];
                    }
                }
            }
        // No key found, set the row back to high before the next iteration
        PORTA |= (1 << row);
        }
    PORTA = 0xFF; // Reset PORTA
    return '\0';  // No button pressed
    }


void initializeLCD()
    {
    EUSART2_Write(0xFE);        // Command to clear screen
    EUSART2_Write(0x58);        // Command to clear screen
    setCursor(1, 1);            // set cursor position 
    blinkingBlockCursorOn();    // blinking cursor
    __delay_ms(100);            // Delay to allow the LCD to process the command
    }


void sendStringToLCD(const char *str)
    {
    while (*str != '\0')           // Loop until the end of the string
        {
        EUSART2_Write(*str);       // Send the current character
        str++;                     // Move to the next character
        }
    }


void setCursor(uint8_t column, uint8_t row)
    {
    EUSART2_Write(0xFE);           // Command prefix
    EUSART2_Write(0x47);           // Set Cursor Position command
    EUSART2_Write(column);         // Column position
    EUSART2_Write(row);            // Row position
    }


void blinkingBlockCursorOn()
    {
    EUSART2_Write(0xFE);            // Command prefix
    EUSART2_Write(0x53);            // Blinking Block Cursor On command
    }


void changeStartupScreen(const char *message)
    {
    EUSART2_Write(0xFE);              // Command prefix
    EUSART2_Write(0x40);              // Start-Up Screen Change command

    while (*message != '\0')          // Loop through each character in the message
        {
        EUSART2_Write(*message);      // Send the current character
        message++;                    // Move to the next character
        }
    }


void pirSensor()                  
    {
    
    if(PORTBbits.RB3 == 1)        // check for signal for the PIR sensor 
        {
        sendStringToLCD("Motion Detected!                        ");
        PORTBbits.RB5 = 1;  
        PORTBbits.RB0 = 1;          
        PORTBbits.RB1 = 1;
        PORTBbits.RB2 = 1;
        __delay_ms(300);
        PORTBbits.RB0 = 0;          
        PORTBbits.RB1 = 0;
        PORTBbits.RB2 = 0;
        __delay_ms(300);
        
        }
    }


void tempratureReadings()
    {
    uint16_t convertedValue;                               // Variable to hold the raw ADC conversion result
    float temperature;                                     // Variable to hold the calculated temperature
    ADC_StartConversion();                                 // Start the ADC conversion process
    convertedValue = ADC_GetConversionResult();            // Fetch the ADC conversion result after the conversion completes
    temperature = (convertedValue / 1023.0) * 1.563;       // Convert the ADC value to a temperature using the equation for the sensor
    char displayString[20];                                // Define a character array to hold the display string
    sprintf(displayString, "%.0f C", temperature);   // Format the temperature into a string with 0 decimal places
    sendStringToLCD(displayString);                        // Send the formatted temperature string to the LCD
    }


void setSecurityCode()
    {
    char code[7];        // Array to store the first entered code
    char tempCode[7];    // Temporary array to store the confirmation code
    char input;          // Variable to store the keypad input
    uint8_t index = 0;   // Index for array positioning
    __delay_ms(250);     // Wait and clear any previous key press
    PORTA = 0xFF;        // Reset keypad state
    while (1)
        {
        initializeLCD();                      // Function to clear the LCD screen
        sendStringToLCD("Set Master Code:    ");
        index = 0;
        while (index < 6)
            {
            input = readKeypad();             // Read input from the keypad
            if (input != '\0')
                {
                code[index] = input;          // Store the digit
                sendStringToLCD("*");         // Display * for each digit
                index++;
                __delay_ms(200);              // Debounce delay
                }
            }
        code[6] = '\0';                       // Null terminate the string
        initializeLCD();                      // Clear LCD after first code entry, before confirmation prompt
        sendStringToLCD("Confirm Master Code:");    // Prompt for code confirmation
        index = 0;                            // Reset index for confirmation code entry
        while (index < 6)
            {
            input = readKeypad();             // Read input from the keypad
            if (input != '\0')
                {
                tempCode[index] = input;      // Store the digit
                sendStringToLCD("*");         // Display * for each digit
                index++;
                __delay_ms(200);              // Debounce delay
                }
            }
        tempCode[6] = '\0';                   // Null terminate the string
        if (strcmp(code, tempCode) == 0)      // Compare the two codes
            {
            initializeLCD();                        // Clear LCD before displaying the final message
            strcpy(confirmedCode, code);            // Copy the confirmed code to the global variable
            sendStringToLCD("Code Confirmed!!");
            __delay_ms(2000);                       // Display message for 2 seconds
            keyValue = 'B';                         // Set keyValue to 'A' to trigger the corresponding case in the main loop
            break;                                  // Exit the loop
            } 
        else
            {
            initializeLCD();                  // Clear LCD before displaying the retry message
            sendStringToLCD("Mismatch, Try Again");
            __delay_ms(2000);                 // Wait for 2 seconds
                                              // Loop will restart
            }
        }
    }



void promptAndCheckPasswordA()
    {
    char enteredCode[7]; // Store entered password
    char input;          // Keypad input
    uint8_t index;       // Array index
    __delay_ms(250);     // Wait and clear any previous key press

    while (1)
        {
        initializeLCD(); // Clear LCD screen
        sendStringToLCD("Master Code To Arm: ");
        index = 0;

        // Read password from keypad
        while (index < 6)
            {
            input = readKeypad(); // Read keypad input
            if (input != '\0')
                {
                enteredCode[index] = input; // Store digit
                sendStringToLCD("*");       // Display asterisk
                index++;
                __delay_ms(250);            // Debouncing delay
                }
            }
        enteredCode[6] = '\0'; // Null-terminate password

        // Check if entered password matches confirmedCode
        if (strcmp(enteredCode, confirmedCode) == 0)
            {
            initializeLCD();
            sendStringToLCD("Arming System!!"); // Display success message
            __delay_ms(500);                 
            PORTBbits.RB1 = 1;
            PORTBbits.RB5 = 1;
            __delay_ms(500);
            PORTBbits.RB1 = 0;
            PORTBbits.RB5 = 0;
            __delay_ms(500);                 
            PORTBbits.RB1 = 1;
            PORTBbits.RB5 = 1;
            __delay_ms(500);
            PORTBbits.RB1 = 0;
            PORTBbits.RB5 = 0;
             __delay_ms(500);                 
            PORTBbits.RB1 = 1;
            PORTBbits.RB5 = 1;
            __delay_ms(500);
            PORTBbits.RB1 = 0;
            PORTBbits.RB5 = 0;
            __delay_ms(500);
            PORTBbits.RB1 = 1;
            PORTBbits.RB5 = 1;
            __delay_ms(500);
            PORTBbits.RB1 = 0;
            PORTBbits.RB5 = 0;
            __delay_ms(500);                 
            PORTBbits.RB1 = 1;
            PORTBbits.RB5 = 1;
            __delay_ms(500);
            PORTBbits.RB1 = 0;
            PORTBbits.RB5 = 0;
             __delay_ms(500);                 
            PORTBbits.RB1 = 1;
            PORTBbits.RB5 = 1;
            __delay_ms(500);
            PORTBbits.RB1 = 0;
            PORTBbits.RB5 = 0;
            __delay_ms(1000);
            initializeLCD();
            sendStringToLCD("System Armed!!"); // Display success message
            __delay_ms(1000);
            keyValue = 77;
            __delay_ms(1000);
            initializeLCD();
            __delay_ms(1000);
            sendStringToLCD("System Monitored!!                      ");
            break;                            // Exit loop on success
            }
        else
            {
            initializeLCD();
            sendStringToLCD("Arming Denied,      Try Again!!"); // Display error message
            __delay_ms(2000);                           // 2-second delay
            }
        }
    }


void promptAndCheckPasswordD()
    {
    char enteredCode[7]; // Store entered password
    char input;          // Keypad input
    uint8_t index;       // Array index
    __delay_ms(300);     // Wait and clear any previous key press

    while (1)
        {
        initializeLCD(); // Clear LCD screen
        sendStringToLCD("Code To Disarm:     ");
        index = 0;

        // Read password from keypad
        while (index < 6)
            {
            input = readKeypad(); // Read keypad input
            if (input != '\0')
                {
                enteredCode[index] = input; // Store digit
                sendStringToLCD("*");       // Display asterisk
                index++;
                __delay_ms(200);            // Debouncing delay
                }
            }
        enteredCode[6] = '\0'; // Null-terminate password

        // Check if entered password matches confirmedCode
        if (strcmp(enteredCode, confirmedCode) == 0)
            {
            initializeLCD();
            sendStringToLCD("System Disarmed!!"); // Display success message
            __delay_ms(2000);
            keyValue = 'B';
            break;                            // Exit loop on success
            }
        else
            {
            initializeLCD();
            sendStringToLCD("Disarming Denied,   Try Again!!"); // Display error message
            __delay_ms(2000);                           // 2-second delay
            }
        }
    }


void printSysStatus()
{
    __delay_ms(100);
    initializeLCD();
    PORTBbits.RB2 = 1;
    if (PORTBbits.RB0 == 1)
    {
        PORTBbits.RB2 = 0;
    }
    sendStringToLCD("Sys Status: DisarmedTemp is   : ");
    tempratureReadings();
}


void masterSettingsMenu()
    {
    char input = '\0';
    uint8_t menuIndex = 1;  // Start with menu index 1
    const char* menuItems[MAX_MENU_ITEMS] = 
        {
        "1-Reset Master Pass", // index 1
        "2-Email Logs",        // index 2
        "3-Add User Priv",     // index 3
        "4-Pair BT Device",    // index 4
        "5-Exit"               // index 5
        };

    if (!promptForMasterCode())
        {
        return; // If master code is incorrect, exit the function
        }

    initializeLCD();
    sendStringToLCD("Master Settings Menu");
    sendStringToLCD(menuItems[menuIndex - 1]);

    while (1)
        {
        input = readKeypad();

        if (input != '\0')
            {
            if (input == '#')
                {
                menuIndex++;
                if (menuIndex > MAX_MENU_ITEMS) 
                    {
                    menuIndex = 1;
                    }
                }
            else if (input == '*')
                {
                if (menuIndex == 1)
                    {
                    menuIndex = MAX_MENU_ITEMS;
                    }
                else
                    {
                    menuIndex--;
                    }
                }
            else if (input == '0')
                {
                switch (menuIndex)
                    {
                    case 1:
                        setSecurityCode();
                        break;
                    case 2:
                        // Logic for Email System Logs
                        break;
                    case 3:
                        // Logic for Add User Privileges
                        break;
                    case 4:
                        // Logic for Pair Bluetooth Device
                        break;
                    case 5:
                        // Logic for Exit option
                        break;
                    }

                printSysStatus();
                return;  // Exit the function immediately after executing an item
                }

            initializeLCD();
            sendStringToLCD("Master Settings Menu");
            sendStringToLCD(menuItems[menuIndex - 1]);

            __delay_ms(100);  // Delay for debouncing
            }
        }
    }




int promptForMasterCode()
    {
    char enteredCode[7]; // Store entered password
    char input;          // Keypad input
    uint8_t index;       // Array index

    initializeLCD();     // Clear LCD screen
    sendStringToLCD("Enter Master Code:  ");

    index = 0;
    while (index < 6)
        {
        input = readKeypad(); // Read keypad input
        if (input != '\0')
            {
            enteredCode[index] = input; // Store digit
            sendStringToLCD("*");       // Display asterisk
            index++;
            __delay_ms(250);            // Debouncing delay
            }
        }
    enteredCode[6] = '\0'; // Null-terminate password

    if (strcmp(enteredCode, confirmedCode) == 0)
        {
        return 1; // Access granted
        }
    else
        {
        initializeLCD();
        sendStringToLCD("Incorrect Code      Try Again!!");
        __delay_ms(2000);
        return 0; // Access denied
        }
    }



void main(void)
{
    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    
    SYSTEM_Initialize();        // Initialize the device
    EUSART2_Initialize();       // Initialize the serial port
    ADC_Initialize();           // Initialize the ADC
    initializeLCD();            // Initialize the LCD
    changeStartupScreen("     MICRO-GEM          Phi & Anwar     "); // Change start-up message to "MICRO-GEM"
    setSecurityCode();
    

    while (1)
        {

        key = readKeypad();
        if (key != '\0')      // check if something is pressed on the keypad
            {
            keyValue = key;   // the isolation of NULL from the keyValue
            }
        
        if (PORTCbits.RC4 == 1)
        {
            keyValue = 'A';
        }
        
        if (PORTCbits.RC3 == 1)
        {
            keyValue = 'D';
        }
        

            
            
        
        switch (keyValue)
            {
            case 'A':
                PORTBbits.RB2 = 0;
                promptAndCheckPasswordA();
                break;
            case 'C':
                masterSettingsMenu();
                break;
            case 77:
                PORTBbits.RB2 = 0;
                PORTBbits.RB1 = 1;
                PORTBbits.RB0 = 0;
                pirSensor();  
                break;
            case 'D':   
                PORTBbits.RB2 = 0;
                PORTBbits.RB1 = 1;  
                promptAndCheckPasswordD();
                PORTBbits.RB5 = 0;
                PORTBbits.RB1 = 0;
                PORTBbits.RB0 = 1;               
                break;
                
            case 'B':
                printSysStatus();
                keyValue = key;
                break;    
            default:
                printf("The number is not 1, 2, or 3\n");
            }
             
        }//While
    
}// Main
/**
 End of File
*/

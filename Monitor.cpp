/**----------------------------------------------------------------------------
             \file Monitor.cpp
--                                                                           --
--              ECEN 5003 Mastering Embedded System Architecture             --
--                  Project 1 Module 3                                       --
--                Microcontroller Firmware                                   --
--                      Monitor.cpp                                            --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--               
--                
--  Designed by:  Tim Scherr
--  Revised by:  Kyle Bryan 
-- 
-- Version: 2.08
-- Date of current revision:  2016-09-29   
-- Target Microcontroller: Freescale MKL25ZVMT4 
-- Tools used:  ARM mbed compiler
--              ARM mbed SDK
--              Freescale FRDM-KL25Z Freedom Board
--               
-- 
   Functional Description: See below 
--
--      Copyright (c) 2015 Tim Scherr All rights reserved.
--
*/              

#include <stdio.h>
#include "shared.h"


/*******************************************************************************
* Set Display Mode Function
* Function determines the correct display mode.  The 3 display modes operate as 
*   follows:
*
*  NORMAL MODE       Outputs only mode and state information changes   
*                     and calculated outputs
*
*  QUIET MODE        No Outputs
*
*  DEBUG MODE        Outputs mode and state information, error counts,
*                    register displays, sensor states, and calculated output
*
*
* There is deliberate delay in switching between modes to allow the RS-232 cable 
* to be plugged into the header without causing problems. 
*******************************************************************************/

void set_display_mode(void)   
{
  UART_direct_msg_put("\r\n****************************************************************\r\n");
	UART_direct_msg_put("*                      Welcome to Debug Monitor                *");
	UART_direct_msg_put("\r\n*   Authored By: Kyle Bryan, Daniel Fairbanks, Shawn Thompson  *");
	UART_direct_msg_put("\r\n*                                                              *");
	UART_direct_msg_put("\r\n* Please Select Mode                                           *");
  UART_direct_msg_put("\r\n*  Hit NOR - Normal                                            *");
  UART_direct_msg_put("\r\n*  Hit QUI - Quiet                                             *");
  UART_direct_msg_put("\r\n*  Hit DEB - Debug                                             *");
  UART_direct_msg_put("\r\n*  Hit V - Version#                                            *");
	UART_direct_msg_put("\r\n*  HIT R - View Registers                                      *");
  UART_direct_msg_put("\r\n*  Enter a Hex Address(e.g 0xdeadbeef) to see its value        *");
	UART_direct_msg_put("\r\n****************************************************************\r\n");
	
	
  
  UART_direct_msg_put("\r\nSelect:  ");
  
}

/*******************************************************************************
. 
*******************************************************************************/
__asm void get_registers(uint32_t * p_arm_registers);

    

char getNumber(int n);
void itoa(uint32_t n, char * s, uint32_t base_n);
void reverse(char *s);

uint32_t hex2int(char *hex);


//*****************************************************************************/
/// \fn void chk_UART_msg(void) 
///
//*****************************************************************************/
void chk_UART_msg(void)    
{
   UCHAR j;
   while( UART_input() )      // becomes true only when a byte has been received
   {                                    // skip if no characters pending
      j = UART_get();                 // get next character

      if( j == '\r' )          // on a enter (return) key press
      {                // complete message (all messages end in carriage return)
         UART_msg_put("->");
         UART_msg_process();
      }
      else 
      {
         if ((j != 0x02) )         // if not ^B
         {                             // if not command, then   
            UART_put(j);              // echo the character   
         }
         else
         {
           ;
         }
         if( j == '\b' ) 
         {                             // backspace editor
            if( msg_buf_idx != 0) 
            {                       // if not 1st character then destructive 
               UART_msg_put(" \b");// backspace
               msg_buf_idx--;
            }
         }
         else if( msg_buf_idx >= MSG_BUF_SIZE )  
         {                                // check message length too large
            UART_msg_put("\r\nToo Long!");
            msg_buf_idx = 0;
         }
         else if ((display_mode == QUIET) && (msg_buf[0] != 0x02) && 
                  (msg_buf[0] != 'D') && (msg_buf[0] != 'N') && 
                  (msg_buf[0] != 'V') &&
                  (msg_buf_idx != 0))
         {                          // if first character is bad in Quiet mode
            msg_buf_idx = 0;        // then start over
         }
         else {                        // not complete message, store character
 
            msg_buf[msg_buf_idx] = j;
            msg_buf_idx++;
            if (msg_buf_idx > 10)
            {
               UART_msg_process();
            }
         }
      }
   }
}

//*****************************************************************************/
///  \fn void UART_msg_process(void) 
///UART Input Message Processing
//*****************************************************************************/
void UART_msg_process(void)
{
   UCHAR chr,err=0;
//   unsigned char  data;


   if( (chr = msg_buf[0]) <= 0x60 ) 
   {      // Upper Case
      switch( chr ) 
      {
         case 'D':
            if((msg_buf[1] == 'E') && (msg_buf[2] == 'B') && (msg_buf_idx == 3)) 
            {
               display_mode = DEBUG;
               UART_msg_put("\r\nMode=DEBUG\n");
               display_timer = 0;
            }
            elseR
               err = 1;
            break;

         case 'N':
            if((msg_buf[1] == 'O') && (msg_buf[2] == 'R') && (msg_buf_idx == 3)) 
            {
               display_mode = NORMAL;
               UART_msg_put("\r\nMode=NORMAL\n");
               //display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'Q':
            if((msg_buf[1] == 'U') && (msg_buf[2] == 'I') && (msg_buf_idx == 3)) 
            {
               display_mode = QUIET;
               UART_msg_put("\r\nMode=QUIET\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'V':
            display_mode = VERSION;
            UART_msg_put("\r\n");
            UART_msg_put( CODE_VERSION ); 
				    UART_msg_put("\r\nSelect:   ");
            display_timer = 0;
            break;
			  
				 case 'R':
					 display_mode = VERSION;
				   uint32_t * reg = (uint32_t*)malloc(16 * sizeof(uint32_t));
           get_registers(reg);
           uint32_t i=0;
           char * word = (char*)malloc(4 * sizeof(char));
           while(i < 16)
           {
                   
               itoa(i,word,10);
               UART_direct_msg_put("\r\nRegister ");
               UART_direct_msg_put(word);
               UART_direct_msg_put(": ");
               itoa(reg[i],word,16);
               UART_direct_msg_put(word);
               i++;
           }
           i=0;
           free(reg);
           free(word);
           // clear flag to ISR      
           display_flag = 0;
					 UART_msg_put("\r\nSelect:   ");
				   break;
        
        case '0':
          if(msg_buf_idx == 10)
          {
              char word[4];
              char buffer[50];
						  sprintf(word, "%s\n\r", msg_buf+2);
              long it1 = strtol(word, 0, 16);
						  /*if(*(char *)it1 == NULL)
							{
								UART_direct_msg_put("ERROR: Nothing at memory address");
								display_timer=0;
								break;
							}*/
              char * val = (char *)it1;
						  sprintf(buffer,"\n\rAddress: 0x%p Data:0x%x%x%x%x\n\r", val,*val, *(val+1), *(val+2), *(val+3) );
              UART_direct_msg_put(buffer);
              display_timer = 0;
              
          }
          else
             err = 1;
          UART_msg_put("\r\nSelect:   ");
					break;
                
         default:
            err = 1;
      }
   }

   else 
   {                                 // Lower Case
      switch( chr ) 
      {
        default:
         err = 1;
      }
   }

   if( err == 1 )
   {
      UART_msg_put("\n\rError!");
   }   
   else if( err == 2 )
   {
      UART_msg_put("\n\rNot in DEBUG Mode!");
   }   
   else
   {
    msg_buf_idx = 0;          // put index to start of buffer for next message
      ;
   }
   msg_buf_idx = 0;          // put index to start of buffer for next message


}


//*****************************************************************************
///   \fn   is_hex
/// Function takes 
///  @param a single ASCII character and returns 
///  @return 1 if hex digit, 0 otherwise.
///    
//*****************************************************************************
UCHAR is_hex(UCHAR c)
{
   if( (((c |= 0x20) >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f'))  )
      return 1;
   return 0;
}

/*******************************************************************************
*   \fn  DEBUG and DIAGNOSTIC Mode UART Operation
*******************************************************************************/
void monitor(void)
{

/**********************************/
/*     Spew outputs               */
/**********************************/

   switch(display_mode)
   {
      case(QUIET):
         {
             UART_msg_put("\r\n ");
             display_flag = 0;
         }  
         break;
      case(VERSION):
         {
             display_flag = 0;
         }  
         break;         
      case(NORMAL):
         {
            if (display_flag == 1)
            {
               UART_msg_put("\r\nNORMAL ");
               UART_msg_put(" Flow: ");
               // ECEN 5803 add code as indicated
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
               UART_msg_put(" Temp: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
               UART_msg_put(" Freq: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
               display_flag = 0;
            }
         }  
         break;
      case(DEBUG):
         {
            if (display_flag == 1)
            {
               UART_msg_put("\r\nDEBUG ");
               UART_msg_put(" Flow: ");
               // ECEN 5803 add code as indicated               
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
               UART_msg_put(" Temp: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
               UART_msg_put(" Freq: ");
               //  add flow data output here, use UART_hex_put or similar for 
               // numbers
						}  
               
 /****************      ECEN 5803 add code as indicated   ***************/             
               //  Create a display of  error counts, sensor states, and
               //  ARM Registers R0-R15
               
               //  Create a command to read a section of Memory and display it
               
               
               //  Create a command to read 16 words from the current stack 
               // and display it in reverse chronological order       
         }  
         break;
      default:
      {
         UART_msg_put("Mode Error");
      }  
   }
}

__asm void get_registers(uint32_t * p_arm_registers)
{
      PUSH {r1-r7, lr}
      STR r0, [r0, #0]
      STR r1, [r0, #4]
      STR r2, [r0, #8]
      STR r3, [r0, #12]
      STR r4, [r0, #16]
      STR r5, [r0, #20]
      STR r6, [r0, #24]
      STR r7, [r0, #28]
      MOV r1, r8
      MOV r2, r9
      MOV r3, r10
      MOV r4, r11
      MOV r5, r12
      MOV r6, r13
      MOV r7, r14
      STR r1, [r0, #32]
      STR r2, [r0, #36]
      STR r3, [r0, #40]
      STR r4, [r0, #44]
      STR r5, [r0, #48]
      STR r6, [r0, #52]
      STR r7, [r0, #56]
      MOV r1, r15
      STR r1, [r0, #60]
      POP {r1-r7, pc}
}

    

char getNumber(uint32_t n){
  if(n>9) {
    return n-10+'a';
  } else {
    return n+'0';
  }
}
 void itoa(uint32_t n, char * s, uint32_t base_n)
 {
     uint32_t i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = getNumber(n % base_n);   /* get next digit */
     } while ((n /= base_n) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }
 
void reverse(char *s) 
{
    char *j;
    int c;
 
    j = s + strlen(s) - 1;
    while(s < j) 
    {
        c = *s;
        *s++ = *j;
        *j-- = c;
    }
}

uint32_t hex2int(char *hex) 
{
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++; 
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;    
        // shift 4 to make space for new digit, and add the 4 bits of the new digit 
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}
/*!
	@file RTC7301.c	
	@brief Source code for Epson RTC-7301 real time clock on 6309 SBC SD card.
	@version V0.0 (c) 2021 Jacob Beeksma.
*/
#include "RTC7301.h"

/**
    date is the interface to the real time clock
    V0.1: Only read RTC date and time
    Uses global variables gd_cent, gd_year, gd_month, gd_date, gd_wday, gt_hrs, gt_min, gt_sec to pass data
*/
int date(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN])
{
CLI_FUNC selectedFunction;
int resultCode;

    if (selectedFunction=lookupCmd(dateCmds, argv[1])){                     //if non-0, got a pointer to function
        resultCode=(*selectedFunction)(argc, argv);                         //Execute the selected function and pass arguments
        return(resultCode);
    } else {                                                                //If 0, we did not find a function
        printf("\a\n Unknown command");
        return(RTC_CMD_ERR);
    }            
} 

/*
    subfunction dispdate of the date command, called when argv[1]==""
*/
int dispdate(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN])  
{    
            updateDT();
            printf("\n");
            dispWday();
            dispDate();
            dispTime();          
}

/*
    subfunction setDateTime of the date command, called when argv[1]=="set"
*/

int setDateTime(int argc, char argv[CLI_MAX_ARGS][CLI_ARGLEN])  
{ 
struct RTC7301_registers * RTC_reg; 
   
            //FIXME: Hack to set date and time provisionally
            RTC_reg=(struct RTC7301_registers *)RTC_BASE_ADDRESS;                       //Set the start address of the structure at the RTC base address.
            RTC_reg->addrF.control    = (unsigned char)(RTC_BANKSEL0 + RTC_STOP);       //Select bank 0, stop clock        
            RTC_reg->addrE.t_cent_10  = (unsigned char)argv[2][0];                      //set values
            RTC_reg->addrD.t_cent_1   = (unsigned char)argv[2][1];
            RTC_reg->addrC.t_year_10  = (unsigned char)argv[2][2];
            RTC_reg->addrB.t_year_1   = (unsigned char)argv[2][3];
            RTC_reg->addrA.t_month_10 = (unsigned char)argv[2][4];
            RTC_reg->addr9.t_month_1  = (unsigned char)argv[2][5];
            RTC_reg->addr8.t_date_10  = (unsigned char)argv[2][6];
            RTC_reg->addr7.t_date_1   = (unsigned char)argv[2][7];
            RTC_reg->addr6.t_wday     = (unsigned char)argv[2][8];
            RTC_reg->addr5.t_hour_10  = (unsigned char)argv[2][9];
            RTC_reg->addr4.t_hour_1   = (unsigned char)argv[2][10];
            RTC_reg->addr3.t_min_10   = (unsigned char)argv[2][11];
            RTC_reg->addr2.t_min_1    = (unsigned char)argv[2][12];
            RTC_reg->addr1.t_sec_10   = (unsigned char)argv[2][13];
            RTC_reg->addr0.t_sec_1    = (unsigned char)argv[2][14];
            RTC_reg->addrF.control    = (unsigned char)(RTC_BANKSEL0 + RTC_RUN);        //Select bank 0, start clock        
}

/*
    copy date and time from RTC into BCD coded global variables
*/
int updateDT()
{
struct RTC7301_registers * RTC_reg; 
unsigned char * pointer;

    //TODO: preliminary test version to see if we can read the RTC from a C program.
    RTC_reg=(struct RTC7301_registers *)RTC_BASE_ADDRESS;                               //Set the start address of the structure at the RTC base address.
    
    gd_cent=((RTC_reg->addrE.t_cent_10%7) <<4) + (RTC_reg->addrD.t_cent_1%7);                    //The two digits of the century
    gd_year=((RTC_reg->addrC.t_year_10%7) <<4) +( RTC_reg->addrB.t_year_1%7);                    //The two digits of the year
    gd_month=((RTC_reg->addrA.t_month_10%7) <<4) + (RTC_reg->addr9.t_month_1%7);                 //The two digits of the month
    gd_date=((RTC_reg->addr8.t_date_10%7) <<4) + (RTC_reg->addr7.t_date_1%7);                    //The two digits of the date
    gd_wday=RTC_reg->addr6.t_wday%7;                                                      //Weekday 0=sunday etc.
    gt_hrs=((RTC_reg->addr5.t_hour_10%7) <<4) + (RTC_reg->addr4.t_hour_1%7);                     //The two digits of the hour
    gt_min=((RTC_reg->addr3.t_min_10%7) <<4) + (RTC_reg->addr2.t_min_1%7);                       //The two digits of the minute        return(0);
    gt_sec=((RTC_reg->addr1.t_sec_10%7) <<4) + (RTC_reg->addr0.t_sec_1%7);                       //The two digits of the minute        return(0);
}

void dispWday()
{
    if (gd_wday<7) {
        printf("%s ",weekday[gd_wday]);
    } else {
        printf("Err ");
    }
}

void dispDate()
{
    printf("%02x/%02x/%02x%02x ", gd_date, gd_month, gd_cent, gd_year);
}

void dispTime()
{
    printf("%02x:%02X:%02x ", gt_hrs, gt_min, gt_sec);
}
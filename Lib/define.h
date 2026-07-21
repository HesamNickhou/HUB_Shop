#ifndef DEFINE_H
#define DEFINE_H

#define ALIVE_MESSAGE "{\"Request\":{\"Status\":69}}\n"
//#define DEBUG_HNA

#define IO 						5
#define Shop 					6
#define Sport 				7 
#define AmusementPark 8 //شهربازي، چند نرخي، اعتباري
#define Parking 			9
#define BusEtebar 		10
#define BCU 					11 
#define BD 						12
#define AutoPark 			13 
#define APARK 				14 
#define Appointment 	15
#define gateRoadblock 16

#define deviceType Sport

#if (deviceType == Sport)
	#define BASE 10000
#else
	#define BASE 0
#endif

//#define Release BASE + 312 //1404/11/12 //رفع ايراد فيدبک پوز سامان کيش
#define Release BASE + 313 //1405/06/21 //پشتيباني از سنسور جديد نبض

#define ONEREADCARD
#define MaxReciveBuf

//#define mediaTechTab
#define AutoStart
//                                     #define serialport //test port

#if (deviceType == IO)
	#define finger
	#define atlas
//	#define Printer
	#define Max30102	
	#define RC522_Routine
#endif

#if (deviceType == Sport)
  #define pos
  #define finger
  //#define atlas
  //#define heightHum
	#define RC522_Routine
  //#define Printer
  #define Max30102
#endif

#if(deviceType==AmusementPark)
	#define pos
	#define ExecTransaction
//	#define cardAccDis	
	#define PN532_Routine 
//#define Printer
#endif


#if(deviceType==BusEtebar)
	#define pos
	#define ExecTransactionBus
	#define PN532_Routine 
// #define Printer
#endif

#if(deviceType==Shop)
	#define RC522_Routine
	#define pos
	#define tarazo
	#define DS7
//	#define finger
//	#define heightHum
//	#define Max30102
//	#define Printer
#endif

#if(deviceType==BCU)
	#define ExecTransactionBCU // for bus
	#define PN532_Routine 
//	#define NFC
	//#define LCD
	//#define finger
	//#define CoordinatedOldBCU	
#endif

#if(deviceType==BD)
	#define ExecTransactionBD
  #define CoordinatedOldBD
	#define PN532_Routine 
#endif

#if(deviceType==Parking)
//	#define UHF
	#define RC522_Routine
	#define pos
	#define ReadWrite 
  #define DS7
	#define deviceType AutoPark
//#define Printert
//  #define remote
//	#define virtualRemote
#endif

#if(deviceType==AutoPark)
  #define ReadWrite //ssssssssss
	#define RC522_Routine
	#define AutoStart	
	#define cardAccDis	
	#define CS2(x)      HAL_GPIO_WritePin(CS2_GPIO_Port,CS2_Pin,x);
	#define pos 
//	#define NoDispenser     //whit red 2 whitout dispenser
#endif



#if(deviceType==APARK)
	#define ExecTransactionAPARK // for AmusementPark
	#define PN532_Routine 
	#define deviceType BCU
#endif

#if(deviceType==Appointment)
	#define RC522_Routine 
	#define remote
#endif

#if(deviceType==gateRoadblock)
	#define RC522_Routine 
	#define GATEROADBLOCK 	
#endif



#if defined(finger) || defined(Printer) || defined(pos)
#define MaxReciveBuf
#endif

#if defined(DS7) && defined(heightHum)
#error "just define DS7 or heightHum" 
#endif

#if defined(Printer) && defined(cardAccDis)
#error "just define printer or cardAccDis" 
#endif

#if defined(PN532_Routine) && defined(RC522_Routine) 
#error "just define PN532 or RC522 " 
#endif

#if !defined(PN532_Routine) && !defined(RC522_Routine) 
#error " define PN532 or RC522 " 
#endif

#if defined(UHF) && defined (pos)
#error "just define UHF or pos" 
#endif

#if defined(ExecTransactionAPARK) && defined (ExecTransactionBCU)
#error "just define  ExecTransactionBCU or ExecTransactionAPARK" 
#endif


#endif
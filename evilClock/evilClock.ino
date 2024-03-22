#include "M5StickCPlus2.h"
#include "WORLD_IR_CODES.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <TFT_eSPI.h>
#include <ESP32Time.h>
#include <EEPROM.h>
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

#include "Noto.h"
#include "smallFont.h"
#include "middleFont.h"
#include "bigFont.h"
#include "secFont.h"

void xmitCodeElement(uint16_t ontime, uint16_t offtime, uint8_t PWM_code );
void delay_ten_us(uint16_t us);
uint8_t read_bits(uint8_t count);
uint16_t rawData[300];

#define putstring_nl(s) Serial.println(s)
#define putstring(s) Serial.print(s)
#define putnum_ud(n) Serial.print(n, DEC)
#define putnum_uh(n) Serial.print(n, HEX)

#define MAX_WAIT_TIME 65535 //tens of us (ie: 655.350ms)

IRsend irsend(IRLED);  // Set the GPIO to be used to sending the message.

extern const IrCode* const NApowerCodes[];
extern const IrCode* const EUpowerCodes[];
extern uint8_t num_NAcodes, num_EUcodes;

uint8_t bitsleft_r = 0;
uint8_t bits_r=0;
uint8_t code_ptr;
volatile const IrCode * powerCode;

ESP32Time rtc(0); 
#define EEPROM_SIZE 4

//colors
unsigned short grays[12];
#define blue 0x1314
#define blue2 0x022F
unsigned short tmpColor=0;

double rad=0.01745;
float x[10];
float y[10];
float px[10];
float py[10];

float x2[10];
float y2[10];
float px2[10];
float py2[10];

float x3[61];
float y3[61];
float px3[61];
float py3[61];

int r=8;
int sx=120;
int sy=84;

int r2=16;
int sx2=23;
int sy2=178;

int r3=21;
int sx3=65;
int sy3=98;


int poz=0;

//time variables
String h,m,s;
int day,month;
String months[12]={"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};

//Function
int setFunction=0;
int setFunctionMax=5;

//settime variables
bool setTimeDate=false;
int setData[8];  //setHour,setMin,setSec,setDate,setMonth,setYear; SET REGION , SET BEEPER;
String setDataLbl[8]={"HOUR","MIN","SEC","DATE","MON","YEAR","REGION","SOUND"};
int setMin[8]={0,0,0,1,1,24,0,0};
int setMax[8]={24,60,60,32,13,36,2,2};
int setPosX[8]={10,50,91,10,50,91,8,8};
int setPosY[8]={54,54,54,124,124,124,172,192};
int chosen=0;


//brightness and battery
int brightnes[6]={16,32,48,64,96,180};
int b=2;
int vol;
int volE;

//region and buzze and ir
int Myregion=0;
int buzzer=0;
bool sendIR=false;

//stopwatch variable
int fase=0; //0=0 1=start 2=stop 
String minn="00";
String secc="00";
String mill="00";

//sleep variables
int sleepTime=10;
int ts,tts=0;
bool slp=false;

// we cant read more than 8 bits at a time so dont try!
uint8_t read_bits(uint8_t count)
{
  uint8_t i;
  uint8_t tmp=0;

  // we need to read back count bytes
  for (i=0; i<count; i++) {
    // check if the 8-bit buffer we have has run out
    if (bitsleft_r == 0) {
      // in which case we read a new byte in
      bits_r = powerCode->codes[code_ptr++];
      DEBUGP(putstring("\n\rGet byte: ");
      putnum_uh(bits_r);
      );
      // and reset the buffer size (8 bites in a byte)
      bitsleft_r = 8;
    }
    // remove one bit
    bitsleft_r--;
    // and shift it off of the end of 'bits_r'
    tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count-1-i));
  }
  // return the selected bits in the LSB part of tmp
  return tmp;
}


#define BUTTON_PRESSED LOW 
#define BUTTON_RELEASED HIGH

uint16_t ontime, offtime;
uint8_t i,num_codes;
uint8_t region;

void setup()  // ##############  SETUP ##################
{

    pinMode(35,INPUT_PULLUP);
    auto cfg = M5.config();
    StickCP2.begin(cfg);
    StickCP2.Rtc.setDateTime( { { 2024, 3, 22 }, { 12, 35, 0 } } );

     irsend.begin();

  pinMode(LED, OUTPUT);
  pinMode(TRIGGER, INPUT_PULLUP);

  delay_ten_us(5000); 

   // Debug output: indicate how big our database is
  DEBUGP(putstring("\n\rNA Codesize: ");
  putnum_ud(num_NAcodes);
  );
  DEBUGP(putstring("\n\rEU Codesize: ");
  putnum_ud(num_EUcodes);
  );

    rtc.setTime(0, 0, 0, 1, 1, 2023,0);
    EEPROM.begin(EEPROM_SIZE);
    b=EEPROM.read(0);
    if(b>7) b=2;

   Myregion=EEPROM.read(1);
    if(Myregion>1) Myregion=0;

    buzzer=EEPROM.read(2);
    if(buzzer>1) buzzer=0;

    StickCP2.Display.setBrightness(brightnes[b]);
 

    sprite.createSprite(135,240);
    sprite.setSwapBytes(true);    
    
    for(int i=0;i<8;i++)
    {
       x[i]=((r-1)*cos(rad*(i*45)))+sx;
       y[i]=((r-1)*sin(rad*(i*45)))+sy;
       px[i]=(r*cos(rad*(i*45)))+sx;
       py[i]=(r*sin(rad*(i*45)))+sy;
    }

      for(int i=0;i<10;i++)
    {
       x2[i]=((r2-3)*cos(rad*(i*36)))+sx2;
       y2[i]=((r2-3)*sin(rad*(i*36)))+sy2;
       px2[i]=(r2*cos(rad*(i*36)))+sx2;
       py2[i]=(r2*sin(rad*(i*36)))+sy2;
    }

    int an=270;
    for(int i=0;i<60;i++)
    {
       x3[i]=((r3-6)*cos(rad*an))+sx3;
       y3[i]=((r3-6)*sin(rad*an))+sy3;
       px3[i]=(r3*cos(rad*an))+sx3;
       py3[i]=(r3*sin(rad*an))+sy3;
       an=an+6; if(an>=360) an=0;
    }

    int co=216;
    for(int i=0;i<12;i++)
    { grays[i]=tft.color565(co, co, co);
    co=co-20;}

}

void sendAllCodes() 
{
  bool endingEarly = false; //will be set to true if the user presses the button during code-sending 
      
  // determine region from REGIONSWITCH: 1 = NA, 0 = EU (defined in main.h)

   if(Myregion==0)
    {region = EU;
    num_codes = num_EUcodes;
    }

      if(Myregion==1)
    {region = NA;
    num_codes = num_NAcodes;
    }
 

  // for every POWER code in our collection
  for (i=0 ; i<num_codes; i++) 
  {

    // print out the code # we are about to transmit
    DEBUGP(putstring("\n\r\n\rCode #: ");
    putnum_ud(i));

    // point to next POWER code, from the right database
    if (region == NA) {
      powerCode = NApowerCodes[i];
    }
    else {
      powerCode = EUpowerCodes[i];
    }
    
    // Read the carrier frequency from the first byte of code structure
    const uint8_t freq = powerCode->timer_val;
    // set OCR for Timer1 to output this POWER code's carrier frequency

    // Print out the frequency of the carrier and the PWM settings
    DEBUGP(putstring("\n\rFrequency: ");
    putnum_ud(freq);
    );
    
    DEBUGP(uint16_t x = (freq+1) * 2;
    putstring("\n\rFreq: ");
    putnum_ud(F_CPU/x);
    );

    // Get the number of pairs, the second byte from the code struct
    const uint8_t numpairs = powerCode->numpairs;
    DEBUGP(putstring("\n\rOn/off pairs: ");
    putnum_ud(numpairs));

    // Get the number of bits we use to index into the timer table
    // This is the third byte of the structure
    const uint8_t bitcompression = powerCode->bitcompression;
    DEBUGP(putstring("\n\rCompression: ");
    putnum_ud(bitcompression);
    putstring("\n\r"));

    // For EACH pair in this code....
    code_ptr = 0;
    for (uint8_t k=0; k<numpairs; k++) {
      uint16_t ti;

      // Read the next 'n' bits as indicated by the compression variable
      // The multiply by 4 because there are 2 timing numbers per pair
      // and each timing number is one word long, so 4 bytes total!
      ti = (read_bits(bitcompression)) * 2;

      // read the onTime and offTime from the program memory
      ontime = powerCode->times[ti];  // read word 1 - ontime
      offtime = powerCode->times[ti+1];  // read word 2 - offtime

      DEBUGP(putstring("\n\rti = ");
      putnum_ud(ti>>1);
      putstring("\tPair = ");
      putnum_ud(ontime));
      DEBUGP(putstring("\t");
      putnum_ud(offtime));      

      rawData[k*2] = ontime * 10;
      rawData[(k*2)+1] = offtime * 10;
      yield();
    }

    // Send Code with library
    irsend.sendRaw(rawData, (numpairs*2) , freq);
    Serial.print("\n");
    yield();
    //Flush remaining bits, so that next code starts
    //with a fresh set of 8 bits.
    bitsleft_r=0;

  
    // delay 205 milliseconds before transmitting next POWER code
    delay_ten_us(20500);

    // if user is pushing (holding down) TRIGGER button, stop transmission early 

    
  } //end of POWER code for loop

  if (endingEarly==false)
  {
    //pause for ~1.3 sec, then flash the visible LED 8 times to indicate that we're done
    delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
    delay_ten_us(MAX_WAIT_TIME); // wait 655.350ms
  }

} //end of sendAllCodes

void drawMain()
{
            
sprite.fillSprite(TFT_BLACK);
sprite.setTextDatum(4);

   sprite.fillSmoothRoundRect(6,6,98,124,2,grays[7],TFT_BLACK);
   sprite.fillSmoothRoundRect(8,8,94,120,2,TFT_BLACK,grays[7]);

   sprite.fillSmoothRoundRect(46,158,84,75,2,grays[7],TFT_BLACK);
   sprite.fillSmoothRoundRect(48,160,80,71,2,TFT_BLACK,grays[7]);
   sprite.fillSmoothRoundRect(52,209,72,18,2,blue2,TFT_BLACK);
   sprite.fillSmoothRoundRect(6,214,34,18,2,TFT_ORANGE,TFT_BLACK);

//seconds %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sprite.loadFont(secFont);
sprite.setTextColor(grays[5],TFT_BLACK);

for(int i=0;i<60;i++)
if(i%5==0)
sprite.fillSmoothCircle(px3[i],py3[i],1,TFT_ORANGE,TFT_BLACK);
sprite.drawWedgeLine(sx3,sy3,x3[s.toInt()],y3[s.toInt()],4,1,TFT_ORANGE,TFT_BLACK);
sprite.fillSmoothCircle(sx3,sy3,2,TFT_BLACK,TFT_ORANGE);

sprite.unloadFont();

//date #########################################################
 sprite.loadFont(smallFont);
 sprite.setTextColor(0x35D7,TFT_BLACK);
 sprite.drawString(months[month-1],22,84);
 sprite.fillRect(12,94,22,30,grays[8]);
 sprite.setTextColor(TFT_ORANGE,grays[8]);
 sprite.drawString(String(day),22,110);
 sprite.setTextColor(grays[3],TFT_BLACK);
 sprite.drawString(String(sleepTime),sx2,sy2+2);
 sprite.unloadFont();


//stopwatch ################
  sprite.loadFont(middleFont);

  if (setFunction == 0)
  {
    sprite.setTextColor(TFT_BLACK,grays[4]);
    sprite.setTextColor(grays[2],TFT_BLACK);
    sprite.drawString(":",88-12,186);
    sprite.drawString(":",88+12,186);
    sprite.drawString(secc,88,186);
    sprite.drawString(minn,88-24,186);
    sprite.drawString(mill,88+24,186);
  }

  if (setFunction == 1)
  {
    sprite.setTextColor(TFT_BLACK,grays[4]);
    sprite.setTextColor(grays[2],TFT_BLACK);
    sprite.drawString("IR",88,186);
  }

  if (setFunction == 2)
  {
    sprite.setTextColor(TFT_BLACK,grays[4]);
    sprite.setTextColor(grays[2],TFT_BLACK);
    sprite.drawString("BT",88,186);
  }

  if (setFunction == 3)
  {
    sprite.setTextColor(TFT_BLACK,grays[4]);
    sprite.setTextColor(grays[2],TFT_BLACK);
    sprite.drawString("Set Time",88,186);
  }

  if (setFunction == 4)
  {
    sprite.setTextColor(TFT_BLACK,grays[4]);
    sprite.setTextColor(grays[2],TFT_BLACK);
    sprite.drawString("Set Brig",88,186);
  }

  sprite.unloadFont();


//time ##########################
   sprite.loadFont(bigFont);
   sprite.setTextColor(grays[0],TFT_BLACK);
   sprite.drawString(h+":"+m,52,52);
   sprite.unloadFont();
   sprite.loadFont(Noto);
  
   
   //////brightness region
   sprite.fillSmoothRoundRect(110,70,20,58,2,blue,TFT_BLACK);
   sprite.fillSmoothCircle(sx,sy,5,grays[1],blue);
   for(int i=0;i<8;i++)
   sprite.drawWedgeLine(px[i],py[i],x[i],y[i],1,1,grays[1],blue);
   for(int i=0;i<b+1;i++)
   sprite.fillRect(115,122-(i*5),10,3,grays[1]);

   for(int i=0;i<10;i++)
   sprite.drawWedgeLine(px2[i],py2[i],x2[i],y2[i],2,2,0x7000,TFT_BLACK);

   for(int i=0;i<sleepTime;i++)
   sprite.drawWedgeLine(px2[i],py2[i],x2[i],y2[i],2,2,TFT_RED,TFT_BLACK);

    // butons
    sprite.setTextColor(TFT_BLACK,TFT_ORANGE);
    sprite.drawString("SET",22,225);
    sprite.setTextColor(grays[1],blue2);
    sprite.drawString("STA/STP",87,220);
    sprite.setTextColor(grays[3],TFT_BLACK);
    
    //batery region
    
    sprite.unloadFont();
    sprite.setTextColor(grays[5],TFT_BLACK);
    sprite.drawString(String(vol/1000.00),121,52);
    sprite.drawRect(114,12,14,28,grays[2]);
    sprite.fillRect(118,9,6,3,grays[2]);
    for(int i=0;i<volE;i++)
    sprite.fillRect(116,35-(i*5),10,3,TFT_GREEN);
    
    sprite.setTextDatum(0);
    sprite.setTextColor(grays[4],TFT_BLACK);
    sprite.drawString("SLEEP",9,202);
    sprite.setTextColor(grays[2],TFT_BLACK);
    
    if (setFunction == 0)
      sprite.drawString("STOPWATCH",55,165);
    if (setFunction > 0)
      sprite.drawString("FUNCTION",55,165);

    sprite.drawString("TIME AND DATE",12,14);
    
    //buzzer
    sprite.setTextColor(TFT_ORANGE,TFT_BLACK);
    sprite.drawString("BEEP",110,134);
     if(buzzer)
     sprite.drawString("ON",110,146);
     else
     sprite.drawString("OFF",110,146);

    
    //region 
    sprite.setTextColor(grays[2],TFT_BLACK);
    sprite.drawString("REG",8,135);
     if(Myregion==0)
     sprite.drawString("EU",8,146);
     else
     sprite.drawString("NA",8,146);

    sprite.setTextColor(grays[6],TFT_BLACK);
    sprite.drawString("VOLOS YT",55,198);
    sprite.setTextColor(grays[6],TFT_BLACK);
    sprite.drawString(String(s),88,116);

    sprite.fillTriangle(94,142,104,135,104,151,TFT_ORANGE);
    sprite.fillRect(94,139,8,9,TFT_ORANGE);
    

sprite.fillRect(36,134,3,5,TFT_RED);
for(int i=0;i<10;i++)
if(i==poz)
sprite.fillRect(30+(i*6),142,3,6,TFT_RED);
else
sprite.fillRect(30+(i*6),142,3,6,grays[6]);

   StickCP2.Display.pushImage(0,0,135,240,(uint16_t*)sprite.getPointer());
     
}

void drawSet()
{
 sprite.fillSprite(TFT_BLACK);
 sprite.setTextDatum(4);
   sprite.fillSmoothRoundRect(48,214,76,18,2,blue2,TFT_BLACK);
   sprite.fillSmoothRoundRect(6,214,34,18,2,TFT_ORANGE,TFT_BLACK);
   sprite.fillSmoothRoundRect(91,190,34,18,2,TFT_ORANGE,TFT_BLACK);

    sprite.loadFont(Noto);
    sprite.setTextColor(TFT_BLACK,TFT_ORANGE);
    sprite.drawString("OK",22,225);
    sprite.drawString("CNG",108,200);
    sprite.setTextColor(grays[1],blue2);
    sprite.drawString("SET",87,225);
    sprite.unloadFont();

    sprite.loadFont(smallFont);
    sprite.setTextColor(TFT_ORANGE,TFT_BLACK);
    sprite.drawString("SETUP",28,18);
    sprite.unloadFont();

 for(int i=0;i<8;i++)
 {
   if(chosen==i) tmpColor=grays[5]; else tmpColor=grays[9]; 

   if(i<6){
   sprite.fillRect(setPosX[i],setPosY[i],36,40,tmpColor);
   sprite.loadFont(smallFont);
   sprite.setTextColor(0x35D7,TFT_BLACK);
   sprite.drawString(setDataLbl[i],setPosX[i]+18,setPosY[i]-12);
   sprite.unloadFont();

    sprite.loadFont(secFont);
    sprite.setTextColor(grays[1],tmpColor);
    sprite.drawString(String(setData[i]),setPosX[i]+17,setPosY[i]+22);
    sprite.unloadFont();
   }
   else
   {
     sprite.setTextDatum(0);
     sprite.fillRect(setPosX[i],setPosY[i],76,16,tmpColor);
     sprite.setTextColor(grays[1],tmpColor);
     sprite.drawString(setDataLbl[i],setPosX[i]+4,setPosY[i]+5);
     if(i==6)
     if(setData[i]==0)
     sprite.drawString("EU",setPosX[i]+50,setPosY[i]+5);
     else
     sprite.drawString("NA",setPosX[i]+50,setPosY[i]+5);

     if(i==7)
     if(setData[i]==0)
     sprite.drawString("OFF",setPosX[i]+50,setPosY[i]+5);
     else
     sprite.drawString("ON",setPosX[i]+50,setPosY[i]+5);
   }
 }

 StickCP2.Display.pushImage(0,0,135,240,(uint16_t*)sprite.getPointer()); 
}


void loop()    //##########################  LOOP  ##################################
 {
 StickCP2.update();
  auto imu_update = StickCP2.Imu.update();
 auto data = StickCP2.Imu.getImuData();
 poz=map(data.accel.y*100,-30,100,0,10);
 if(setTimeDate==0)
 {
    
    vol = StickCP2.Power.getBatteryVoltage();
    volE=map(vol,3000,4180,0,5);
    auto dt = StickCP2.Rtc.getDateTime();

  if(digitalRead(35)==0 && setFunction==3)
  {
    setTimeDate=!setTimeDate;
    setData[0]=dt.time.hours;
    setData[1]=dt.time.minutes;
    setData[2]=dt.time.seconds;
    setData[3]=dt.date.date;
    setData[4]=dt.date.month;
    setData[5]=dt.date.year-2000;
    setData[6]=Myregion;
    setData[7]=buzzer;

    if(buzzer)
      StickCP2.Speaker.tone(6000, 100);
    delay(200);
  } 

  if(digitalRead(35)==0 && setFunction==4)
  {
    sleepTime=10;
    b++; if(b==6) b=0;
    EEPROM.write(0, b);
    EEPROM.commit();
    StickCP2.Display.setBrightness(brightnes[b]);
    if(buzzer)
    StickCP2.Speaker.tone(6000, 100);
  } 


  if(slp)
  {
    StickCP2.Display.setBrightness(brightnes[b]);
    slp=false;
    fase=-1;
    sleepTime=10; 
  }

if(fase==1){
if(rtc.getSecond()<10)  secc="0"+String(rtc.getSecond());  else secc=String(rtc.getSecond());
if(rtc.getMinute()<10)  minn="0"+String(rtc.getMinute());  else minn=String(rtc.getMinute());
if(rtc.getMillis()/10<10)  mill="0"+String(rtc.getMillis()/10);  else mill=String(rtc.getMillis()/10);
}

   

if (StickCP2.BtnB.wasPressed()) {
    setFunction++;
    sleepTime=10;
    if (setFunction >=setFunctionMax)
    {
      setFunction=0;
    }
  }

if (StickCP2.BtnA.wasPressed()) {
      sleepTime=10;
      if(fase==0 && poz==1){
      sendAllCodes();
      }
      else
     { fase++; if(fase==3) fase=0;
      if(fase==1)  rtc.setTime(0, 0, 0, 1, 1, 2023,0);
      if(fase==0){
       minn="00"; secc="00"; mill="00";
      }
       if(buzzer)
      StickCP2.Speaker.tone(6000, 100);
    }}

if(dt.time.seconds<10) s="0"+String(dt.time.seconds); else s=String(dt.time.seconds);
if(dt.time.minutes<10) m="0"+String(dt.time.minutes); else m=String(dt.time.minutes);

if(Myregion==1){
if(dt.time.hours>12){
if(dt.time.hours-12<10) h="0"+String(dt.time.hours-12); else h=String(dt.time.hours-12);
}else
{if(dt.time.hours<10) h="0"+String(dt.time.hours); else h=String(dt.time.hours);}}

if(Myregion==0)
if(dt.time.hours<10) h="0"+String(dt.time.hours); else h=String(dt.time.hours);

day=dt.date.date;
month=dt.date.month;

ts=dt.time.seconds;
if(tts!=ts && fase==0) {sleepTime--; tts=ts;}

if(sleepTime==0 && fase!=1)
{
slp=true;
StickCP2.Display.setBrightness(0);  
delay(20);
StickCP2.Power.lightSleep();
}


drawMain();}

if(setTimeDate)
{
  if (StickCP2.BtnB.wasPressed()) {
    chosen++; if(chosen==8) chosen=0;
    if(buzzer)
      StickCP2.Speaker.tone(6000, 100);
  }

   if (StickCP2.BtnA.wasPressed()) {
     setData[chosen]++;
     if(setData[chosen]==setMax[chosen])
     setData[chosen]=setMin[chosen];
     if(buzzer)
     StickCP2.Speaker.tone(6000, 100);
   }

  if(digitalRead(35)==0)
  {
    buzzer=setData[7];
    Myregion=setData[6];
    EEPROM.write(1, Myregion);
    EEPROM.write(2, buzzer);
       EEPROM.commit();
    sleepTime=10;
    setTimeDate=!setTimeDate;

   StickCP2.Rtc.setDateTime( { { setData[5]+2000, setData[4], setData[3] }, { setData[0],setData[1] , setData[2] } } );
   
    if(buzzer)
      StickCP2.Speaker.tone(6000, 100);
     delay(300);
    }

  drawSet();
}

}

void delay_ten_us(uint16_t us) {
  uint8_t timer;
  while (us != 0) {
    // for 8MHz we want to delay 80 cycles per 10 microseconds
    // this code is tweaked to give about that amount.
    for (timer=0; timer <= DELAY_CNT; timer++) {
      NOP;
      NOP;
    }
    NOP;
    us--;
  }
}

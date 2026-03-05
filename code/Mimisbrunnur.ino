/***************************************************
Mimisbrunnur E-book reader firmware
by Kári Össurarson

Based on the following software libraries, hardware components and
associated example code. All rights to their respective owners:

AdaFruit Feather RP2040 ThinkInk, part# 5727
Adafruit 5.83" Monochrome eInk display, part# 6397

Arduino Philhower core for RP2040
-Download manager: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
-Board manager: Raspberry Pi Pico/RP2040 by Earle F Philhower, III
Choose board: Tools>Board>Raspberry Pi RP2040 Boards>Adafruit Feather RP2040 ThinkINK

Adafruit EPD library

elapsedMillis library by Paul Stoffgren

Time library by Michael Margolis

Sd Fat library by Bill Greiman
*NOTE* Libray needs modification to use software SPI:
Change #define SPI_DRIVER_SELECT 0 to 2 in libraries/SdFat/src/SdFatConfig.h

The font files, FreeSans-13-1252.h, FreeSansOblique-13-1252, FreeSansBold-13-1252.h and FreeSansBoldOblique-13-1252,
are gfx renditions of the GNU FreeFont, proportional sans-serif version in regular, italics, bold and bold-italics,
downloaded from https://ftp.gnu.org/gnu/freefont/freefont-otf-20120503.tar.gz,
changed to 13 pixel BDF format using Font forge (https://fontforge.org/en-US/downloads/),
according to instructions from https://learn.adafruit.com/custom-fonts-for-pyportal-circuitpython-display/conversion,
(the instructions are for Circuit Python, but the idea was to get a BDF version of the font so it could be...)
further adapted to gfx format, manually cleaned up and augmented by code points 0x80 to 0x9F (CP1252) 
using GFX Font Editor (https://github.com/ScottFerg56/GFXFontEditor) by Forest Moon Productions.

The icons in icons.h were created with Greenfish Icon Editor Pro (https://greenfishsoftware.org/),
saved as .ico files, and converted to arduino code using https://javl.github.io/image2cpp/

****************************************************/

#include "Adafruit_ThinkInk.h"
#include <elapsedMillis.h>
#include <TimeLib.h>
#include "SdFat.h"
#include "FreeSans-13-1252.h"
#include "FreeSansOblique-13-1252.h"
#include "FreeSansBold-13-1252.h"
#include "FreeSansBoldOblique-13-1252.h"
#include "icons.h"
#include "strings.h"
#include "styles.h"

//eInk pin defines
#define EPD_DC PIN_EPD_DC       // ThinkInk 24-pin connector DC
#define EPD_CS PIN_EPD_CS       // ThinkInk 24-pin connector CS
#define EPD_BUSY PIN_EPD_BUSY   // ThinkInk 24-pin connector Busy
#define SRAM_CS -1              // use onboard RAM
#define EPD_RESET PIN_EPD_RESET // ThinkInk 24-pin connector Reset
#define EPD_SPI &SPI1           // secondary SPI for ThinkInk

// 5.83" Monochrome displays with 648 x 480 pixels and UC8179 chipset
// See Adafruit demo code for initializing other displays
ThinkInk_583_Mono_AAAMFGN display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

//SD card block
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 3
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 5;
// Pin numbers in templates must be constants.
const uint8_t SOFT_MISO_PIN = 4;
const uint8_t SOFT_MOSI_PIN = 3;
const uint8_t SOFT_SCK_PIN = 2;
// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI
SdFs sd;

const int volt_pin=28; //Battery voltage pin
const int battlow=3400; //Battery low threshold. Should be at least 3400mV, as readings get unreliable below that.
const int batthyst=50; //Battery low hysterisis. Operation will restart when voltage rises above battlow+batthyst
const int button_pins[]={29,14,15,0,1,25,24,26}; //Button pins
int button_state[8]; //Button state
int button_held[8]; //Set when a button has been detected as held
const int debounce_time=50;
const int hold_time=1000;
const int BUTTON_DOWN=1;
const int BUTTON_UP=2;
const int BUTTON_HELD=3;
elapsedMillis debounce;
elapsedMillis blinktime;
const int light_pin=12; //Light pin
const int lightval=96; //Value (0-255) when light is on
bool lighton=false; //True when light is on
const int statusheight=16; //Height of status bar
const int menuheight=41; //Height of menu bar
const int menulineY=21; //Y position of line through menu items, counting from bottom of display
const int menutextY=26; //Y position of baseline of menu text items, counting from bottom of display
const int menuiconY=18; //Y position of menu icon, counting from bottom of display
const int menuwidth=52; //Width of each menu item
const int menumargin=16; //Menu bar margins
const int margin=4; //Margin at top and bottom of page
const uint8_t FLAGFIRSTLINE=0x80; //Set when we are in the first line of a paragraph
const uint8_t FLAGV2=0x40; //Set for version 2 of files
uint8_t flags= FLAGFIRSTLINE | FLAGV2; //Assume we start in first line and use v2 files
FsFile file;
String fullpath="/";
uint32_t startpos=0; //File position of start of page
uint32_t endpos=0; //File position of start of next page
int linkcount=0; //Number of links on current page
String link[9]; //Array for link paths
int filesize=0;
bool nohistory=true; //Set by readpos when there is no history to backtract
const uint8_t FONT_NORMAL=0;
const uint8_t FONT_ITALIC=1;
const uint8_t FONT_BOLD=2;
const uint8_t FONT_BOLDITALIC=3;
const uint8_t FONT_BUILTIN=4;
uint8_t curfont=FONT_BUILTIN; //curfont should follow actual font set on display
bool styleprocessed=false; //Indicates whether <head></head> has been processed for ingisting CSS
const int PT_CSS=0;
const int PT_SCAN=1;
const int PT_OUTPUT=2;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // turn on Busy LED)

  fullpath.reserve(512);

  pinMode(light_pin, OUTPUT);
  
  //Set up buttons
  for (int i=0;i<8;i++) { 
    pinMode(button_pins[i], INPUT_PULLUP);
    button_state[i]=HIGH;
    button_held[i]=false;
  }

  //Global display initialization
  display.begin(THINKINK_MONO);
  display.setRotation(3); //Portrait orientation, with connector going to the left when viewed from the front
  display.setTextColor(EPD_BLACK);

  elapsedMillis serwait=0;
  while (!Serial && serwait<2000); //Wait up to 2 seconds for serial, then continue
  //Serial.printf("Flags=%d\n", flags);
  
  clearstylestack();
  
  battest(); //Check if battery is low
  readpos(false); //Read last record from position file

}

void loop() {
  //Display a page from fullpath, starting from position startpos
  //If fullpath contains a section reference it will be scanned for
  //Sets endpos to start of next page
  //A subloop at the end cycles until the page must be refreshed
  elapsedMillis timing=0;
  String section="";
  int sectionpos=fullpath.indexOf("#");
  if (sectionpos!=-1) {
    //Section reference found
    section=fullpath.substring(sectionpos+1);
    fullpath.remove(sectionpos);
  }
  //While fullpath is just a path, or if specified file cannot be opened, let user select another file
  while (fullpath.endsWith("/") || !fileopen(file, fullpath, O_READ)) {
    Serial.printf("Invalid file, path only=%d\n",fullpath.endsWith("/"));
    displayfiles(false);
  }
  timing=0;
  if (!styleprocessed) {
    processpage(PT_CSS,"");
    styleprocessed=true;
    Serial.printf("Ingested CSS in %lu mS\n", (unsigned long)timing);
    timing=0;
  }
  setfont(FONT_NORMAL);
  display.setTextSize(0);
  display.clearBuffer();
  writepos(false);
  //Serial.printf("Startpos before page=%d\n", startpos);
  if (section!="") { //If there is a section
    int temppos=startpos; //Remember current startpos
    startpos=file.size(); //Scan through whole file
    if (!processpage(PT_SCAN,section)) startpos=temppos; //Restore previous start position if not found
  }
  processpage(PT_OUTPUT, ""); //Output page, section is no longer relevant
  timing=0;
  digitalWrite(LED_BUILTIN,HIGH); //Make sure we leave the busy led on after all the blinking, there is still proccessing to be done
  //Serial.printf("Startpos after page=%d\n", startpos);
  filesize=file.size();
  file.close();
  Serial.printf("Loop: %s startpos %d\n",fullpath.c_str(),startpos);
  //Scan bookmarks
  int32_t bmfirstat=-1;
  int32_t bmfirstpos=0;
  int32_t bmnextat=-1;
  int32_t bmnextpos=0;
  int32_t bmthisat=-1;
  FsFile bmfile;
  if (bmfile.open((fullpath+".bookmarks").c_str(),O_READ)) {
    //Open bookmarks file, if present
    int32_t temppos;
    int32_t tempat;
    bmfile.seek(0);
    //Loop until end of file
    while (bmfile.available()) {
      tempat=bmfile.curPosition(); //Remember where record started
      temppos=readrecord(bmfile,false); //Read record
      //Abort if bookmarks file is bad
      if (temppos==-1) {
        Serial.println("Bad bookmarks file");
        bmthisat=-1; //No bookmarks reliably found
        bmfirstat=-1;
        bmnextat=-1;
        break;
      }
      if ((unsigned)temppos==startpos){ //Current page found
        bmthisat=tempat;
      } else if ((unsigned)temppos<startpos && (bmfirstat==-1 || temppos<bmfirstpos)) { //A new first position found
        bmfirstat=tempat;
        bmfirstpos=temppos;
      } else if (unsigned(temppos)>startpos && (bmnextat==-1 || temppos<bmnextpos)) { //A new next position found
        bmnextat=tempat;
        bmnextpos=temppos;
      }
    } //while (bmfile.available()) {
    bmfile.close();
    // Serial.printf("First position is %d at %d\n",bmfirstpos,bmfirstat);
    // Serial.printf("This position at %d\n",bmthisat);
    // Serial.printf("Next position is %d at %d\n",bmnextpos,bmnextat);
    //If no next bookmark found, use first bookmark (which may also be nonexistant)
    if (bmnextat==-1) {
      bmnextat=bmfirstat;
      bmnextpos=bmfirstpos;
    }
  } //if (bmfile.open((fullpath+".bookmarks").c_str(),O_READ)) {
  Serial.printf("Bookmarks processed in %lu mS\n", (unsigned long)timing);
  timing=0;
  //Draw status bar, switching to built in font
  drawstatusbar();
  display.setCursor(4,4);
  display.printf("%s %.1f%%",fullpath.substring(fullpath.lastIndexOf('/')+1).c_str(),100.0*startpos/filesize);
  //Draw menu bar
  drawmenubar(1,linkcount,1,":");
  drawmenuicon(0, iconprev);
  if (!nohistory) drawmenuicon(1, iconcancel);
  drawmenuicon(2, iconopen);
  if (bmthisat==-1) drawmenuicon(3, iconbmadd);
  else drawmenuicon(3,iconbmrem);
  if (bmnextat!=-1) drawmenuicon(4,iconbmgoto);
  drawmenuicon(5, iconskip);
  drawmenuicon(6, iconlight);
  drawmenuicon(7, iconnext);
  Serial.printf("Menu and status bars drawn in %lu mS\n", (unsigned long)timing);
  timing=0;
  display.display();
  //digitalWrite(LED_BUILTIN,LOW);
  Serial.printf("Displayed page in %lu mS\n", (unsigned long)timing);
  //Check buttons
  bool nextcycle=false; //False while scanning buttons, set true to display another page
  while (!nextcycle) {
    for (int button=0;button<8;button++) {
      //Scan through buttons
      int buttonval=checkbutton(button);
      if (buttonval==BUTTON_UP) {
        //Button released after short press
        if (button==0) {
          //Previous page
          scanpages("");
          //linestart=pagestart;
          nextcycle=true;
        }
        if (button==1 && !nohistory) {
          //Cancel / Link return
          readpos(true);
          styleprocessed=false;
          nextcycle=true;
        }
        if (button==2) {
          //Open file
          displayfiles(true);
          nextcycle=true;
        }
        if (button==3) {
          //Add/remove bookmark
          if (fileopen(bmfile, fullpath+ ".bookmarks", O_RDWR | O_CREAT)) {
            //Open bookmarks file, starting SD if needed
            if (bmthisat==-1) {
              //Not bookmarked yet, add one
              if (bmnextat==-1){
                Serial.println("Adding bookmark at start of bookmarks file");
                bmfile.seek(0); //Write to beginning if no valid bookmark in file
              } else {
                Serial.println("Adding bookmark at end of bookmarks file");
                bmfile.seekEnd(); //Else write to end
              }
              readpos(false); //Reset flags ond style stack to start of page
              writerecord(bmfile);
              bmfile.truncate();
            } else {
              //Already bookmarked, remove
              bmfile.seek(bmthisat); //Point to the record to be removed
              Serial.printf("Deleting bookmark at %d\n",bmthisat);
              readrecord(bmfile, false); //Point to the next record
              int32_t copyfrom=bmfile.curPosition(); //Remember place
              Serial.printf("Next bookmark at %d\n", copyfrom);
              int c=bmfile.read(); //Read byte
              //Loop until file empty
              while (c!=-1) {
                bmfile.seek(bmthisat); //Point to spot to write to
                bmfile.write(c);
                bmthisat++; //Increment spot to write to
                copyfrom++; //Increment spot to read from
                bmfile.seek(copyfrom); //Point to new spot to read from
                c=bmfile.read();
              }
              bmfile.truncate(bmthisat); //Cut off last record
            } //if (bmthisat==-1) {} else {
            bmfile.close();
          } //if (fileopen(bmfile, fullpath+ ".bookmarks", O_RDWR | O_CREAT)) {
          nextcycle=true;
        } //if (button==3) {
        if (button==4 && bmnextat!=-1) {
          //Goto next bookmark
          if (fileopen(bmfile, fullpath+ ".bookmarks", O_READ)) {
            //Open bookmarks file, starting SD if needed
            bmfile.seek(bmnextat);
            readrecord(bmfile,true);
            bmfile.close();
          }
          nextcycle=true;
        }
        if (button==5) {
          //Skip pages
          displayskip();
          nextcycle=true;
        }
        if (button==6) switchlight();
        if (button==7) {
          //Next page
          startpos=endpos;
          nextcycle=true;
        }
      } else if (buttonval==BUTTON_HELD && button<linkcount) {
        //Button has been held down for proscribed time and there is an associated link
        Serial.printf("Link %s\n",link[button].c_str());
        parsepath(fullpath, link[button]); //Parse link and modify fullpath
        flags|=FLAGFIRSTLINE; //Assume link starts on first line of paragraph
        startpos=0; //Will be updated by sectionseek() if there is one
        styleprocessed=false;
        writepos(true); //PUSH to position file
        clearstylestack();
        nextcycle=true;
      } // else if (buttonval==BUTTON_HELD && button<linkcount) {
    } //for (int button=0;button<8;button++) {
  } //while (!nextcycle) {
  //Serial.printf("Startpos after selection=%d\n", startpos);
} //void loop() {

//Parses relative path relpath and modifies fullpath
void parsepath(String &fullpath, String &relpath) {
  //If this is not just a section reference within same file
  if (!relpath.startsWith("#")) {
    fullpath.remove(fullpath.lastIndexOf('/')+1); //Remove filename from current path
    if (relpath.startsWith("/")) {
      //This link is absolute to root
      fullpath="/";
      relpath.remove(0,1); //Remove root reference
    }
    while(relpath.startsWith("../")) {
      //Scan through any parent directories
      if (fullpath!="/") {
        //Cant do parent directory if already in root
        fullpath.remove(fullpath.length()-1); //Remove trailing slash
        fullpath.remove(fullpath.lastIndexOf("/")+1); //Remove directory
      }
      relpath.remove(0,3); //Remove parent directory reference from link
    }
  }
  fullpath+=relpath; //Append what's left of the link to fullpath
}

//Scans document looking for previous page or page with section reference
void scanpages(String section) {
  if (!fileopen(file, fullpath, O_READ)) {
    return; //We can do nothing if we cannot open the file
  }
  int pages=0;
  elapsedMillis scantime;
  uint32_t target=startpos; //Target for endpos
  Serial.printf("Scanpages: Target=%d\n",target);
  startpos=0;
  setfont(FONT_NORMAL);
  display.setTextSize(0);
  flags|=FLAGFIRSTLINE; //First line of first page is first line of paragraph
  uint8_t saveflags=flags;
  clearstylestack();
  stylemember savestack[stylestackmaxcount];
  uint8_t savecount=stylestackcount;
  //Initialize savestack
  strcpy(savestack[0].tag,stylestack[0].tag);
  stylecopy(savestack[0].value,stylestack[0].value);
  if (processpage(PT_SCAN, section)) return; //Process first page to get endpos, return if section found
  while(endpos<target && startpos!=endpos) {
    //Repeat until endpos>=target or startpos==endpos (indicating no progress)
    //Serial.printf("Startpos=%d endpos=%d target=%d\n",startpos,endpos,target);
    saveflags=flags; //Save flags and style stack
    savecount=stylestackcount;
    for (int i=0;i<savecount;i++) {
      strcpy(savestack[i].tag,stylestack[i].tag);
      stylecopy(savestack[i].value,stylestack[i].value);
    }
    startpos=endpos; //Move on to next page
    if (processpage(PT_SCAN, section)) return; //Process next page, return ig section found
    pages++;
  }
  flags=saveflags; //Restore flags and style stack to beginning of page
  stylestackcount=savecount;
  for (int i=0;i<savecount;i++) {
    strcpy(stylestack[i].tag,savestack[i].tag);
    stylecopy(stylestack[i].value,savestack[i].value);
  }
  file.close();
  Serial.print(pages);
  Serial.printf(" pages scanned in %lu mS\n", (unsigned long)scantime);
  Serial.printf("After scan, startpos=%d  endpos=%d\n",startpos,endpos);
} //void scanpages() {

void displayskip() {
  //Displays skip bar
  digitalWrite(LED_BUILTIN,HIGH);
  display.fillRect(0,display.height()-menuheight,display.width(),menuheight,EPD_WHITE); //Clear previous menubaar
  drawmenubar(0,98,14,"%");
  drawmenuicon(1, iconcancel);
  drawmenuicon(6, iconlight);
  display.display();
  //Check buttons
  while (true) {
    for (int button=0;button<8;button++) {
      //Scan through buttons
      int buttonval=checkbutton(button);
      if (buttonval==BUTTON_UP) {
        //Button released after short press
        if (button==1) return; //Cancel, return without modifying anything
        if (button==6) switchlight();
      } else if (buttonval==BUTTON_HELD) {
        //Button has been held down for proscribed time
        Serial.printf("Filesize %d\n",filesize);
        startpos=filesize*button*.14;
        Serial.printf("Start position %d\n",startpos);
        scanpages("");
        writepos(true); //PUSH to position file
        return;
      }
    } //for (int button=0;button<8;button++) {
  } //while (true) {
} //void displayskip() {

//Appends val to buffer and flags from latest stylestack style to stylebuffer, updating bufwidth and curstyle
inline void bufferadd(String &buffer, String &stylebuffer, int &bufwidth, int val, int16_t curstyle[]) {
  stylecopy(curstyle,stylestack[stylestackcount-1].value);
  buffer+=char(val);
  stylebuffer+=char(curstyle[SVR_FLAGS]);
  bufwidth+=charadvance(getflag(SFR_FONT,curstyle[SVR_FLAGS]),val);
}

//Sets the display font if different from curfont, updates curfont
inline void setfont(uint8_t newfont) {
  if (newfont!=curfont){
    switch (newfont) {
      case FONT_NORMAL:
        display.setFont(&_FreeSans_);
        break;
      case FONT_ITALIC:
        display.setFont(&_FreeSansOblique_);
        break;
      case FONT_BOLD:
        display.setFont(&_FreeSansBold_);
        break;
      case FONT_BOLDITALIC:
        display.setFont(&_FreeSansBoldOblique_);
        break;
      case FONT_BUILTIN:
        display.setFont();
    }
    curfont=newfont;
  }
}

//Displays buffer on screen, using stylebuffer and curstyle.
//dX is horizontal starting position, dY is text baseline, 
//bufferwidth is width of buffer in pixels and displaywidth also in pixels .
void displaytext(int &dX, int &dY, String &buffer, String &stylebuffer, int bufferwidth, int displaywidth, int16_t curstyle[]) {
  Serial.printf("First line=%d, dX=%d :%s\n",flags & FLAGFIRSTLINE,dX,buffer.c_str());
  //Existing dX is ignored for center and right alignment
  if (getflag(SFR_RIGHT,curstyle[SVR_FLAGS])) dX=displaywidth-curstyle[SVR_RIGHTMARGIN]-bufferwidth;
  if (getflag(SFR_CENTER,curstyle[SVR_FLAGS])) 
          dX=(displaywidth-curstyle[SVR_RIGHTMARGIN]-curstyle[SVR_LEFTMARGIN]-bufferwidth)/2;
  display.setCursor(dX,dY); //Set start of line
  int understart=dX;
  int underend;
  for (uint32_t i=0;i<buffer.length();i++) {
    setfont(getflag(SFR_FONT,stylebuffer[i]));
    display.print(buffer[i]);
    underend=understart+charadvance(curfont, buffer[i]);
    if (getflag(SFR_UNDERLINE,stylebuffer[i])) display.drawLine(understart,dY,underend,dY,EPD_BLACK);
    if (getflag(SFR_STRIKE,stylebuffer[i])) display.drawLine(understart,dY-5,underend,dY-5,EPD_BLACK);
    understart=underend;
  }
}

//PUSHes stag to stylestack if it isn't already there. Does nothing if stack is full or tag is too long (>=16 letters).
void pushstyle(String tag) {
  if (stylestackcount<stylestackmaxcount && tag.length()<tagsize && strcmp(stylestack[stylestackcount-1].tag,tag.c_str())!=0) {
    strcpy(stylestack[stylestackcount].tag, tag.c_str()); //Store tag
    stylecopy(stylestack[stylestackcount].value,stylestack[stylestackcount-1].value); //Preserve style for now
    stylestackcount++;
    //Serial.printf("Push style stack, count=%d, tag=%s\n",stylestackcount,tag.c_str());
  }
}

//Copies values from source value array to dest value array
inline void stylecopy(int16_t dest[], int16_t source[]) {
  //Array size is always stecount, no need to pass it as a seperate parameter
  for (int i=0;i<stecount;i++) dest[i]=source[i];
}

//Returns true if dY is within bounds of display.
inline bool withinbounds(int dY) {
  return dY<=display.height()-menuheight-descender-margin; //
}

//Processes a page from file according to precesstype. PT_CSS (0) scans from start of file to end of <head> section
//in order to ingest stylesheets, but otherwise doesn't update values. PT_SCAN (1) scans from startpos to end of page
//and updates working values without displaying the page. PT_OUTPUT (2) is same as PT_SCAN, except actually outputs
//page to display. Returns true if section reference found on page.
bool processpage(int processtype, const String &section) {
  // Serial.printf("Before page: Flags=%d, processtype=%d\n",flags,processpage);
  // Serial.println("Style stack at start of page:");
  // for (int i=0;i<stylestackcount;i++) {
  //   Serial.printf(" tag=%s",stylestack[i].tag);
  //   Serial.printf(", style=%x\n",stylestack[i].style);
  if (processtype==PT_CSS) {
    stylelistcount=0;
    clearnewstylelistmember();
  }
  elapsedMillis processtime=0;
  int16_t curstyle[stecount];
  stylecopy(curstyle,stylestack[stylestackcount-1].value);
  bool foundsection=false;
  int dX=0;
  int dY=statusheight+lineheight-descender+margin;
  int dW=display.width();
  int16_t val=0;
  String wordbuf=""; //Holds current word
  String wordstyle=""; //Holds style flags for current word
  wordbuf.reserve(64);
  wordstyle.reserve(64);
  int wordwidth=0; //Pixel width of word in buffer
  String linebuf="";
  String linestyle=""; //Holds style flags for current line
  linebuf.reserve(128);
  linestyle.reserve(128);
  int linewidth=0; //Pixel width of line
  String textbuf=""; //Intermediate buffer to hold text to be processed when next in plain mode
  textbuf.reserve(128);
  const int RMTEXT=1; //Used when reading text file
  const int RMPLAIN=2; //Used when reading (x)html outside tags and escape sequences
  const int RMESC=3; //Used when reading escape sequences
  const int RMTAG=4; //Used when reading tag names
  const int RMATTRIB=5; //Used when reading tag attribute names
  const int RMVALUE=6; //Used when reading tag attribute values
  const int RMSTYLENAME=7; //Used when reading style names
  const int RMSTYLEPROPERTY=8; //Used when reading style property names
  const int RMSTYLEVALUE=9; //Used when reading style property values
  const int RMSTYLECOMMENT=10; //Used when reading style comments
  int readmode=RMPLAIN;
  if (fullpath.endsWith(".txt") || fullpath.endsWith(".TXT")) readmode=RMTEXT;
  String tag=""; //Holds current tag name, sometimes in lower case
  char qtype=' '; //Specifies type of quote being used when reading quoted value, space means unquoted, null means not applicable
  char styleqtype=' '; //Same as qtype except for use in style definitions
  tag.reserve(16);
  String escseq=""; //Escape sequence
  escseq.reserve(16);
  String attrib=""; //Attribute name
  attrib.reserve(16);
  String value=""; //Attribute value
  value.reserve(256);
  String stylename=""; //Style name
  stylename.reserve(16);
  String styleproperty=""; //Style property name
  styleproperty.reserve(16);
  String stylevalue=""; //Style property value
  stylevalue.reserve(16);
  int stylevalnum=0; //Style properties can have several values, this is used to id them
  int stylelistindex=stylelistcount; //Index of style list being modified
  linkcount=0;
  bool inhead=false; //True when in head
  bool attribcomplete=false; //Signifies that attribute name is complete and unless next non-whitespace character is = , it is the start of a new attribute
  int maxwidth;
  bool dowordbreak=false; //Signifies that word break should be processed
  bool dolinebreak=false; //Signifies that line break should be processed
  bool dohr=false; //Signifies that horizontal rule should be processed
  bool dotag=false; //Signifies that the tag should be processed
  bool doattrib=false; //Signifies that attribute should be processed
  int callingmode=0; //Used to identify mode that called another
  int prevbottom=0; //Used to hold bottom margin from previous paragraph, for potential collapsing with next top margin
  bool linkstylesheet=false; //Set when rel="stylesheet" attribute found in link tag
  String linkhref=""; //Holds the href attribute value for link tag
  bool extstylesheet=false; //Set to true while processing external style sheet
  stylemember linestack[stylestackmaxcount]; //Holds style stack at start of line, in case of orphaned lines
  uint8_t linestackcount=stylestackcount;
  copystack(linestack,stylestack,stylestackcount);
  stylemember wordstack[stylestackmaxcount]; //Holds style stack at start of line, in case of orphaned lines
  uint8_t wordstackcount=stylestackcount;
  copystack(wordstack,stylestack,stylestackcount);
  int extpos=0; //Used to remember where we were in document while reading external stylesheet
  if (processtype==PT_CSS) file.seek(0); //Go to start of file
  else file.seek(startpos); //Go to start of page
  int wordpos=file.curPosition(); //Holds position at start of word
  val=Uto1252(file.read()); //Set working value to first byte of page
  while (val!=-1 && withinbounds(dY) && linkcount<=8) {
    //Not end of file, within bounds of the display and managable link count
    //Values that should be processed as appropriate every cycle, and therefore reset every cycle:
    dowordbreak=false; //Signifies that word break should be processed
    dolinebreak=false; //Signifies that line break should be processed
    dohr=false; //Signifies that horizontal rule should be processed
    dotag=false; //Signifies that the tag should be processed
    doattrib=false; //Signifies that attribute should be processed
    if (blinktime>50) {
      digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN)); //Change busy led status
      blinktime=0;
    }
    if (val=='\r') val=0; //CR, always ignored
    // Serial.printf("Value=%d",val);
    // Serial.printf("=%c",char(val));
    // Serial.printf(" mode=%d",readmode);
    // Serial.printf(" inhead=%d\n",inhead);
    switch (readmode) {
      case RMTEXT:
        //Text file
        switch (val) {
          case '\t':
            //TAB, insert 7 spaces and mark wordbreak, will also be processed under RMPLAIN
            for (int i=0;i<7;i++) bufferadd(wordbuf,wordstyle,wordwidth,' ',curstyle);
            break;
          case '\n':
            //LF, will also be processed under RMPLAIN
            dolinebreak=true;            
        }
        [[fallthrough]];
      case RMPLAIN:
        //Plaintext, also for text files
        switch (val) {
          case '\t': case '\n': case ' ':
            //Whitespace
            if (!inhead && processtype!=PT_CSS) dowordbreak=true;
            val=0;
            break;
          case '<':
            if (readmode!=RMTEXT) {
              //Start of tag
              readmode=RMTAG;
              tag="";
              val=0;
              //Serial.println("Switching to RMTAG");
            }
            break;
          case '&':
            if (readmode!=RMTEXT) {
              //Start of escape sequence
              readmode=RMESC;
              escseq="";
              val=0;
            }
        } //switch (val) {
        if (!inhead && val!=0) {
          if (val<' ') val=0x7F; //Unhandled control code, display as unknown character
          bufferadd(wordbuf,wordstyle,wordwidth,val,curstyle);
        }
        val=0;
        break;
      case RMESC:
        //Reading escape sequence
        if (val==';') {
          //End of escape sequence
          readmode=RMPLAIN; //Return to plain mode
          bufferadd(wordbuf,wordstyle,wordwidth,char(escape(escseq)),curstyle);
        } else {
          escseq+=char(val);
        }
        val=0;
        break;
      case RMTAG:
        //Reading tag name
        switch (val) {
          case 0: break; //Ignore 0
          case '\t': case '\n': case ' ':
            //Whitespace
            if (tag!="") {
              //We'll allow leading spaces before the tag name, though it isn't according to standard
              tag.toLowerCase();
              stylelistindex=stylelistfind(tag); //Check if tag is in style list
              if (stylelistindex!=-1) {
                Serial.printf("Found style: tag=%s\n",tag.c_str());
                pushstyle(tag);
                applystyle(stylelistindex);
              }
              readmode=RMATTRIB;
              attrib="";
              //Serial.println("Switching to RMATTRIB");
            }
            break;
          case '>': //End of tag string
            if (tag!="") {
              tag.toLowerCase();
              stylelistindex=stylelistfind(tag); //Check if tag is in style list
              if (stylelistindex!=-1) {
                Serial.printf("Found style: tag=%s\n",tag.c_str());
                pushstyle(tag);
                applystyle(stylelistindex);
              }
            }
            dotag=true; //Signifies that the tag should be processed
            break;
          default:
            tag+=char(val);
        } //switch (val) {
        val=0;
        break;
      case RMATTRIB:
        //Reading attribute name
        switch (val) {
          case 0: //Ignore
            val=0;
            break;
          case '\t': case '\n': case ' ':
            //Whitespace
            if (attrib!="") {
              //Allow leading spaces before the attribute name
              attribcomplete=true; //Attribute name is complete, but may be followed by value
              attrib.toLowerCase();
              //Serial.println("Attribute complete");
            }
            val=0;
            break;
          case '>': //End of tag string
            if (attrib!="") doattrib=true; //Signifies that the attribute should be processed
            dotag=true; //Signifies that the tag should be processed
            val=0;
            //Serial.println("End of tag");
            break;
          case '=':
            //Equals
            readmode=RMVALUE;
            qtype=' ';
            val=0;
            //Serial.println("Switching to RMVALUE");
            break;
          default:
            if (attribcomplete) {
              //This is start of another attribute
              doattrib=true; //Process the previous one
              attribcomplete=false; //This one isn't complete though
              //Serial.println("Start of new attribute");
            } else {
              attrib+=char(val);
              val=0;
            }
        } //switch (val) {
        break;
      case RMVALUE:
        //Reading attribute value
        switch (val) {
          case 0: break; // Invalid/ignored
          case '>': //End of tag string
            if (qtype==' ') {
              //Unquoted value
              if (attrib!="") doattrib=true; //Signifies that the attribute should be processed
              dotag=true; //Signifies that the tag should be processed
              //Serial.println("End of tag");
            } else {
              value+=char(val); //Add to string if inside quotes
            }
            val=0;
            break;
          case '\t': case '\n': case ' ':
            //Whitespace
            if (value!="" && qtype==' ') {
              //Allow whitespace before the attribute value and inside quotes
              doattrib=true; //Attribute value is complete, process attribute
            } else {
              value+=char(val); //Add to string if inside quotes
            }
            val=0;
            break;
          case '\"': case '\'':
            //Quotes
            if (qtype==' ') {
              //Quote type not set
              qtype=val;
            } else {
              if (qtype==val) {
                //End of value
                doattrib=true; //Attribute value is complete, process attribute
              } else {
                value+=char(val); //Add to buffer if inside quotes and not matching quote type
              }
            }
            val=0;
            break;
          default:
            if (attrib=="style") {
              readmode=RMSTYLEPROPERTY; //Switch to reading style property name
              clearnewstylelistmember();
              stylelistindex=stylelistcount;
              styleproperty="";
            } else {
              value+=char(val);
              val=0;
            }
        } //switch (val) {
        break;
      case RMSTYLENAME: //Reading style name
        switch(val) {
          case 0: case '\t': case '\n': case ' ': break; //Ignore
          case '{':
            //Start of style definition
            readmode=RMSTYLEPROPERTY; //Start reading style property
            qtype='\0'; //Quotes do not apply in CSS, only in inline styles
            //Serial.printf("Style %s",stylename.c_str());
            stylelistindex=stylelistfind(stylename);
            if (stylelistindex==-1) {
              stylelistindex=stylelistcount;
              if (stylelistcount<stylelistmaxcount-1) {
                stylelistcount++;
                clearnewstylelistmember();
                //Serial.println(" added.");
              } //else {
                //Serial.println(" not added.");
              //}
              stylelist[stylelistindex].name=stylename;
            } //else {
              //Serial.println(" found.");
            //}
            break;
          case '<':
            //End of internal style sheet, return to reading tag (probably </style>)
            readmode=RMTAG;
            tag="";
            break;
          case '/':
            //Start of style comment; we'll ignore the asterisk as / is illegal here for other purposes anyway
            readmode=RMSTYLECOMMENT;
            callingmode=RMSTYLENAME;
            break;
          default:
            stylename+=char(val);
        } // switch(val) {
        val=0;
        break;
      case RMSTYLEPROPERTY: //Reading style property name
        switch(val) {
          case 0: break; // Invalid/ignored
          case '\t': case '\n': case ' ':
            //Whitespace
            if (qtype==' ') doattrib=true; //if the (inline) style definition isn't enclosed in quotes, end of HTML attribute
            //Otherwise ignore
            break;
          case ':':
            //Start of style property value
            readmode=RMSTYLEVALUE;
            styleqtype=' '; //Not set
            stylevalnum=0;
            break;
          case '\'': case '\"':
            //Quotes
            if (val==qtype) doattrib=true; //If quote matches, end of HTML attribute
            break;
          case '}':
            //End of style definition
            readmode=RMSTYLENAME; //Return to reading style name, shouldn't happen with inline styles
            stylename="";
            break;
          case '/':
            //Start of style comment; we'll ignore the asterisk as / is illegal here for other purposes anyway
            readmode=RMSTYLECOMMENT;
            callingmode=RMSTYLEPROPERTY;
            break;
          default:
            styleproperty+=char(val);
        } // switch(val) {
        val=0;
        break;
      case RMSTYLEVALUE:
        //Reading style property value
        switch(val) {
          case 0: break; // Invalid/ignored
          case '\t': case '\n': case ' ': case ',':
            //Whitespace or comma
            if (qtype==' ') { //If the (inline) style definition isn't enclosed in quotes,
              doattrib=true; //end of HTML attribute
            } else if (styleqtype==' ') {
              //If outside quotes
              if (stylevalue!="") {
                //Ignore if leading, otherwise end of value
                processstyleproperty(stylelistindex,styleproperty,stylevalue,stylevalnum);
                stylevalue=""; //Preapare to read next style attribute value
                stylevalnum++;
              }
            } else {
              //Inside quotes
              stylevalue+=char(val);
            }
            break;
          case '\'': case '\"':
            //Quotes
            if (val==qtype) {
              doattrib=true; //If quote matches, end of HTML attribute
            } else {
              if (styleqtype==' ') { //If styleqtype has not been set
                styleqtype=val; //Set styleqtype
              } else {
                if (styleqtype==val) { //If styleqtype has been matched
                  //End of value
                  processstyleproperty(stylelistindex,styleproperty,stylevalue,stylevalnum);
                  stylevalue=""; //Preapare to read next style attribute value
                  stylevalnum++;
                  styleqtype=' '; //Clear styleqtype
                } else {
                  stylevalue+=char(val);
                }
              }
            }
            break;
          case ';':
            if (styleqtype==' ') {
              //End of value definitions for this property
              processstyleproperty(stylelistindex,styleproperty,stylevalue,stylevalnum);
              readmode=RMSTYLEPROPERTY; //Scan for next property            
              styleproperty="";
              stylevalue="";
              stylevalnum=0;
            } else {
              //Allow inside quotes
              stylevalue+=char(val);
            }
            break;
          case '/':
            if (styleqtype!=' ') {
              //Allow inside quoted values
              stylevalue+=char(val);
            } else {
              //Start of style comment; we'll ignore the asterisk as / is illegal here for other purposes anyway
              readmode=RMSTYLECOMMENT;
              callingmode=RMSTYLEVALUE;
            }
            break;
          default:
            stylevalue+=char(val);
        } // switch(val) {
        val=0;
        break;
      case RMSTYLECOMMENT:
        if (val=='/') readmode=callingmode; //End of comment, return to previous mode
        val=0; //Ignore everything in comments
    } //switch (readmode) {
    // if (doattrib || dotag) {
    //   if (tag.length()==2 && tag.startsWith("h") && tag[1]>='0' && tag[1]<='9') tag="h"; //Special case for hX tags, store only the h
    //   if (tag.length()==3 && tag.startsWith("/h") && tag[2]>='0' && tag[2]<='9') tag="/h"; //Allows for mismatched opening and closing tags
    // }
    if (doattrib) {
      //Here we process attributes
      // Serial.printf("Processing attribute: tag=%s",tag.c_str());
      // Serial.printf(" attribute=%s",attrib.c_str());
      // Serial.printf(" value=%s\n",value.c_str());
      // if (value!="") Serial.printf("=%s\n",value.c_str());
      // else Serial.println();
      if ((attrib=="id" || attrib=="name") && section!="" && value==section) {
        foundsection=true; //Section found
        Serial.printf("Section found in %lu mS\n",(unsigned long)processtime);
        processtime=0;
      } 
      if (processtype!=PT_CSS) { //Do not process most attributes when scanning for CSS
        if (attrib=="align"){
          //align attribute is global and can apply to any tag
          value.toLowerCase();
          pushstyle(tag);
          if (value=="center") setflag(SFR_CENTER,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
          if (value=="right") setflag(SFR_RIGHT,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
        } else if (tag=="a" && attrib=="href") {
          //href attribute of link tag
          linkcount++;
          link[linkcount-1]=value;
          textbuf+=String(linkcount)+": ";
          pushstyle(tag);
          setflag(SFR_UNDERLINE,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
        } else if (tag=="content" && attrib=="src" && linkcount>0) {
          //src attribute of content tag in .ncx file
          //The link count should already have been extended by navPoint tag
          link[linkcount-1]=value;
        } else if (tag=="img" && attrib=="alt") {
          //alt attribute of image tag
          textbuf+=" "+value+" ";
        } else if (attrib=="style") {
          pushstyle(tag);
          applystyle(stylelistindex); //Use the style we have been reading
        } else if (attrib=="class") {
          //Serial.printf("class=%s\n",value.c_str());
          value="."+value;
          stylelistindex=stylelistfind(value); //Search for .class in style list
          if (stylelistindex==-1) { //If not found, try tag.class
            value=tag+value;
            stylelistindex=stylelistfind(value);
          }
          if (stylelistindex!=-1) { //If either found, push stack
            Serial.printf("Found style: tag=%s", tag.c_str());
            Serial.printf(" class=%s\n",value.c_str());
            pushstyle(tag);
            applystyle(stylelistindex);
          }
        } else if (attrib=="/" && strcmp(stylestack[stylestackcount-1].tag,tag.c_str())==0 && stylestackcount>1) {
          //Self closing tag, has been pushed to stylestack, and stack more than one tag deep, pop stylestack)
          stylestackcount--;
          Serial.printf("Pop style stack, self closing tag=%s\n",tag.c_str());
          // Serial.printf(" now style=%x",stylestack[stylestackcount-1].style);
          // Serial.printf(" and count=%d\n",stylestackcount);
        }
      } else if (tag=="link") {
        // Serial.printf("<link %s",attrib.c_str());
        // Serial.printf("=\"%s\">\n",value.c_str());
        if (attrib=="rel" && value=="stylesheet") linkstylesheet=true;
        if (attrib=="href") {
          linkhref=fullpath;
          parsepath(linkhref,value);
        }
      }
      attrib="";
      value="";
      doattrib=false;
      attribcomplete=false;
      readmode=RMATTRIB;
      //Serial.println("Switching to RMATTRIB");
    }
    if (dotag) {
      //Here we process tags
      tag.toLowerCase(); //Simplify comparisons
      readmode=RMPLAIN; //Return to plain read mode after processing tag
      // if (seeksection) {
      //   startpos=file.position();
      // }
      //Serial.printf("Processing tag=%s",tag.c_str());
      //Serial.printf(", current style tag=%s\n",stylestack[stylestackcount-1].tag);
      if (tag.startsWith("head")) inhead=true; //Catch <head>
      else if (tag=="navpoint") { //Catch navPoint tag in .ncx file
        linkcount++;
        textbuf+=String(linkcount)+": ";
        pushstyle(tag);
        setflag(SFR_UNDERLINE,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
      }
      if (tag.startsWith("/")) {
        //Closing tag
        tag.remove(0,1); //Slice off that leading slash
        if (tag.startsWith("head")) { //Catch </head>
          inhead=false;
          if (processtype==PT_CSS) return false; //Exit function immediately, no section in CSS
        }
        if (strcmp(stylestack[stylestackcount-1].tag,tag.c_str())==0 && stylestackcount>1) {
          //Matches the last tag in stylestack, and stack more than one tag deep, pop stylestack)
          stylestackcount--;
          Serial.printf("Pop style stack, closing tag=%s\n",tag.c_str());
          // Serial.printf(" now style=%x",stylestack[stylestackcount-1].style);
          // Serial.printf(" and count=%d\n",stylestackcount);
        }
      } else if (processtype!=PT_CSS) { //Not scannig for CSS
        if  (tag=="b" || tag=="big" || tag=="strong" || (tag.startsWith("h") && tag.length()==2 && tag[1]>='0')) {
          //Bold
          pushstyle(tag);
          setflag(SFR_BOLD,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
        } else if (tag=="cite" || tag=="i" || tag=="address" || tag=="var" || tag=="em") {
          // Italics
          pushstyle(tag);
          setflag(SFR_ITALIC,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
        } else if (tag=="caption" || tag=="center") {
          //Center
          pushstyle(tag);
          setflag(SFR_CENTER,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
        } else if (tag=="ins" || tag=="mark" || tag=="u") {
          //Underline
          pushstyle(tag);
          setflag(SFR_UNDERLINE,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
        } else if (tag=="del" || tag=="s" || tag=="strike") {
          //Strikethrough
          pushstyle(tag);
          setflag(SFR_STRIKE,stylestack[stylestackcount-1].value[SVR_FLAGS],1);
        } else if (tag=="hr") {
          //Horizontal rule
          dowordbreak=true;
          dolinebreak=true; //Force line break
          dohr=true;
        }
      } else if (tag=="style" && inhead) {
        //Style tag in head and scanning for CSS
        readmode=RMSTYLENAME; //Switch to ingesting styles
        stylename="";
      } else if (tag=="link" && inhead && linkstylesheet) {
        //External stylesheet
        //Serial.printf("External stylesheet %s\n",linkhref.c_str());
        extpos=file.position(); //Remember place
        file.close(); //Close document
        fileopen(file, linkhref, O_READ); //Open stylesheet instead
        file.seek(0); //Start at top
        readmode=RMSTYLENAME; //Switch to ingesting styles
        stylename="";
        extstylesheet=true;
        linkstylesheet=false;
      }
      if (processtype!=PT_CSS && (tag=="blockquote" || tag=="br" || tag=="code" ||
                tag=="div" || tag=="dd" || tag=="dl" || tag=="dt" || tag=="h" || tag=="li" ||
                tag=="menu" || tag=="nav" || tag=="ol" || tag=="p" || tag=="samp" || tag=="section" ||
                tag=="/ul" || tag=="navpoint")) {
        //End of paragraph, some of these tags may already have been processed in another manner
        //Closing tags have already had the leading slash sliced off, reducing the number of comparisons
        //We don't really care whether they are opening or closing for this purpose
        //Only single line breaks between paragraphs will be performed, even if multiple are ordered
        dowordbreak=true;
        dolinebreak=true; //Force line break
      }
      tag="";
      attrib="";
      value="";
      dotag=false;
      doattrib=false;
      attribcomplete=false;
    }
    if (dowordbreak) {
      //If word break
      maxwidth=dW;
      if (flags & FLAGFIRSTLINE) maxwidth-=curstyle[SVR_INDENT];
      maxwidth-=curstyle[SVR_RIGHTMARGIN];
      maxwidth-=curstyle[SVR_LEFTMARGIN];
      if (linewidth+charadvance(getflag(SFR_FONT, linestyle[linestyle.length()-1]), ' ')+wordwidth>=maxwidth) { 
        //If line will extend off screen we must output the line and move to next
        dX=curstyle[SVR_LEFTMARGIN];
        if (flags & FLAGFIRSTLINE) {
          //Adjust position for first line
          dX+=curstyle[SVR_INDENT]; //Add indent
          //Collapse margins
          if (abs(prevbottom)>abs(curstyle[SVR_TOPMARGIN])) dY+=prevbottom;
          else dY+=curstyle[SVR_TOPMARGIN];
        }
        //If we are still within bounds
        if (withinbounds(dY)) {
          //Output the line if so commanded
          if (processtype==PT_OUTPUT) displaytext(dX, dY,linebuf,linestyle,linewidth,dW,curstyle); 
          endpos=wordpos; //Update end position to start of orphaned word
          copystack(linestack,wordstack,wordstackcount); //Save wordstack as linestack
          linestackcount=wordstackcount;
          flags &= ~FLAGFIRSTLINE; //Clear firstline flag for next line
          //Serial.printf("After automatic linebreak, flags=%d\n",flags);
          linebuf=""; //Clear line buffer
          linestyle="";
          linewidth=0;
          dY+=lineheight;
        } else {
          copystack(stylestack,linestack,linestackcount); //Revert style stack to beginning of line
          stylestackcount=linestackcount;
        }
      } else {
        //Line will *not* extend beyond boundaries
        if (linewidth!=0 && wordwidth!=0) {
          //There is content in both line and word
          linebuf+=' '; //Add space to line
          linestyle+=linestyle[linestyle.length()-1]; //Use styleflags from last character in line
          //Except for underline and strikethrough if end and start of word do not agree
          if (!(wordstyle[0] & SFR_UNDERLINE.mask)) linestyle[linestyle.length()-1]&=~SFR_UNDERLINE.mask;
          if (!(wordstyle[0] & SFR_STRIKE.mask)) linestyle[linestyle.length()-1]&=~SFR_STRIKE.mask;
          linewidth+=charadvance(getflag(SFR_FONT,linestyle[linestyle.length()-1]),' ');
        }
      }
      //Always performed in dowordbreak
      linebuf+=wordbuf; //Add word to line
      linestyle+=wordstyle;
      linewidth+=wordwidth;
      wordbuf=""; //Initialize next word
      wordstyle="";
      wordwidth=0;
      copystack(wordstack,stylestack,stylestackcount); //Save stylestack at start of word
      wordstackcount=stylestackcount;
      wordpos=file.curPosition(); //Update to start of next word
    } // if (dowordbreak) {
    if (dolinebreak && linewidth!=0) {
      //Forced linebreak with contents in line, performed after adding word to line buffer
      dX=curstyle[SVR_LEFTMARGIN];
      if (flags & FLAGFIRSTLINE) {
        //Adjust position for first line
        dX+=curstyle[SVR_INDENT]; //Add indent
        //Collapse margins
        if (abs(prevbottom)>abs(curstyle[SVR_TOPMARGIN])) dY+=prevbottom;
        else dY+=curstyle[SVR_TOPMARGIN];
      }
      if (withinbounds(dY)) { //If still within bounds
        if (processtype==PT_OUTPUT) displaytext(dX,dY,linebuf,linestyle,linewidth,dW,curstyle); //Output the line if so commanded
        endpos=file.curPosition(); //Update page end position
        copystack(linestack,stylestack,stylestackcount); //Save stylestack
        linestackcount=stylestackcount;
        flags |= FLAGFIRSTLINE; //Set firstline flag for next line
        linebuf=""; //Clear line buffer
        linestyle="";
        linewidth=0;
        prevbottom=curstyle[SVR_BOTTOMMARGIN]; //Hold bottom margin for potential collapsing with next paragraphs top margin
        dY+=lineheight; //Add line height
      } else {
        copystack(stylestack,linestack,linestackcount); //Revert style stack to beginning of line
        stylestackcount=linestackcount;
      }
    }
    if (dohr && withinbounds(dY)) {
      //Do horizontal rule if so commanded and still within limits
      //Always associated with dolinebreak, so at this point dY points to a new line
      //Serial.println("Horizontal rule");
      if (processtype==PT_OUTPUT) display.drawFastHLine(curstyle[SVR_LEFTMARGIN]+curstyle[SVR_INDENT], dY-lineheight/2, 
              dW-curstyle[SVR_LEFTMARGIN]-curstyle[SVR_RIGHTMARGIN]-2*curstyle[SVR_INDENT], EPD_BLACK);
      dY+=curstyle[SVR_BOTTOMMARGIN];
    }
    if (val==0) {
      //If val is clear
      if (readmode<=RMPLAIN && textbuf!="") {
        //Textfile or plain readmode and something in text buffer
        //Serial.printf("Reading from textbuf=%s",textbuf.c_str());
        val=textbuf[0]; //Next working value will be the first character of the text buffer
        //Serial.printf(" val=%c\n",char(val));
        textbuf.remove(0,1); //Slice it off the text buffer.
      } else {
        val=Uto1252(file.read()); //Next working value will be next byte in file
      }
      if (val==-1 && extstylesheet) { //End of external stylesheet
        file.close();
        fileopen(file,fullpath,O_READ); //Reopen document
        file.seek(extpos); //Return to previous position
        val=Uto1252(file.read()); //Read next byte
        extstylesheet=false;
      }
    }
  } // while (val!=-1 && dY<=dH-menuheight-descender-margin && linkcount<=8) {
  //digitalWrite(LED_BUILTIN,HIGH); //Make sure we leave the busy led on after all the blinking, there is still proccessing to be done
  Serial.printf("Processed page in %lu mS\n", (unsigned long)processtime);
  //Serial.printf("After page: Flags=%d, processtype=%d\n",flags,processpage);
  return foundsection;
} // void processpage(bool dooutput, String &section) {

inline int charadvance(uint8_t font, uint8_t val) {
  switch (font) {
    case FONT_NORMAL: return _FreeSans_Glyphs[val-0x20].xAdvance;
    case FONT_ITALIC: return _FreeSansOblique_Glyphs[val-0x20].xAdvance;
    case FONT_BOLD: return _FreeSansBold_Glyphs[val-0x20].xAdvance;
    case FONT_BOLDITALIC: return _FreeSansBoldOblique_Glyphs[val-0x20].xAdvance;
  }
  return 0; //Shouldnt happen
}

void switchlight() {
  //Switches light on or off
  if (lighton) {
    //Light currently on
    digitalWrite(light_pin,LOW);
    lighton=false;
  } else {
    analogWrite(light_pin,lightval);
    lighton=true;
  }
}

void drawstatusbar() {
  //Draws the status bar frame, switches to built in font and displays memory free and battery voltage in it
  display.drawRect(0,0,display.width(),statusheight,EPD_BLACK);
  setfont(FONT_BUILTIN);
  display.setTextSize(0);
  display.setCursor(display.width()-100,4);
  display.printf("%3dkB Laus %4.2fV",rp2040.getFreeHeap()/1024,1.0*battmv()/1000);
}

void drawmenuicon(int index, const unsigned char *icon) {
  //Draw an icon at specified menu index
  display.drawBitmap(menumargin+index*((display.width()-2*menumargin)/8)+menuwidth/2-iconwidth/2,
                      display.height()-menuiconY, icon, iconwidth, iconheight, EPD_BLACK);
}

void drawmenubar(int firstitem, int lastitem, int step, String ending) {
  //Draws menu bar frames and numbers them with labels from firstitem to lastitem
  int16_t  x1, y1; //Used by getTextBounds
  uint16_t ww, wh; //Used by getTextBounds
  String label;
  setfont(FONT_BOLD);
  int itemnum=firstitem;
  for (int i=0;i<8;i++) {
    display.drawRect(menumargin+i*((display.width()-2*menumargin)/8),display.height()-menuheight,menuwidth,menuheight,EPD_BLACK);
    display.drawLine(menumargin+i*((display.width()-2*menumargin)/8),display.height()-menulineY,menumargin+i*((display.width()-2*menumargin)/8)+menuwidth,display.height()-menulineY,EPD_BLACK);
    if (itemnum<=lastitem) {
      //Print label
      label="["+String(itemnum)+ending+"]";
      display.getTextBounds(label.c_str(), 0, 50, &x1, &y1, &ww, &wh); //Get size of word
      display.setCursor(menumargin+i*((display.width()-2*menumargin)/8)+menuwidth/2-ww/2,display.height()-menutextY);
      display.print(label);
      itemnum+=step;
    }
  }
}

//Opens file filen, opening SD card if neccessary.
//Returns true if sucessful, otherwise false.
bool fileopen(FsFile &obj, const String& filen, int mode) {
  digitalWrite(LED_BUILTIN,HIGH);
  if (!obj.open(filen.c_str(),mode)) {
    //Try initializing SD card
    Serial.println("Initializing SD card");
    while (!sd.begin(SD_CONFIG)) {
      //Loop until SD card available
      //Show warning
      digitalWrite(LED_BUILTIN, HIGH); //Turn on busy LED
      setfont(FONT_NORMAL);
      display.setTextSize(2);
      display.clearBuffer();
      display.setCursor(0,100);
      printlncentre(Uto1252string("Ekkert minniskort."));
      drawstatusbar();
      drawmenubar(1,0,1,":");
      drawmenuicon(1, iconcancel);
      drawmenuicon(6, iconlight);
      display.display();
      while (checkbutton(1)!=BUTTON_UP) {
        //Loop until cancel button is pressed
        if (checkbutton(6)==BUTTON_UP) switchlight(); //Light on/off
      }
      Serial.println("Retry...");
    } //while (!sd.begin(SD_CONFIG)) {
    //SD card connected
    if (!obj.open(filen.c_str(),mode)) {
      // Retry opening file
      Serial.printf("Failed to open %s\n",filen.c_str());
      return false;
    }
  } //if (!obj.open(filen.c_str(),mode)) {
  return true;
} //bool fileopen(FsFile &obj, const String& filen, int mode) {

//Reads flags, startpos, style stack and fullpath from /mimisbrunnur.pos
//If pop is true, reads the second to last record amd deletes the last record,
//otherwise reads the last record.
//Returns true if successful and file is not empty afterwards, or false otherwise
//Sets nohistory if only one item in position file
bool readpos(bool pop) {
  Serial.println("Reading position");
  //Open file, starting SD if needed. Return false if file doesnt exist.
  FsFile posfile;
  nohistory=true; //Assume no history
  if (!fileopen(posfile, "/mimisbrunnur.pos", O_RDWR)) {
    Serial.println("Couldnt open position file");
    return false;
  }
  if (posfile.size()==0) {
    //Empty file
    Serial.println("Empty position file");
    posfile.close();
    return false;
  }
  int pospos=findlastrecord(posfile);
  //If bad position file
  if (pospos==-1) {
    Serial.println("Cannot find last record");
    posfile.close();
    return false;
  }
  if (pop) {
    //posfile.seek(pospos);
    posfile.truncate(pospos);
    //Serial.println("Last record truncated");
    pospos=findlastrecord(posfile);
    //Serial.printf("Previous record found at %d\n",pospos);
    //If the last record is the only record
    if (pospos==-1) {
      Serial.println("POPed the last record");
      posfile.close();
      return false;
    }
  }
  if (pospos!=0) nohistory=false; //If we are not reading the only item in file
  posfile.seek(pospos);
  //Read record, setting values, exit if unseccesful
  if (readrecord(posfile,true)==-1){
    posfile.close();
    return false;
  }
  fullpath=""; //Clear fullpath
  int c=posfile.read(); //Read first character in fullpath
  //Repeat until null
  while (c!=0 && c!=-1) {
    //Serial.printf("%c %d ",char(c),c);
    fullpath+=char(c); //Add to fullpath
    c=posfile.read(); //Read next character
  }
  //Serial.println();
  posfile.close();
  return true;
} //bool readpos(bool pop) {

//Finds the last record in the position file and returns the position of that record
//Does rudimentary checking whether file is valid and returns -1 if it isnt
int findlastrecord(FsFile &posfile) {
  int pospos=-1;
  posfile.seek(0);
  //Repeat reading record until nothing left
  while (posfile.available()>0) {
    pospos=posfile.position(); //Remember position at start of record
    if (readrecord(posfile, false)==-1) return-1; //Attempt to read record, exit if unsuccessful
    int c=posfile.read(); //Read first character in fullpath
    //Repeat until null
    while (c!=0 && c!=-1) c=posfile.read(); //Read next character
  }
  return pospos;
}

//Reads one record from position or bookmark file
//Does not read the fullpath from a position file record
//Returns the document file position contained in the record if successfull
//Returns -1 if unseccessfull. Sets startpos, flags and style stack if setdata is true
//Sets recfile location to end of record+1
inline int32_t readrecord(FsFile &recfile, bool setdata) {
  int c=recfile.read(); //Read first byte of record
  //If not file v2, exit with error
  if (! (c & FLAGV2)) {
    Serial.printf("Not v2 file, flags=%d\n",c);
    return -1;
  }
  int32_t pos;
  recfile.read(&pos,4);
  uint8_t ssc=recfile.read(); //Read size of style stack, 1 byte
  //If stack size < 1 or > max, exit with error
  if (ssc<1 || ssc>stylestackmaxcount) {
    Serial.printf("Readrecord: Stack size out of bounds, %d\n",ssc);
    return -1;
  } 
  //Serial.printf("Readrecord: Style stack items=%d\n",ssc);
  if (setdata) {
    flags=c;
    startpos=pos;
    //Read stylestack, one member at a time, each member containing char[tagsize] tag buffer and uint16_t[stecount] value array
    for (int i=0;i<ssc;i++) {
      for (int ii=0;ii<tagsize;ii++) {
        c=recfile.read();
        if (c==-1) {
          Serial.println("Readrecord: Incomplete tag in stylestack");
          return -1;
        }
        stylestack[i].tag[ii]=c;
      }
      recfile.read(&stylestack[i].value,stecount*2);
      // Serial.printf(" item=%d",i+1);
      // Serial.printf(", tag=%s",stylestack[i].tag);
      // Serial.printf(", style=%x\n",stylestack[i].style);
    }
    stylestackcount=ssc;
  } else {
    recfile.seek(recfile.position()+ssc*(stecount*2+tagsize)); //Jump over stack
  }
  return pos;
} //int32_t readrecord(&FsFile recfile, bool setdata) {

//Writes flags, startpos, style stack and fullpath to /mimisbrunnur.pos
//If push is true, adds record to the end of file,
//otherwise updates the last record.
//Returns true if successful, or false otherwise
//Sets nohistory if only one item in file
bool writepos(bool push) {
  //Open file, starting SD if needed. Return false if unsuccessful.
  nohistory=true; //Assume no history
  FsFile posfile;
  if (!fileopen(posfile, "/mimisbrunnur.pos", O_RDWR | O_CREAT)) return false; 
  int pospos=findlastrecord(posfile);
  //If posfile is bad, write record at top
  if (pospos==-1) {
    Serial.println("Position file bad, writing to top");
    posfile.seek(0);
  } else {
    //If push, add to end of file
    if (push) {
      posfile.seek(posfile.size());
      //Serial.printf("Pushing at %d\n",posfile.curPosition());
    } else {
      //No push and file ok, update last record
      posfile.seek(pospos);
    }
  }
  if (posfile.curPosition()!=0) nohistory=false; //If we are not writing at top of file, there is history
  //Write record
  writerecord(posfile);
  posfile.write(fullpath.c_str(),fullpath.length()+1); //Write fullpath, null terminated
  posfile.truncate(); //Truncate any possible trash data
  //Serial.printf("Position within position file after writing: %d\n",posfile.curPosition());
  posfile.close();
  return true;
} //bool writepos(bool push) {

//Writes one record to position or bookmark file, using current data
//Does not write the fullpath to a position file record
//Sets recfile location to end of record+1
void writerecord(FsFile &recfile) {
  //Serial.printf("Writerecord: flags=%d startpos=%d stylestackcount=%d\n",flags,startpos,stylestackcount);
  recfile.write(flags);
  recfile.write(&startpos,4); //Write start position, uint32_t=4 bytes
  recfile.write(stylestackcount); //Write size of style stack, 1 byte
  //Write stylestack, one member at a time, each member containing char[tagsize] tag buffer and uint16_t[stecount] value array
  for (int i=0;i<stylestackcount;i++) {
    recfile.write(&stylestack[i].tag,tagsize);
    recfile.write(&stylestack[i].value,stecount*2);
  }
}

void displayfiles(bool allowcancel) {
  //Displays a file choice dialog
  //If allowcancel is true, the dialog can be cancelled and nothing will be changed
  //If allowcancel is false, the cancel option is not shown and you must select a file, usually because there is no valid file to default to
  Serial.println("Start of displayfiles function");
  int curpage=0; //Number of the page currently being displayed
  String temppath=fullpath.substring(0,fullpath.lastIndexOf('/')+1); //Working path while in dialog
  const int maxpages=64;
  String lastitem[maxpages]; //Holds the last items on each page for browsing purposes
  for (int i=0;i<maxpages;i++) {
    lastitem[i].reserve(32);
    lastitem[i]="";
  }
  while (true) {
    //Main loop of the function, repeated each time a new page is displayed
    //Exit by return statement(s)
    digitalWrite(LED_BUILTIN, HIGH); //Turn on busy LED
    setfont(FONT_NORMAL);
    display.setTextSize(0);
    display.clearBuffer();
    display.setCursor(0,statusheight+lineheight-descender+margin); //Start of first line
    display.println(temppath); // Show the working path at the start of the page
    display.println();
    FsFile dir, item;
    int pagecount=0; //Number of pages in the directory
    String itemarr[8]; //Holds the items on the current page
    for (int i=0;i<8;i++) {
      itemarr[i].reserve(32);
      itemarr[i]="";
    }
    int itemcount=0; //Number of items on the page
    int totalcount=0; //Total number of items
    if (curpage==0 && temppath!="/") {
      //First item in all directories except root is parent directory
      itemarr[0]="..";
//      display.println("1: ..");
      itemcount=1;
      totalcount=1;
    }
    if (!fileopen(dir, temppath, O_READ)) {
      //Try to open directory, starting SD if needed.
      temppath="/"; //Try root if not successful
      if (!fileopen(dir,temppath, O_READ)) return; //Shouldnt happen. 
                                                   //fileopen() will start SD card if needed, and then we should always have root.
    }
    dir.rewind();
    while (item.openNext(&dir, O_READ)) {
      //Scan throught the current directory
      if (!item.isHidden()) {
        //Don't include hidden items
        totalcount++;
        char tempname[256];
        item.getName(tempname,sizeof(tempname)-1); //Retrieve item name
        String tempstring=String(tempname);
        if (item.isDir()) tempstring+="/";
        Serial.printf("Found %s\n",tempstring.c_str());
        bool itemfound=false;
        if (curpage==0 || filesort(lastitem[curpage-1],tempstring)<0) { //First page or item comes after last item of previous page
          for (int i=itemcount-1;i>=0;i--) { //Go backwards through items
            if (filesort(itemarr[i],tempstring)>0) { //New item is earlier than this one
              Serial.printf("Inserting %s",tempstring.c_str());
              Serial.printf(" before %s\n",itemarr[i].c_str());
              if (i<7) itemarr[i+1]=itemarr[i]; //Shuffle old item down the list if there is space for it
              itemarr[i]=tempstring; //Store new item here
              itemfound=true;
            }
          }
          if (itemcount<8) { //If page is not full, item will always be added to it
            itemcount++;
            if (!itemfound) {
              itemarr[itemcount-1]=tempstring; //At the end if not inserted already
              Serial.printf("Adding %s\n",tempstring.c_str());
            }
          }
          if (itemcount>0) lastitem[curpage]=itemarr[itemcount-1]; //Remember last item of page
        }
      }
      item.close();
    }
    dir.close();
    for (int i=0;i<itemcount;i++) display.printf("%d: %s\n",i+1,itemarr[i].c_str());
    pagecount=(totalcount+7)/8;
    //Draw status bar, setting font to built in
    drawstatusbar();
    display.setCursor(4,4);
    display.printf("%d/%d",curpage+1,pagecount);
    //Draw menu bar
    // int visibleitems=8;
    // if (curpage==pagecount-1 && itemcount!=0) visibleitems=itemcount; //Last page and not full
    drawmenubar(1,itemcount,1,":");
    if (curpage>0) drawmenuicon(0, iconprev); //Page back
    if (allowcancel) drawmenuicon(1, iconcancel);
    drawmenuicon(6, iconlight);
    if (curpage<pagecount-1) drawmenuicon(7, iconnext);
    //Display output
    display.display();
    //Check buttons
    bool nextcycle=false; //False while scanning buttons, set true to display another page
    while (!nextcycle) {
      for (int button=0;button<8;button++) {
        //Scan through buttons
        int buttonval=checkbutton(button);
        if (buttonval==BUTTON_UP) {
          //The button has been released after short press
          if (button==0 && curpage>0) {
            //Previous
            curpage--;
            nextcycle=true;
          }
          if (button==1 && allowcancel) {
            //Cancel
            return; //Exit function
          }
          if (button==6) switchlight(); //Light
          if (button==7 && curpage<pagecount-1) {
            //Next
            curpage++;
            nextcycle=true;
          }
        }
        if (buttonval==BUTTON_HELD && button<itemcount) {
          //Button has been held for a long press and a valid item has been selected
          if (itemarr[button]=="..") {
            //Parent directory
            temppath.remove(temppath.length()-1); //Remove trailing slash from working path
            temppath.remove(temppath.lastIndexOf("/")+1); //Truncate to last slash before that
            curpage=0; //Start with first page of parent directory
            nextcycle=true;
          } else if (itemarr[button].endsWith("/")) {
            //Subdirectory
            temppath+=itemarr[button];
            curpage=0; //Start with first page of subdirectory
            nextcycle=true;
          } else {
            //File
            fullpath=temppath+itemarr[button];
            startpos=0; //Start at beginning of file
            flags|=FLAGFIRSTLINE; //First line at beginning of file
            clearstylestack();
            sd.remove("/mimisbrunnur.pos"); //Delete link history when opening new file
            nohistory=true;;
            styleprocessed=false;
            return; //Exit function
          }
        } //if (buttonval==BUTTON_HELD && button<visibleitems) {
      } //for (int button=0;button<8;button++) {
    } //while (!nextcycle) {
  } //while (true) {
} //void displayfiles(bool allowcancel) {

void printlncentre (String line){
  //Prints line centered to display
  int16_t  x1, y1; //Used by getTextBounds
  uint16_t lw, lh; //Used by getTextBounds
  display.getTextBounds(line.c_str(),0,display.getCursorY(),&x1,&y1,&lw,&lh);
  display.setCursor((display.width()-lw)/2,display.getCursorY());
  display.println(line);
}

int battmv() {
  //Returns battery voltage in milliVolts
  int accumulator=0;
  for (int i=0;i<8;i++) accumulator+=analogRead(volt_pin); //Accumulate 8 readings for averaging
  return accumulator*6600/8192; //3300 mV *2 (because of 1:2 voltage divider) / 1024 full scale resolution * 8 readings
}

void battest() {
  //Checks if battery is below limit
  //If so, turns off light and rechecks
  //If still below limit, show warning and loop until voltage is above limit + hysterisis
  if (battmv()<battlow) {
    //Battery low
    lighton=false;
    if (battmv()<battlow) { //If battery still low
      digitalWrite(LED_BUILTIN, HIGH); //Turn on busy LED
      //Show warning
      display.clearBuffer();
      drawstatusbar();
      setfont(FONT_NORMAL);
      display.setTextSize(2);
      display.setCursor(0,100);
      printlncentre(Uto1252string("Rafhlaðan er tóm."));
      printlncentre(Uto1252string("Hlaðið við fyrsta tækifæri"));
      printlncentre(Uto1252string("og hafið bókina lokaða þangað til."));
      display.display();
      digitalWrite(LED_BUILTIN, LOW); //Turn off busy LED
      sd.end(); //Stop SD activity
      while (battmv()<=battlow+batthyst) ; //Idle until battery voltage rises above hysterisis
    }
  }
}

//Debounces and checks button. Returns 0 if nothing is happening, BUTTON_DOWN if button has been pressed,
//BUTTON_UP if button has been released, or BUTTON_HELD if button has been held for hold_time.
//Does not return BUTTON_UP after BUTTON_HELD.
int checkbutton(int button) {
  battest(); //Always do battery test when checking buttons, ensures it is frequent.
  digitalWrite(LED_BUILTIN, LOW); //Turn off busy LED to indicate that the user can now press buttons
  if (debounce>debounce_time && digitalRead(button_pins[button])!=button_state[button]) { 
    //Debounce time is over and button state has changed
    debounce=0; //Reset timer
    button_state[button]=digitalRead(button_pins[button]);
    if (button_state[button]==LOW) {
      //Serial.printf("Button %d down\n",button);
      return BUTTON_DOWN;
    } else {
      if (button_held[button]) {
        //Button has previously been detected as held
        button_held[button]=false;
      } else {
        //Serial.printf("Button %d up\n",button);
        return BUTTON_UP;
      }
    }
  } else if (button_state[button]==LOW && debounce>hold_time && !button_held[button]) {
    //Button hasn't changed, is down, hold_time exceeded and button hasn't yet been detected as held
    button_held[button]=true;
    //Serial.printf("Button %d held\n",button);
    return BUTTON_HELD;
  }
  return 0;
}


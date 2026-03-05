//This file is for offloading of style related functions and declarations from the main file

const int lineheight=16; //In pixels, total including line spacing
const int descender=3; //Depth of descenders from baseline

//Definition of a style flag reference, i.e. bitmask and position of flags
//Can refer to parts of flags value, in which case pos is bit address
//or to entire values, in which case pos is element address
struct styleflagref {
  uint16_t mask;
  uint8_t position;
};

//Flag references
const styleflagref SFR_UNDERLINE={0x00000001,0};
const styleflagref SFR_STRIKE={0x00000002,1}; //Strikethrough
const styleflagref SFR_FONT={0x0000000C,2}; //0 regular, 1 italic, 2 bold, 3 bold italic
const styleflagref SFR_ITALIC={0x00000004,2};
const styleflagref SFR_BOLD={0x00000008,3};
//Bits 4 to 7 reserved
const styleflagref SFR_ALIGN={0x00000300,8}; //0 left, 1 center, 2 right, 3 undefined
const styleflagref SFR_CENTER={0x00000100,8};
const styleflagref SFR_RIGHT={0x00000200,9};
//Bits 10 to 14 reserved
//Bit 15 unavailable due to int16_t (signed) used to hold flags

//Value references, indices to style element
uint8_t SVR_FLAGS=0;
uint8_t SVR_INDENT=1;
uint8_t SVR_TOPMARGIN=2;
uint8_t SVR_RIGHTMARGIN=3;
uint8_t SVR_BOTTOMMARGIN=4;
uint8_t SVR_LEFTMARGIN=5;

//Sets flag according to element and value
inline void setflag(styleflagref reference, int16_t &flagvar, uint8_t value) {
  flagvar&=~reference.mask; //Clear bits
  flagvar|=value << reference.position; //Set bits according to value
}

//Gets flag element and returns value
inline uint8_t getflag (styleflagref reference, int16_t flagvar) {
  return (flagvar & reference.mask) >> reference.position;
}

const uint8_t stecount=12; //Number of values in a style element, each value is 16 bits so size in bytes is stecount*2
//Style elements are int16_t[stecount] arrays, indexed by SR constants for values
//Flags (member[0] of the array), are further indexed by SR constants for flags

const uint8_t tagsize=16; //Size of tag buffer in stylemember

//Member of style stack
struct stylemember {
  char tag[tagsize]; //Tag that initiated change
  int16_t value[stecount]; //Style that got replaced by tag
};
//const uint8_t smsize=stecount*2+tagsize;

const uint8_t stylestackmaxcount=16;
stylemember stylestack[stylestackmaxcount];
uint8_t stylestackcount=1; //Current size of the style stack, in members

//Clears values of a provided style and sets to default
void clearstyle(int16_t value[]) {
  value[SVR_FLAGS]=0; //No flags set
  value[SVR_INDENT]=16; //16 pixel indent
  for (int i=SVR_TOPMARGIN;i<=SVR_LEFTMARGIN;i++) value[i]=0; //No margins
  value[SVR_BOTTOMMARGIN]=8; //8 pixel bottom margin
}

//Clears the style stack, setting base style
void clearstylestack() {
  stylestackcount=1;
  stylestack[0].tag[0]='\0'; //No tag for the base style
  clearstyle(stylestack[0].value);//Set base style
}

//Copies stylestack of size count, from source to dest.
inline void copystack(stylemember dest[], stylemember source[], int count) {
  for (int i=0;i<count;i++) {
    strncpy(dest[i].tag, source[i].tag, tagsize);
    for (int ii=0;ii<stecount;ii++) dest[i].value[ii]=source[i].value[ii];
  }
}

//Member of style list, defining name of style (by pointer) stylemember, flag bitmask and value bitmask
struct stylelistmember {
  //char *nameptr; //Pointer to char array holding the name of the style reference
  String name; //Name of style reference
  int16_t value[stecount]; //Style value array associated with name
  int16_t flagmask; //Bitmask for flags affected by style, see SR constants
  uint32_t valuemask; //Bitmask for values affected by style, see SR constants
}; //stylelist[stylelistmaxcount];

const int stylelistmaxcount=256;
stylelistmember stylelist[stylelistmaxcount];
int stylelistcount=0; //Current size of the style list, in members

//Converts style values into float number of pixels. Returns NaN (not a number) if value is not recognized. 
float getpixels(String &value) {
  float retval=0;
  if (value[0]<'0' or value[0]>'9') return std::nan(""); //Value must start with a digit
  retval=value.toFloat();
  if (value.endsWith("em")) return retval*=lineheight; //All other numerical values are considered pixel values
  return retval;
}

//Processes a style property, value, and value# and applies to new stylelistitem index
void  processstyleproperty(int index, String &property, String &value,int valnum) {
  property.toLowerCase();
  value.toLowerCase();
  // Serial.printf("Setting style # %d",index);
  // Serial.printf(", %s", property.c_str());
  // Serial.printf(": %s",value.c_str());
  // Serial.printf("; value #%d\n", valnum);
  float pixelval=0;
  if (value!="inherit") {
    //For all properties, if the value is inherit it is not adjusted
    if (property=="text-decoration" || property=="text-decoration-line") {
      //Underline or strikethrough
      setflag(SFR_UNDERLINE,stylelist[index].flagmask,1); //Mark as modified
      setflag(SFR_STRIKE,stylelist[index].flagmask,1);
      setflag(SFR_UNDERLINE,stylelist[index].value[SVR_FLAGS],0); //Default to off
      setflag(SFR_STRIKE,stylelist[index].value[SVR_FLAGS],0);
      if (value.indexOf("underline")!=-1) setflag(SFR_UNDERLINE,stylelist[index].value[SVR_FLAGS],1);
      if (value.indexOf("line-through")!=-1) setflag(SFR_STRIKE,stylelist[index].value[SVR_FLAGS],1);
    } else if (property=="font-style") {
      //Italics
      setflag(SFR_ITALIC,stylelist[index].flagmask,1); //Mark as modified
      setflag(SFR_ITALIC,stylelist[index].value[SVR_FLAGS],0); //Default to off
      if (value.indexOf("italic")!=-1 || value.indexOf("oblique")!=-1) setflag(SFR_ITALIC,stylelist[index].value[SVR_FLAGS],1);
    } else if (property=="font-weight") {
      //Bold
      setflag(SFR_BOLD,stylelist[index].flagmask,1); //Mark as modified
      setflag(SFR_BOLD,stylelist[index].value[SVR_FLAGS],0); //Default to off
      if (value.indexOf("bold")!=-1) setflag(SFR_BOLD,stylelist[index].value[SVR_FLAGS],1);
    } else if (property=="text-align") {
      //Alignment
      setflag(SFR_ALIGN,stylelist[index].flagmask,3); //Mark as modified
      setflag(SFR_ALIGN,stylelist[index].value[SVR_FLAGS],0); //Default to off
      if (value.indexOf("center")!=-1) setflag(SFR_CENTER,stylelist[index].value[SVR_FLAGS],1);
      if (value.indexOf("right")!=-1) setflag(SFR_RIGHT,stylelist[index].value[SVR_FLAGS],1);
    } else if (property=="text-indent") {
      //Indent
      stylelist[index].valuemask|=1<<SVR_INDENT; //Mark as modified
      pixelval=getpixels(value); //Get pixel value
      if (!std::isnan(pixelval)) stylelist[index].value[SVR_INDENT]=pixelval;
      else stylelist[index].value[SVR_INDENT]=0;
    } else if (property=="margin") {
      //Margin, shorthand
      stylelist[index].valuemask|=1<<SVR_TOPMARGIN; //Mark as modified
      stylelist[index].valuemask|=1<<SVR_RIGHTMARGIN; //Mark as modified
      stylelist[index].valuemask|=1<<SVR_BOTTOMMARGIN; //Mark as modified
      stylelist[index].valuemask|=1<<SVR_LEFTMARGIN; //Mark as modified
      pixelval=getpixels(value); //Get pixel value
      if (!std::isnan(pixelval)) {
        switch(valnum) {
          case 0: //First value sets all four sides
            stylelist[index].value[SVR_TOPMARGIN]=pixelval;
            stylelist[index].value[SVR_RIGHTMARGIN]=pixelval;
            stylelist[index].value[SVR_BOTTOMMARGIN]=pixelval;
            stylelist[index].value[SVR_LEFTMARGIN]=pixelval;
            break;
          case 1: //Second value overrides right and left sides
            stylelist[index].value[SVR_RIGHTMARGIN]=pixelval;
            stylelist[index].value[SVR_LEFTMARGIN]=pixelval;
            break;
          case 2: //Third value overrides bottom
            stylelist[index].value[SVR_BOTTOMMARGIN]=pixelval;
            break;
          case 3: //Fourth value overrides left side
            stylelist[index].value[SVR_LEFTMARGIN]=pixelval;
        } // switch(valnum) {
      } // if (!std::isnan(pixelval)) {
    } else if (property=="margin-top") {
      //Top margin
      stylelist[index].valuemask|=1<<SVR_TOPMARGIN; //Mark as modified
      pixelval=getpixels(value); //Get pixel value
      if (!std::isnan(pixelval)) stylelist[index].value[SVR_TOPMARGIN]=pixelval;
    } else if (property=="margin-right") {
      //Right margin
      stylelist[index].valuemask|=1<<SVR_RIGHTMARGIN; //Mark as modified
      pixelval=getpixels(value); //Get pixel value
      if (!std::isnan(pixelval)) stylelist[index].value[SVR_RIGHTMARGIN]=pixelval;
    } else if (property=="margin-bottom") {
      //Bottom margin
      stylelist[index].valuemask|=1<<SVR_BOTTOMMARGIN; //Mark as modified
      pixelval=getpixels(value); //Get pixel value
      if (!std::isnan(pixelval)) stylelist[index].value[SVR_BOTTOMMARGIN]=pixelval;
    } else if (property=="margin-left") {
      //Left margin
      stylelist[index].valuemask|=1<<SVR_LEFTMARGIN; //Mark as modified
      pixelval=getpixels(value); //Get pixel value
      if (!std::isnan(pixelval)) stylelist[index].value[SVR_LEFTMARGIN]=pixelval;
    }
  } // if (value!="inherit") {
} // void  processstyleproperty(int index, String &property, String &value,int valnum) {

//Applies style from stylelist[stylelistindex] to latest styleset in stylestack
void applystyle(int stylelistindex) {
  int mask=0x0001;
  for (int i=0;i<15;i++) {
    //Loop throgh flags
    if (stylelist[stylelistindex].flagmask & mask) {
      //All flags are exclusive; if modified the new value is used directly
      if (stylelist[stylelistindex].value[SVR_FLAGS] & mask) stylestack[stylestackcount-1].value[SVR_FLAGS]|=mask;
      else stylestack[stylestackcount-1].value[SVR_FLAGS]&=~mask;
    }
    mask<<=1; //Shift mask 1 bit to left
  }
  mask=1<<SVR_INDENT;
  if (stylelist[stylelistindex].valuemask & mask) stylestack[stylestackcount-1].value[SVR_INDENT]=stylelist[stylelistindex].value[SVR_INDENT];
  //Indent is excluse; if modified the new value is used directly
  mask<<=1; //Shift mask 1 bit to left
  if (stylelist[stylelistindex].valuemask & mask) stylestack[stylestackcount-1].value[SVR_TOPMARGIN]=
                                                  stylelist[stylelistindex].value[SVR_TOPMARGIN];
  //Top margin is excluse; if modified the new value is used directly
  mask<<=1; //Shift mask 1 bit to left
  if (stylelist[stylelistindex].valuemask & mask) stylestack[stylestackcount-1].value[SVR_RIGHTMARGIN]+=
                                                  stylelist[stylelistindex].value[SVR_RIGHTMARGIN];
  //Right margin is cumulative; if modified the new value is added to the previous value
  mask<<=1; //Shift mask 1 bit to left
  if (stylelist[stylelistindex].valuemask & mask) stylestack[stylestackcount-1].value[SVR_BOTTOMMARGIN]=
                                                  stylelist[stylelistindex].value[SVR_BOTTOMMARGIN];
  //Bottom margin is excluse; if modified the new value is used directly
  mask<<=1; //Shift mask 1 bit to left
  if (stylelist[stylelistindex].valuemask & mask) stylestack[stylestackcount-1].value[SVR_LEFTMARGIN]+=
                                                  stylelist[stylelistindex].value[SVR_LEFTMARGIN];
  //Left margin is cumulative; if modified the new value is added to the previous value
}

//Clears the next member of stylelist for clean assembly
void clearnewstylelistmember() {
  stylelist[stylelistcount].name="";
  clearstyle(stylelist[stylelistcount].value);
  stylelist[stylelistcount].valuemask=0;
  stylelist[stylelistcount].flagmask=0;
}

//Searches for style name in style list and returns index if found, otherwise -1
int stylelistfind(String &stylename) {
  for (int i=0;i<stylelistcount;i++) {
    if (stylelist[i].name==stylename) return i;
  }
  return -1;
}

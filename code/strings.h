//This file is for offloading of string functions from the main file

//Array for weighing codepoints for alphabetic sorting.
//ICELANDIC ALPHABET! Will work for english alphabet / ASCII, no promises for other alphabets.
//Use codepoint values when weighing values are equal.
//Sorting order is first control codes, second symbols, third numbers, fourth letters.
//For letters, capital letters precede lower case only for same letter, e.g. B comes before b, but b comes before C.
//Accented letters come after unaccented letters, e.g. AaÁá.
//In icelandic the acute accent is the only one used, but here we allow all accents the same weight.
//Ð comes after D.
//ÞÆÖ come at the end of the alphabet (in icelandic, Ö is regarded as a separate letter, not as accented O).
//Icelandic alphabet: AÁBCDÐEÉFGHIÍJKLMNOÓPQRSTUÚVWXYÝZÞÆÖ
//Assumes a CP1252 codepoint as index.
const uint8_t SORTWEIGH_IS[]={
    0, //0x00  ;Null      
    0, //0x01  ;Start Of Heading    
    0, //0x02  ;Start Of Text    
    0, //0x03  ;End Of Text    
    0, //0x04  ;End Of Transmission    
    0, //0x05  ;Enquiry      
    0, //0x06  ;Acknowledge      
    0, //0x07  ;Bell      
    0, //0x08  ;Backspace      
    0, //0x09  ;Horizontal Tabulation     
    0, //0x0a  ;Line Feed     
    0, //0x0b  ;Vertical Tabulation     
    0, //0x0c  ;Form Feed     
    0, //0x0d  ;Carriage Return     
    0, //0x0e  ;Shift Out     
    0, //0x0f  ;Shift In     
    0, //0x10  ;Data Link Escape    
    0, //0x11  ;Device Control One    
    0, //0x12  ;Device Control Two    
    0, //0x13  ;Device Control Three    
    0, //0x14  ;Device Control Four    
    0, //0x15  ;Negative Acknowledge     
    0, //0x16  ;Synchronous Idle     
    0, //0x17  ;End Of Transmission Block   
    0, //0x18  ;Cancel      
    0, //0x19  ;End Of Medium    
    0, //0x1a  ;Substitute      
    0, //0x1b  ;Escape      
    0, //0x1c  ;File Separator     
    0, //0x1d  ;Group Separator     
    0, //0x1e  ;Record Separator     
    0, //0x1f  ;Unit Separator     
    1, //0x20   ;Space      
    1, //0x21 ! ;Exclamation Mark     
    1, //0x22 " ;Quotation Mark     
    1, //0x23 # ;Number Sign     
    1, //0x24 $ ;Dollar Sign     
    1, //0x25 % ;Percent Sign     
    1, //0x26 & ;Ampersand      
    1, //0x27 ' ;Apostrophe      
    1, //0x28 ( ;Left Parenthesis     
    1, //0x29 ) ;Right Parenthesis     
    1, //0x2a * ;Asterisk      
    1, //0x2b + ;Plus Sign     
    1, //0x2c , ;Comma      
    1, //0x2d - ;Hyphen-Minus      
    1, //0x2e . ;Full Stop     
    1, //0x2f / ;Solidus      
    2, //0x30 0 ;Digit Zero     
    2, //0x31 1 ;Digit One     
    2, //0x32 2 ;Digit Two     
    2, //0x33 3 ;Digit Three     
    2, //0x34 4 ;Digit Four     
    2, //0x35 5 ;Digit Five     
    2, //0x36 6 ;Digit Six     
    2, //0x37 7 ;Digit Seven     
    2, //0x38 8 ;Digit Eight     
    2, //0x39 9 ;Digit Nine     
    1, //0x3a : ;Colon      
    1, //0x3b ; ;Semicolon      
    1, //0x3c < ;Less-Than Sign     
    1, //0x3d = ;Equals Sign     
    1, //0x3e > ;Greater-Than Sign     
    1, //0x3f ? ;Question Mark     
    1, //0x40 @ ;Commercial At     
    3, //0x41 A ;Latin Capital Letter A   
    5, //0x42 B ;Latin Capital Letter B   
    7, //0x43 C ;Latin Capital Letter C   
    9, //0x44 D ;Latin Capital Letter D   
    11, //0x45 E ;Latin Capital Letter E   
    13, //0x46 F ;Latin Capital Letter F   
    15, //0x47 G ;Latin Capital Letter G   
    17, //0x48 H ;Latin Capital Letter H   
    19, //0x49 I ;Latin Capital Letter I   
    21, //0x4a J ;Latin Capital Letter J   
    23, //0x4b K ;Latin Capital Letter K   
    25, //0x4c L ;Latin Capital Letter L   
    27, //0x4d M ;Latin Capital Letter M   
    29, //0x4e N ;Latin Capital Letter N   
    31, //0x4f O ;Latin Capital Letter O   
    33, //0x50 P ;Latin Capital Letter P   
    35, //0x51 Q ;Latin Capital Letter Q   
    37, //0x52 R ;Latin Capital Letter R   
    39, //0x53 S ;Latin Capital Letter S   
    41, //0x54 T ;Latin Capital Letter T   
    43, //0x55 U ;Latin Capital Letter U   
    45, //0x56 V ;Latin Capital Letter V   
    47, //0x57 W ;Latin Capital Letter W   
    49, //0x58 X ;Latin Capital Letter X   
    51, //0x59 Y ;Latin Capital Letter Y   
    53, //0x5a Z ;Latin Capital Letter Z   
    1, //0x5b [ ;Left Square Bracket    
    1, //0x5c \ ;Reverse Solidus     
    1, //0x5d ] ;Right Square Bracket    
    1, //0x5e ^ ;Circumflex Accent     
    1, //0x5f _ ;Low Line     
    1, //0x60 ` ;Grave Accent     
    3, //0x61 a ;Latin Small Letter A   
    5, //0x62 b ;Latin Small Letter B   
    7, //0x63 c ;Latin Small Letter C   
    9, //0x64 d ;Latin Small Letter D   
    11, //0x65 e ;Latin Small Letter E   
    13, //0x66 f ;Latin Small Letter F   
    15, //0x67 g ;Latin Small Letter G   
    17, //0x68 h ;Latin Small Letter H   
    19, //0x69 i ;Latin Small Letter I   
    21, //0x6a j ;Latin Small Letter J   
    23, //0x6b k ;Latin Small Letter K   
    25, //0x6c l ;Latin Small Letter L   
    27, //0x6d m ;Latin Small Letter M   
    29, //0x6e n ;Latin Small Letter N   
    31, //0x6f o ;Latin Small Letter O   
    33, //0x70 p ;Latin Small Letter P   
    35, //0x71 q ;Latin Small Letter Q   
    37, //0x72 r ;Latin Small Letter R   
    39, //0x73 s ;Latin Small Letter S   
    41, //0x74 t ;Latin Small Letter T   
    43, //0x75 u ;Latin Small Letter U   
    45, //0x76 v ;Latin Small Letter V   
    47, //0x77 w ;Latin Small Letter W   
    49, //0x78 x ;Latin Small Letter X   
    51, //0x79 y ;Latin Small Letter Y   
    53, //0x7a z ;Latin Small Letter Z   
    1, //0x7b { ;Left Curly Bracket    
    1, //0x7c | ;Vertical Line     
    1, //0x7d } ;Right Curly Bracket    
    1, //0x7e ~ ;Tilde      
    0, //0x7f  ;Delete      
    1, //0x80 € ;Euro Sign     
    0, //0x81        
    1, //0x82 ‚ ;Single Low-9 Quotation Mark   
    14, //0x83 ƒ ;Latin Small Letter F With Hook 
    1, //0x84 „ ;Double Low-9 Quotation Mark   
    1, //0x85 … ;Horizontal Ellipsis     
    1, //0x86 † ;Dagger      
    1, //0x87 ‡ ;Double Dagger     
    1, //0x88 ˆ ;Modifier Letter Circumflex Accent   
    1, //0x89 ‰ ;Per Mille Sign    
    40, //0x8a Š ;Latin Capital Letter S With Caron 
    1, //0x8b ‹ ;Single Left-Pointing Angle Quotation Mark  
    56, //0x8c Œ ;Latin Capital Ligature Oe   
    0, //0x8d        
    54, //0x8e Ž ;Latin Capital Letter Z With Caron 
    0, //0x8f        
    0, //0x90        
    1, //0x91 ‘ ;Left Single Quotation Mark   
    1, //0x92 ’ ;Right Single Quotation Mark   
    1, //0x93 “ ;Left Double Quotation Mark   
    1, //0x94 ” ;Right Double Quotation Mark   
    1, //0x95 • ;Bullet      
    1, //0x96 – ;En Dash     
    1, //0x97 — ;Em Dash     
    1, //0x98 ˜ ;Small Tilde     
    1, //0x99 ™ ;Trade Mark Sign    
    40, //0x9a š ;Latin Small Letter S With Caron 
    1, //0x9b › ;Single Right-Pointing Angle Quotation Mark  
    56, //0x9c œ ;Latin Small Ligature Oe   
    0, //0x9d        
    54, //0x9e ž ;Latin Small Letter Z With Caron 
    52, //0x9f Ÿ ;Latin Capital Letter Y With Diaeresis 
    1, //0xa0   ;No-Break Space     
    1, //0xa1 ¡ ;Inverted Exclamation Mark    
    1, //0xa2 ¢ ;Cent Sign     
    1, //0xa3 £ ;Pound Sign     
    1, //0xa4 ¤ ;Currency Sign     
    1, //0xa5 ¥ ;Yen Sign     
    1, //0xa6 ¦ ;Broken Bar     
    1, //0xa7 § ;Section Sign     
    1, //0xa8 ¨ ;Diaeresis      
    1, //0xa9 © ;Copyright Sign     
    1, //0xaa ª ;Feminine Ordinal Indicator    
    1, //0xab « ;Left-Pointing Double Angle Quotation Mark  
    1, //0xac ¬ ;Not Sign     
    1, //0xad ­ ;Soft Hyphen     
    1, //0xae ® ;Registered Sign     
    1, //0xaf ¯ ;Macron      
    1, //0xb0 ° ;Degree Sign     
    1, //0xb1 ± ;Plus-Minus Sign     
    1, //0xb2 ² ;Superscript Two     
    1, //0xb3 ³ ;Superscript Three     
    1, //0xb4 ´ ;Acute Accent     
    1, //0xb5 µ ;Micro Sign     
    1, //0xb6 ¶ ;Pilcrow Sign     
    1, //0xb7 · ;Middle Dot     
    1, //0xb8 ¸ ;Cedilla      
    1, //0xb9 ¹ ;Superscript One     
    1, //0xba º ;Masculine Ordinal Indicator    
    1, //0xbb » ;Right-Pointing Double Angle Quotation Mark  
    1, //0xbc ¼ ;Vulgar Fraction One Quarter   
    1, //0xbd ½ ;Vulgar Fraction One Half   
    1, //0xbe ¾ ;Vulgar Fraction Three Quarters   
    1, //0xbf ¿ ;Inverted Question Mark    
    4, //0xc0 À ;Latin Capital Letter A With Grave 
    4, //0xc1 Á ;Latin Capital Letter A With Acute 
    4, //0xc2 Â ;Latin Capital Letter A With Circumflex 
    4, //0xc3 Ã ;Latin Capital Letter A With Tilde 
    4, //0xc4 Ä ;Latin Capital Letter A With Diaeresis 
    4, //0xc5 Å ;Latin Capital Letter A With Ring Above
    56, //0xc6 Æ ;Latin Capital Ligature Ae   
    8, //0xc7 Ç ;Latin Capital Letter C With Cedilla 
    12, //0xc8 È ;Latin Capital Letter E With Grave 
    12, //0xc9 É ;Latin Capital Letter E With Acute 
    12, //0xca Ê ;Latin Capital Letter E With Circumflex 
    12, //0xcb Ë ;Latin Capital Letter E With Diaeresis 
    20, //0xcc Ì ;Latin Capital Letter I With Grave 
    20, //0xcd Í ;Latin Capital Letter I With Acute 
    20, //0xce Î ;Latin Capital Letter I With Circumflex 
    20, //0xcf Ï ;Latin Capital Letter I With Diaeresis 
    10, //0xd0 Ð ;Latin Capital Letter Eth   
    30, //0xd1 Ñ ;Latin Capital Letter N With Tilde 
    32, //0xd2 Ò ;Latin Capital Letter O With Grave 
    32, //0xd3 Ó ;Latin Capital Letter O With Acute 
    32, //0xd4 Ô ;Latin Capital Letter O With Circumflex 
    32, //0xd5 Õ ;Latin Capital Letter O With Tilde 
    57, //0xd6 Ö ;Latin Capital Letter O With Diaeresis 
    1, //0xd7 × ;Multiplication Sign     
    32, //0xd8 Ø ;Latin Capital Letter O With Stroke 
    44, //0xd9 Ù ;Latin Capital Letter U With Grave 
    44, //0xda Ú ;Latin Capital Letter U With Acute 
    44, //0xdb Û ;Latin Capital Letter U With Circumflex 
    44, //0xdc Ü ;Latin Capital Letter U With Diaeresis 
    52, //0xdd Ý ;Latin Capital Letter Y With Acute 
    55, //0xde Þ ;Latin Capital Letter Thorn   
    40, //0xdf ß ;Latin Small Letter Sharp S  
    4, //0xe0 à ;Latin Small Letter A With Grave 
    4, //0xe1 á ;Latin Small Letter A With Acute 
    4, //0xe2 â ;Latin Small Letter A With Circumflex 
    4, //0xe3 ã ;Latin Small Letter A With Tilde 
    4, //0xe4 ä ;Latin Small Letter A With Diaeresis 
    4, //0xe5 å ;Latin Small Letter A With Ring Above
    56, //0xe6 æ ;Latin Small Ligature Ae   
    8, //0xe7 ç ;Latin Small Letter C With Cedilla 
    12, //0xe8 è ;Latin Small Letter E With Grave 
    12, //0xe9 é ;Latin Small Letter E With Acute 
    12, //0xea ê ;Latin Small Letter E With Circumflex 
    12, //0xeb ë ;Latin Small Letter E With Diaeresis 
    20, //0xec ì ;Latin Small Letter I With Grave 
    20, //0xed í ;Latin Small Letter I With Acute 
    20, //0xee î ;Latin Small Letter I With Circumflex 
    20, //0xef ï ;Latin Small Letter I With Diaeresis 
    10, //0xf0 ð ;Latin Small Letter Eth   
    30, //0xf1 ñ ;Latin Small Letter N With Tilde 
    32, //0xf2 ò ;Latin Small Letter O With Grave 
    32, //0xf3 ó ;Latin Small Letter O With Acute 
    32, //0xf4 ô ;Latin Small Letter O With Circumflex 
    32, //0xf5 õ ;Latin Small Letter O With Tilde 
    57, //0xf6 ö ;Latin Small Letter O With Diaeresis 
    1, //0xf7 ÷ ;Division Sign     
    32, //0xf8 ø ;Latin Small Letter O With Stroke 
    44, //0xf9 ù ;Latin Small Letter U With Grave 
    44, //0xfa ú ;Latin Small Letter U With Acute 
    44, //0xfb û ;Latin Small Letter U With Circumflex 
    44, //0xfc ü ;Latin Small Letter U With Diaeresis 
    52, //0xfd ý ;Latin Small Letter Y With Acute 
    55, //0xfe þ ;Latin Small Letter Thorn   
    52  //0xff ÿ ;Latin Small Letter Y With Diaeresis
};

//Similar to strcmp, but operates on arduino Strings and uses alphabetic sorting.
//ICELANDIC ALPHABET!
//Returns <0 if string1 comes before string2, >0 if string1 comes after string2, and 0 if strings are equal.
inline int sortcmp(String &string1, String &string2) {
  int l1=string1.length();
  int l2=string2.length();
  int l;
  if (l1<l2) l=l1; //Find length of shorter string
  else l=l2;
  int c;
  int w;
  for (int i=0;i<l;i++) { //Loop through strings until end of shorter string
    c=string1[i]-string2[i]; //Get difference of codepoint values
    if (c!=0) { //If not equal
      w=SORTWEIGH_IS[(unsigned char) string1[i]] - SORTWEIGH_IS[(unsigned char) string2[i]]; //Get difference of sorting weght
      if (w!=0) return w; //Return difference if not equal
      return c; //Otherwise return codepoint difference
    }
  }
  //Equal so far
  if (l1<l2) return -1; //String1 is shorter than string2
  if (l1>l2) return 1; //String1 is longer than string2
  return 0; //Both strings are equal
}

//Extends sortcmp by including file priority.
//First priority is parent directory (..), second is subdirectories (end with /),
//third priority is .ncx files, and then everything else.
inline int filesort(String &string1, String &string2) {
  //if (string1==string2) return 0; //This should never happen with files in the same directory
  if (string1=="..") return -1;
  if (string2=="..") return 1;
  if (string1.endsWith("/") && string2.endsWith("/")) return sortcmp(string1,string2); //Both subdirectories, return alphabetic sort
  if (string1.endsWith("/")) return -1; //String1 is subdirectory and string2 not, string1 comes first
  if (string2.endsWith("/")) return 1; //String2 is subdirectory and string1 not, string2 comes first
  if (string1.endsWith(".ncx") && string2.endsWith(".ncx")) return sortcmp(string1,string2); //Both .ncx files, return alphabetic sort
  if (string1.endsWith(".ncx")) return -1; //String1 is .ncx file and string2 not, string1 comes first
  if (string2.endsWith(".ncx")) return 1; //String2 is .ncx file and string1 not, string2 comes first
  return sortcmp(string1, string2); //Otherwise, return alphabetic sort
}

//Attempts to convert a decoded unicode codepoint to CP1252
int16_t Unicodeto1252(int32_t unicode) {
  if (unicode<0x100) return unicode; //0xFF and lower map directly to CP1252
  //Extendeded codes, possible match with CP1252
  //List courtesy of https://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt
  switch (unicode) {
    case 0x0100: return 0x41; //;Latin Capital Letter A With Macron
    case 0x0101: return 0x61; //;Latin Small Letter A With Macron
    case 0x0102: return 0x41; //;Latin Capital Letter A With Breve
    case 0x0103: return 0x61; //;Latin Small Letter A With Breve
    case 0x0104: return 0x41; //;Latin Capital Letter A With Ogonek
    case 0x0105: return 0x61; //;Latin Small Letter A With Ogonek
    case 0x0106: return 0x43; //;Latin Capital Letter C With Acute
    case 0x0107: return 0x63; //;Latin Small Letter C With Acute
    case 0x0108: return 0x43; //;Latin Capital Letter C With Circumflex
    case 0x0109: return 0x63; //;Latin Small Letter C With Circumflex
    case 0x010a: return 0x43; //;Latin Capital Letter C With Dot Above
    case 0x010b: return 0x63; //;Latin Small Letter C With Dot Above
    case 0x010c: return 0x43; //;Latin Capital Letter C With Caron
    case 0x010d: return 0x63; //;Latin Small Letter C With Caron
    case 0x010e: return 0x44; //;Latin Capital Letter D With Caron
    case 0x010f: return 0x64; //;Latin Small Letter D With Caron
    case 0x0110: return 0xd0; //;Latin Capital Letter D With Stroke
    case 0x0111: return 0x64; //;Latin Small Letter D With Stroke
    case 0x0112: return 0x45; //;Latin Capital Letter E With Macron
    case 0x0113: return 0x65; //;Latin Small Letter E With Macron
    case 0x0114: return 0x45; //;Latin Capital Letter E With Breve
    case 0x0115: return 0x65; //;Latin Small Letter E With Breve
    case 0x0116: return 0x45; //;Latin Capital Letter E With Dot Above
    case 0x0117: return 0x65; //;Latin Small Letter E With Dot Above
    case 0x0118: return 0x45; //;Latin Capital Letter E With Ogonek
    case 0x0119: return 0x65; //;Latin Small Letter E With Ogonek
    case 0x011a: return 0x45; //;Latin Capital Letter E With Caron
    case 0x011b: return 0x65; //;Latin Small Letter E With Caron
    case 0x011c: return 0x47; //;Latin Capital Letter G With Circumflex
    case 0x011d: return 0x67; //;Latin Small Letter G With Circumflex
    case 0x011e: return 0x47; //;Latin Capital Letter G With Breve
    case 0x011f: return 0x67; //;Latin Small Letter G With Breve
    case 0x0120: return 0x47; //;Latin Capital Letter G With Dot Above
    case 0x0121: return 0x67; //;Latin Small Letter G With Dot Above
    case 0x0122: return 0x47; //;Latin Capital Letter G With Cedilla
    case 0x0123: return 0x67; //;Latin Small Letter G With Cedilla
    case 0x0124: return 0x48; //;Latin Capital Letter H With Circumflex
    case 0x0125: return 0x68; //;Latin Small Letter H With Circumflex
    case 0x0126: return 0x48; //;Latin Capital Letter H With Stroke
    case 0x0127: return 0x68; //;Latin Small Letter H With Stroke
    case 0x0128: return 0x49; //;Latin Capital Letter I With Tilde
    case 0x0129: return 0x69; //;Latin Small Letter I With Tilde
    case 0x012a: return 0x49; //;Latin Capital Letter I With Macron
    case 0x012b: return 0x69; //;Latin Small Letter I With Macron
    case 0x012c: return 0x49; //;Latin Capital Letter I With Breve
    case 0x012d: return 0x69; //;Latin Small Letter I With Breve
    case 0x012e: return 0x49; //;Latin Capital Letter I With Ogonek
    case 0x012f: return 0x69; //;Latin Small Letter I With Ogonek
    case 0x0130: return 0x49; //;Latin Capital Letter I With Dot Above
    case 0x0131: return 0x69; //;Latin Small Letter Dotless I
    case 0x0134: return 0x4a; //;Latin Capital Letter J With Circumflex
    case 0x0135: return 0x6a; //;Latin Small Letter J With Circumflex
    case 0x0136: return 0x4b; //;Latin Capital Letter K With Cedilla
    case 0x0137: return 0x6b; //;Latin Small Letter K With Cedilla
    case 0x0139: return 0x4c; //;Latin Capital Letter L With Acute
    case 0x013a: return 0x6c; //;Latin Small Letter L With Acute
    case 0x013b: return 0x4c; //;Latin Capital Letter L With Cedilla
    case 0x013c: return 0x6c; //;Latin Small Letter L With Cedilla
    case 0x013d: return 0x4c; //;Latin Capital Letter L With Caron
    case 0x013e: return 0x6c; //;Latin Small Letter L With Caron
    case 0x0141: return 0x4c; //;Latin Capital Letter L With Stroke
    case 0x0142: return 0x6c; //;Latin Small Letter L With Stroke
    case 0x0143: return 0x4e; //;Latin Capital Letter N With Acute
    case 0x0144: return 0x6e; //;Latin Small Letter N With Acute
    case 0x0145: return 0x4e; //;Latin Capital Letter N With Cedilla
    case 0x0146: return 0x6e; //;Latin Small Letter N With Cedilla
    case 0x0147: return 0x4e; //;Latin Capital Letter N With Caron
    case 0x0148: return 0x6e; //;Latin Small Letter N With Caron
    case 0x014c: return 0x4f; //;Latin Capital Letter O With Macron
    case 0x014d: return 0x6f; //;Latin Small Letter O With Macron
    case 0x014e: return 0x4f; //;Latin Capital Letter O With Breve
    case 0x014f: return 0x6f; //;Latin Small Letter O With Breve
    case 0x0150: return 0x4f; //;Latin Capital Letter O With Double Acute
    case 0x0151: return 0x6f; //;Latin Small Letter O With Double Acute
    case 0x0152: return 0x8c; //;Latin Capital Ligature Oe
    case 0x0153: return 0x9c; //;Latin Small Ligature Oe
    case 0x0154: return 0x52; //;Latin Capital Letter R With Acute
    case 0x0155: return 0x72; //;Latin Small Letter R With Acute
    case 0x0156: return 0x52; //;Latin Capital Letter R With Cedilla
    case 0x0157: return 0x72; //;Latin Small Letter R With Cedilla
    case 0x0158: return 0x52; //;Latin Capital Letter R With Caron
    case 0x0159: return 0x72; //;Latin Small Letter R With Caron
    case 0x015a: return 0x53; //;Latin Capital Letter S With Acute
    case 0x015b: return 0x73; //;Latin Small Letter S With Acute
    case 0x015c: return 0x53; //;Latin Capital Letter S With Circumflex
    case 0x015d: return 0x73; //;Latin Small Letter S With Circumflex
    case 0x015e: return 0x53; //;Latin Capital Letter S With Cedilla
    case 0x015f: return 0x73; //;Latin Small Letter S With Cedilla
    case 0x0160: return 0x8a; //;Latin Capital Letter S With Caron
    case 0x0161: return 0x9a; //;Latin Small Letter S With Caron
    case 0x0162: return 0x54; //;Latin Capital Letter T With Cedilla
    case 0x0163: return 0x74; //;Latin Small Letter T With Cedilla
    case 0x0164: return 0x54; //;Latin Capital Letter T With Caron
    case 0x0165: return 0x74; //;Latin Small Letter T With Caron
    case 0x0166: return 0x54; //;Latin Capital Letter T With Stroke
    case 0x0167: return 0x74; //;Latin Small Letter T With Stroke
    case 0x0168: return 0x55; //;Latin Capital Letter U With Tilde
    case 0x0169: return 0x75; //;Latin Small Letter U With Tilde
    case 0x016a: return 0x55; //;Latin Capital Letter U With Macron
    case 0x016b: return 0x75; //;Latin Small Letter U With Macron
    case 0x016c: return 0x55; //;Latin Capital Letter U With Breve
    case 0x016d: return 0x75; //;Latin Small Letter U With Breve
    case 0x016e: return 0x55; //;Latin Capital Letter U With Ring Above
    case 0x016f: return 0x75; //;Latin Small Letter U With Ring Above
    case 0x0170: return 0x55; //;Latin Capital Letter U With Double Acute
    case 0x0171: return 0x75; //;Latin Small Letter U With Double Acute
    case 0x0172: return 0x55; //;Latin Capital Letter U With Ogonek
    case 0x0173: return 0x75; //;Latin Small Letter U With Ogonek
    case 0x0174: return 0x57; //;Latin Capital Letter W With Circumflex
    case 0x0175: return 0x77; //;Latin Small Letter W With Circumflex
    case 0x0176: return 0x59; //;Latin Capital Letter Y With Circumflex
    case 0x0177: return 0x79; //;Latin Small Letter Y With Circumflex
    case 0x0178: return 0x9f; //;Latin Capital Letter Y With Diaeresis
    case 0x0179: return 0x5a; //;Latin Capital Letter Z With Acute
    case 0x017a: return 0x7a; //;Latin Small Letter Z With Acute
    case 0x017b: return 0x5a; //;Latin Capital Letter Z With Dot Above
    case 0x017c: return 0x7a; //;Latin Small Letter Z With Dot Above
    case 0x017d: return 0x8e; //;Latin Capital Letter Z With Caron
    case 0x017e: return 0x9e; //;Latin Small Letter Z With Caron
    case 0x0180: return 0x62; //;Latin Small Letter B With Stroke
    case 0x0189: return 0xd0; //;Latin Capital Letter African D
    case 0x0191: return 0x83; //;Latin Capital Letter F With Hook
    case 0x0192: return 0x83; //;Latin Small Letter F With Hook
    case 0x0197: return 0x49; //;Latin Capital Letter I With Stroke
    case 0x019a: return 0x6c; //;Latin Small Letter L With Bar
    case 0x019f: return 0x4f; //;Latin Capital Letter O With Middle Tilde
    case 0x01a0: return 0x4f; //;Latin Capital Letter O With Horn
    case 0x01a1: return 0x6f; //;Latin Small Letter O With Horn
    case 0x01ab: return 0x74; //;Latin Small Letter T With Palatal Hook
    case 0x01ae: return 0x54; //;Latin Capital Letter T With Retroflex Hook
    case 0x01af: return 0x55; //;Latin Capital Letter U With Horn
    case 0x01b0: return 0x75; //;Latin Small Letter U With Horn
    case 0x01b6: return 0x7a; //;Latin Small Letter Z With Stroke
    case 0x01c0: return 0x7c; //;Latin Letter Dental Click
    case 0x01c3: return 0x21; //;Latin Letter Retroflex Click
    case 0x01cd: return 0x41; //;Latin Capital Letter A With Caron
    case 0x01ce: return 0x61; //;Latin Small Letter A With Caron
    case 0x01cf: return 0x49; //;Latin Capital Letter I With Caron
    case 0x01d0: return 0x69; //;Latin Small Letter I With Caron
    case 0x01d1: return 0x4f; //;Latin Capital Letter O With Caron
    case 0x01d2: return 0x6f; //;Latin Small Letter O With Caron
    case 0x01d3: return 0x55; //;Latin Capital Letter U With Caron
    case 0x01d4: return 0x75; //;Latin Small Letter U With Caron
    case 0x01d5: return 0x55; //;Latin Capital Letter U With Diaeresis And Macron
    case 0x01d6: return 0x75; //;Latin Small Letter U With Diaeresis And Macron
    case 0x01d7: return 0x55; //;Latin Capital Letter U With Diaeresis And Acute
    case 0x01d8: return 0x75; //;Latin Small Letter U With Diaeresis And Acute
    case 0x01d9: return 0x55; //;Latin Capital Letter U With Diaeresis And Caron
    case 0x01da: return 0x75; //;Latin Small Letter U With Diaeresis And Caron
    case 0x01db: return 0x55; //;Latin Capital Letter U With Diaeresis And Grave
    case 0x01dc: return 0x75; //;Latin Small Letter U With Diaeresis And Grave
    case 0x01de: return 0x41; //;Latin Capital Letter A With Diaeresis And Macron
    case 0x01df: return 0x61; //;Latin Small Letter A With Diaeresis And Macron
    case 0x01e4: return 0x47; //;Latin Capital Letter G With Stroke
    case 0x01e5: return 0x67; //;Latin Small Letter G With Stroke
    case 0x01e6: return 0x47; //;Latin Capital Letter G With Caron
    case 0x01e7: return 0x67; //;Latin Small Letter G With Caron
    case 0x01e8: return 0x4b; //;Latin Capital Letter K With Caron
    case 0x01e9: return 0x6b; //;Latin Small Letter K With Caron
    case 0x01ea: return 0x4f; //;Latin Capital Letter O With Ogonek
    case 0x01eb: return 0x6f; //;Latin Small Letter O With Ogonek
    case 0x01ec: return 0x4f; //;Latin Capital Letter O With Ogonek And Macron
    case 0x01ed: return 0x6f; //;Latin Small Letter O With Ogonek And Macron
    case 0x01f0: return 0x6a; //;Latin Small Letter J With Caron
    case 0x0261: return 0x67; //;Latin Small Letter Script G
    case 0x02b9: return 0x27; //;Modifier Letter Prime
    case 0x02ba: return 0x22; //;Modifier Letter Double Prime
    case 0x02bc: return 0x27; //;Modifier Letter Apostrophe
    case 0x02c4: return 0x5e; //;Modifier Letter Up Arrowhead
    case 0x02c6: return 0x88; //;Modifier Letter Circumflex Accent
    case 0x02c8: return 0x27; //;Modifier Letter Vertical Line
    case 0x02c9: return 0xaf; //;Modifier Letter Macron
    case 0x02ca: return 0xb4; //;Modifier Letter Acute Accent
    case 0x02cb: return 0x60; //;Modifier Letter Grave Accent
    case 0x02cd: return 0x5f; //;Modifier Letter Low Macron
    case 0x02da: return 0xb0; //;Ring Above
    case 0x02dc: return 0x98; //;Small Tilde
    case 0x0300: return 0x60; //;Combining Grave Accent
    case 0x0301: return 0xb4; //;Combining Acute Accent
    case 0x0302: return 0x5e; //;Combining Circumflex Accent
    case 0x0303: return 0x7e; //;Combining Tilde
    case 0x0304: return 0xaf; //;Combining Macron
    case 0x0305: return 0xaf; //;Combining Overline
    case 0x0308: return 0xa8; //;Combining Diaeresis
    case 0x030a: return 0xb0; //;Combining Ring Above
    case 0x030e: return 0x22; //;Combining Double Vertical Line Above
    case 0x0327: return 0xb8; //;Combining Cedilla
    case 0x0331: return 0x5f; //;Combining Macron Below
    case 0x0332: return 0x5f; //;Combining Low Line
    case 0x037e: return 0x3b; //;Greek Question Mark
    case 0x0393: return 0x47; //;Greek Capital Letter Gamma
    case 0x0398: return 0x54; //;Greek Capital Letter Theta
    case 0x03a3: return 0x53; //;Greek Capital Letter Sigma
    case 0x03a6: return 0x46; //;Greek Capital Letter Phi
    case 0x03a9: return 0x4f; //;Greek Capital Letter Omega
    case 0x03b1: return 0x61; //;Greek Small Letter Alpha
    case 0x03b2: return 0xdf; //;Greek Small Letter Beta
    case 0x03b4: return 0x64; //;Greek Small Letter Delta
    case 0x03b5: return 0x65; //;Greek Small Letter Epsilon
    case 0x03bc: return 0xb5; //;Greek Small Letter Mu
    case 0x03c0: return 0x70; //;Greek Small Letter Pi
    case 0x03c3: return 0x73; //;Greek Small Letter Sigma
    case 0x03c4: return 0x74; //;Greek Small Letter Tau
    case 0x03c6: return 0x66; //;Greek Small Letter Phi
    case 0x04bb: return 0x68; //;Cyrillic Small Letter Shha
    case 0x0589: return 0x3a; //;Armenian Full Stop
    case 0x066a: return 0x25; //;Arabic Percent Sign
    case 0x2000: return 0x20; //;En Quad
    case 0x2001: return 0x20; //;Em Quad
    case 0x2002: return 0x20; //;En Space
    case 0x2003: return 0x20; //;Em Space
    case 0x2004: return 0x20; //;Three-Per-Em Space
    case 0x2005: return 0x20; //;Four-Per-Em Space
    case 0x2006: return 0x20; //;Six-Per-Em Space
    case 0x2010: return 0x2d; //;Hyphen
    case 0x2011: return 0x2d; //;Non-Breaking Hyphen
    case 0x2013: return 0x96; //;En Dash
    case 0x2014: return 0x97; //;Em Dash
    case 0x2017: return 0x3d; //;Double Low Line
    case 0x2018: return 0x91; //;Left Single Quotation Mark
    case 0x2019: return 0x92; //;Right Single Quotation Mark
    case 0x201a: return 0x82; //;Single Low-9 Quotation Mark
    case 0x201c: return 0x93; //;Left Double Quotation Mark
    case 0x201d: return 0x94; //;Right Double Quotation Mark
    case 0x201e: return 0x84; //;Double Low-9 Quotation Mark
    case 0x2020: return 0x86; //;Dagger
    case 0x2021: return 0x87; //;Double Dagger
    case 0x2022: return 0x95; //;Bullet
    case 0x2024: return 0xb7; //;One Dot Leader
    case 0x2026: return 0x85; //;Horizontal Ellipsis
    case 0x2030: return 0x89; //;Per Mille Sign
    case 0x2032: return 0x27; //;Prime
    case 0x2035: return 0x60; //;Reversed Prime
    case 0x2039: return 0x8b; //;Single Left-Pointing Angle Quotation Mark
    case 0x203a: return 0x9b; //;Single Right-Pointing Angle Quotation Mark
    case 0x2044: return 0x2f; //;Fraction Slash
    case 0x2070: return 0xb0; //;Superscript Zero
    case 0x2074: return 0x34; //;Superscript Four
    case 0x2075: return 0x35; //;Superscript Five
    case 0x2076: return 0x36; //;Superscript Six
    case 0x2077: return 0x37; //;Superscript Seven
    case 0x2078: return 0x38; //;Superscript Eight
    case 0x207f: return 0x6e; //;Superscript Latin Small Letter N
    case 0x2080: return 0x30; //;Subscript Zero
    case 0x2081: return 0x31; //;Subscript One
    case 0x2082: return 0x32; //;Subscript Two
    case 0x2083: return 0x33; //;Subscript Three
    case 0x2084: return 0x34; //;Subscript Four
    case 0x2085: return 0x35; //;Subscript Five
    case 0x2086: return 0x36; //;Subscript Six
    case 0x2087: return 0x37; //;Subscript Seven
    case 0x2088: return 0x38; //;Subscript Eight
    case 0x2089: return 0x39; //;Subscript Nine
    case 0x20ac: return 0x80; //;Euro Sign
    case 0x20a1: return 0xa2; //;Colon Sign
    case 0x20a4: return 0xa3; //;Lira Sign
    case 0x20a7: return 0x50; //;Peseta Sign
    case 0x2102: return 0x43; //;Double-Struck Capital C
    case 0x2107: return 0x45; //;Euler Constant
    case 0x210a: return 0x67; //;Script Small G
    case 0x210b: return 0x48; //;Script Capital H
    case 0x210c: return 0x48; //;Black-Letter Capital H
    case 0x210d: return 0x48; //;Double-Struck Capital H
    case 0x210e: return 0x68; //;Planck Constant
    case 0x2110: return 0x49; //;Script Capital I
    case 0x2111: return 0x49; //;Black-Letter Capital I
    case 0x2112: return 0x4c; //;Script Capital L
    case 0x2113: return 0x6c; //;Script Small L
    case 0x2115: return 0x4e; //;Double-Struck Capital N
    case 0x2118: return 0x50; //;Script Capital P
    case 0x2119: return 0x50; //;Double-Struck Capital P
    case 0x211a: return 0x51; //;Double-Struck Capital Q
    case 0x211b: return 0x52; //;Script Capital R
    case 0x211c: return 0x52; //;Black-Letter Capital R
    case 0x211d: return 0x52; //;Double-Struck Capital R
    case 0x2122: return 0x99; //;Trade Mark Sign
    case 0x2124: return 0x5a; //;Double-Struck Capital Z
    case 0x2128: return 0x5a; //;Black-Letter Capital Z
    case 0x212a: return 0x4b; //;Kelvin Sign
    case 0x212b: return 0xc5; //;Angstrom Sign
    case 0x212c: return 0x42; //;Script Capital B
    case 0x212d: return 0x43; //;Black-Letter Capital C
    case 0x212e: return 0x65; //;Estimated Symbol
    case 0x212f: return 0x65; //;Script Small E
    case 0x2130: return 0x45; //;Script Capital E
    case 0x2131: return 0x46; //;Script Capital F
    case 0x2133: return 0x4d; //;Script Capital M
    case 0x2134: return 0x6f; //;Script Small O
    case 0x2205: return 0xd8; //;Empty Set
    case 0x2212: return 0x2d; //;Minus Sign
    case 0x2213: return 0xb1; //;Minus-Or-Plus Sign
    case 0x2215: return 0x2f; //;Division Slash
    case 0x2216: return 0x5c; //;Set Minus
    case 0x2217: return 0x2a; //;Asterisk Operator
    case 0x2218: return 0xb0; //;Ring Operator
    case 0x2219: return 0xb7; //;Bullet Operator
    case 0x221a: return 0x76; //;Square Root
    case 0x221e: return 0x38; //;Infinity
    case 0x2223: return 0x7c; //;Divides
    case 0x2229: return 0x6e; //;Intersection
    case 0x2236: return 0x3a; //;Ratio
    case 0x223c: return 0x7e; //;Tilde Operator
    case 0x2248: return 0x98; //;Almost Equal To
    case 0x2261: return 0x3d; //;Identical To
    case 0x2264: return 0x3d; //;Less-Than Or Equal To
    case 0x2265: return 0x3d; //;Greater-Than Or Equal To
    case 0x226a: return 0xab; //;Much Less-Than
    case 0x226b: return 0xbb; //;Much Greater-Than
    case 0x22c5: return 0xb7; //;Dot Operator
    case 0x2302: return 0xa6; //;House
    case 0x2303: return 0x5e; //;Up Arrowhead
    case 0x2310: return 0xac; //;Reversed Not Sign
    case 0x2320: return 0x28; //;Top Half Integral
    case 0x2321: return 0x29; //;Bottom Half Integral
    case 0x2329: return 0x3c; //;Left-Pointing Angle Bracket
    case 0x232a: return 0x3e; //;Right-Pointing Angle Bracket
    case 0x2500: return 0x2d; //;Box Drawings Light Horizontal
    case 0x2502: return 0xa6; //;Box Drawings Light Vertical
    case 0x250c: return 0x2b; //;Box Drawings Light Down And Right
    case 0x2510: return 0x2b; //;Box Drawings Light Down And Left
    case 0x2514: return 0x2b; //;Box Drawings Light Up And Right
    case 0x2518: return 0x2b; //;Box Drawings Light Up And Left
    case 0x251c: return 0x2b; //;Box Drawings Light Vertical And Right
    case 0x2524: return 0xa6; //;Box Drawings Light Vertical And Left
    case 0x252c: return 0x2d; //;Box Drawings Light Down And Horizontal
    case 0x2534: return 0x2d; //;Box Drawings Light Up And Horizontal
    case 0x253c: return 0x2b; //;Box Drawings Light Vertical And Horizontal
    case 0x2550: return 0x2d; //;Box Drawings Double Horizontal
    case 0x2551: return 0xa6; //;Box Drawings Double Vertical
    case 0x2552: return 0x2b; //;Box Drawings Down Single And Right Double
    case 0x2553: return 0x2b; //;Box Drawings Down Double And Right Single
    case 0x2554: return 0x2b; //;Box Drawings Double Down And Right
    case 0x2555: return 0x2b; //;Box Drawings Down Single And Left Double
    case 0x2556: return 0x2b; //;Box Drawings Down Double And Left Single
    case 0x2557: return 0x2b; //;Box Drawings Double Down And Left
    case 0x2558: return 0x2b; //;Box Drawings Up Single And Right Double
    case 0x2559: return 0x2b; //;Box Drawings Up Double And Right Single
    case 0x255a: return 0x2b; //;Box Drawings Double Up And Right
    case 0x255b: return 0x2b; //;Box Drawings Up Single And Left Double
    case 0x255c: return 0x2b; //;Box Drawings Up Double And Left Single
    case 0x255d: return 0x2b; //;Box Drawings Double Up And Left
    case 0x255e: return 0xa6; //;Box Drawings Vertical Single And Right Double
    case 0x255f: return 0xa6; //;Box Drawings Vertical Double And Right Single
    case 0x2560: return 0xa6; //;Box Drawings Double Vertical And Right
    case 0x2561: return 0xa6; //;Box Drawings Vertical Single And Left Double
    case 0x2562: return 0xa6; //;Box Drawings Vertical Double And Left Single
    case 0x2563: return 0xa6; //;Box Drawings Double Vertical And Left
    case 0x2564: return 0x2d; //;Box Drawings Down Single And Horizontal Double
    case 0x2565: return 0x2d; //;Box Drawings Down Double And Horizontal Single
    case 0x2566: return 0x2d; //;Box Drawings Double Down And Horizontal
    case 0x2567: return 0x2d; //;Box Drawings Up Single And Horizontal Double
    case 0x2568: return 0x2d; //;Box Drawings Up Double And Horizontal Single
    case 0x2569: return 0x2d; //;Box Drawings Double Up And Horizontal
    case 0x256a: return 0x2b; //;Box Drawings Vertical Single And Horizontal Double
    case 0x256b: return 0x2b; //;Box Drawings Vertical Double And Horizontal Single
    case 0x256c: return 0x2b; //;Box Drawings Double Vertical And Horizontal
    case 0x2580: return 0xaf; //;Upper Half Block
    case 0x2584: return 0x5f; //;Lower Half Block
    case 0x2588: return 0xa6; //;Full Block
    case 0x258c: return 0xa6; //;Left Half Block
    case 0x2590: return 0xa6; //;Right Half Block
    case 0x2591: return 0xa6; //;Light Shade
    case 0x2592: return 0xa6; //;Medium Shade
    case 0x2593: return 0xa6; //;Dark Shade
    case 0x25a0: return 0xa6; //;Black Square
    case 0x263c: return 0xa4; //;White Sun With Rays
    case 0x2758: return 0x7c; //;Light Vertical Bar
    case 0x3000: return 0x20; //;Ideographic Space
    case 0x3008: return 0x3c; //;Left Angle Bracket
    case 0x3009: return 0x3e; //;Right Angle Bracket
    case 0x300a: return 0xab; //;Left Double Angle Bracket
    case 0x300b: return 0xbb; //;Right Double Angle Bracket
    case 0x301a: return 0x5b; //;Left White Square Bracket
    case 0x301b: return 0x5d; //;Right White Square Bracket
    case 0x30fb: return 0xb7; //;Katakana Middle Dot
    case 0xff01: return 0x21; //;Fullwidth Exclamation Mark
    case 0xff02: return 0x22; //;Fullwidth Quotation Mark
    case 0xff03: return 0x23; //;Fullwidth Number Sign
    case 0xff04: return 0x24; //;Fullwidth Dollar Sign
    case 0xff05: return 0x25; //;Fullwidth Percent Sign
    case 0xff06: return 0x26; //;Fullwidth Ampersand
    case 0xff07: return 0x27; //;Fullwidth Apostrophe
    case 0xff08: return 0x28; //;Fullwidth Left Parenthesis
    case 0xff09: return 0x29; //;Fullwidth Right Parenthesis
    case 0xff0a: return 0x2a; //;Fullwidth Asterisk
    case 0xff0b: return 0x2b; //;Fullwidth Plus Sign
    case 0xff0c: return 0x2c; //;Fullwidth Comma
    case 0xff0d: return 0x2d; //;Fullwidth Hyphen-Minus
    case 0xff0e: return 0x2e; //;Fullwidth Full Stop
    case 0xff0f: return 0x2f; //;Fullwidth Solidus
    case 0xff10: return 0x30; //;Fullwidth Digit Zero
    case 0xff11: return 0x31; //;Fullwidth Digit One
    case 0xff12: return 0x32; //;Fullwidth Digit Two
    case 0xff13: return 0x33; //;Fullwidth Digit Three
    case 0xff14: return 0x34; //;Fullwidth Digit Four
    case 0xff15: return 0x35; //;Fullwidth Digit Five
    case 0xff16: return 0x36; //;Fullwidth Digit Six
    case 0xff17: return 0x37; //;Fullwidth Digit Seven
    case 0xff18: return 0x38; //;Fullwidth Digit Eight
    case 0xff19: return 0x39; //;Fullwidth Digit Nine
    case 0xff1a: return 0x3a; //;Fullwidth Colon
    case 0xff1b: return 0x3b; //;Fullwidth Semicolon
    case 0xff1c: return 0x3c; //;Fullwidth Less-Than Sign
    case 0xff1d: return 0x3d; //;Fullwidth Equals Sign
    case 0xff1e: return 0x3e; //;Fullwidth Greater-Than Sign
    case 0xff1f: return 0x3f; //;Fullwidth Question Mark
    case 0xff20: return 0x40; //;Fullwidth Commercial At
    case 0xff21: return 0x41; //;Fullwidth Latin Capital Letter A
    case 0xff22: return 0x42; //;Fullwidth Latin Capital Letter B
    case 0xff23: return 0x43; //;Fullwidth Latin Capital Letter C
    case 0xff24: return 0x44; //;Fullwidth Latin Capital Letter D
    case 0xff25: return 0x45; //;Fullwidth Latin Capital Letter E
    case 0xff26: return 0x46; //;Fullwidth Latin Capital Letter F
    case 0xff27: return 0x47; //;Fullwidth Latin Capital Letter G
    case 0xff28: return 0x48; //;Fullwidth Latin Capital Letter H
    case 0xff29: return 0x49; //;Fullwidth Latin Capital Letter I
    case 0xff2a: return 0x4a; //;Fullwidth Latin Capital Letter J
    case 0xff2b: return 0x4b; //;Fullwidth Latin Capital Letter K
    case 0xff2c: return 0x4c; //;Fullwidth Latin Capital Letter L
    case 0xff2d: return 0x4d; //;Fullwidth Latin Capital Letter M
    case 0xff2e: return 0x4e; //;Fullwidth Latin Capital Letter N
    case 0xff2f: return 0x4f; //;Fullwidth Latin Capital Letter O
    case 0xff30: return 0x50; //;Fullwidth Latin Capital Letter P
    case 0xff31: return 0x51; //;Fullwidth Latin Capital Letter Q
    case 0xff32: return 0x52; //;Fullwidth Latin Capital Letter R
    case 0xff33: return 0x53; //;Fullwidth Latin Capital Letter S
    case 0xff34: return 0x54; //;Fullwidth Latin Capital Letter T
    case 0xff35: return 0x55; //;Fullwidth Latin Capital Letter U
    case 0xff36: return 0x56; //;Fullwidth Latin Capital Letter V
    case 0xff37: return 0x57; //;Fullwidth Latin Capital Letter W
    case 0xff38: return 0x58; //;Fullwidth Latin Capital Letter X
    case 0xff39: return 0x59; //;Fullwidth Latin Capital Letter Y
    case 0xff3a: return 0x5a; //;Fullwidth Latin Capital Letter Z
    case 0xff3b: return 0x5b; //;Fullwidth Left Square Bracket
    case 0xff3c: return 0x5c; //;Fullwidth Reverse Solidus
    case 0xff3d: return 0x5d; //;Fullwidth Right Square Bracket
    case 0xff3e: return 0x5e; //;Fullwidth Circumflex Accent
    case 0xff3f: return 0x5f; //;Fullwidth Low Line
    case 0xff40: return 0x60; //;Fullwidth Grave Accent
    case 0xff41: return 0x61; //;Fullwidth Latin Small Letter A
    case 0xff42: return 0x62; //;Fullwidth Latin Small Letter B
    case 0xff43: return 0x63; //;Fullwidth Latin Small Letter C
    case 0xff44: return 0x64; //;Fullwidth Latin Small Letter D
    case 0xff45: return 0x65; //;Fullwidth Latin Small Letter E
    case 0xff46: return 0x66; //;Fullwidth Latin Small Letter F
    case 0xff47: return 0x67; //;Fullwidth Latin Small Letter G
    case 0xff48: return 0x68; //;Fullwidth Latin Small Letter H
    case 0xff49: return 0x69; //;Fullwidth Latin Small Letter I
    case 0xff4a: return 0x6a; //;Fullwidth Latin Small Letter J
    case 0xff4b: return 0x6b; //;Fullwidth Latin Small Letter K
    case 0xff4c: return 0x6c; //;Fullwidth Latin Small Letter L
    case 0xff4d: return 0x6d; //;Fullwidth Latin Small Letter M
    case 0xff4e: return 0x6e; //;Fullwidth Latin Small Letter N
    case 0xff4f: return 0x6f; //;Fullwidth Latin Small Letter O
    case 0xff50: return 0x70; //;Fullwidth Latin Small Letter P
    case 0xff51: return 0x71; //;Fullwidth Latin Small Letter Q
    case 0xff52: return 0x72; //;Fullwidth Latin Small Letter R
    case 0xff53: return 0x73; //;Fullwidth Latin Small Letter S
    case 0xff54: return 0x74; //;Fullwidth Latin Small Letter T
    case 0xff55: return 0x75; //;Fullwidth Latin Small Letter U
    case 0xff56: return 0x76; //;Fullwidth Latin Small Letter V
    case 0xff57: return 0x77; //;Fullwidth Latin Small Letter W
    case 0xff58: return 0x78; //;Fullwidth Latin Small Letter X
    case 0xff59: return 0x79; //;Fullwidth Latin Small Letter Y
    case 0xff5a: return 0x7a; //;Fullwidth Latin Small Letter Z
    case 0xff5b: return 0x7b; //;Fullwidth Left Curly Bracket
    case 0xff5c: return 0x7c; //;Fullwidth Vertical Line
    case 0xff5d: return 0x7d; //;Fullwidth Right Curly Bracket
    case 0xff5e: return 0x7e; //;Fullwidth Tilde
  }
  return 0x7F; //Sorry, can't help
}

//Decodes UTF-8 to CP1252. Accepts a single byte at a time and returns 0 until a full sequence has been decoded, then returns
//the CP1252 code. Returns 0x7F if input is invalid or codepoint unknown. Can handle UTF-16 only up to U+00FF.
//Negative input passed through unchanged.
inline int16_t Uto1252(int16_t inbyte) {
  static uint32_t u8cp=0; //Holds UTF-8 codepoint
  static uint8_t u8bytesleft=0; //Number of bytes left of current UTF-8 character
  static bool utf16=false; //Set when decoding UTF-16
  if (inbyte<0) return inbyte;
  if (utf16) {
    utf16=false;
    return inbyte; //Pass second byte of UTF-16 straight through
  } else {
    if (inbyte==0) { //First byte of UTF-16
     utf16=true;
     return 0; //Return 0 for first byte of UTF-16
    } else if (u8bytesleft==0) { //Not currently decoding UTF-8
      if (inbyte<0xC0) { //Up to 0x7F is simple ASCII, 0x80 to 0xBF is illegal for UTF-8, but we will pass it through as possibly-CP1252
        return inbyte; //Pass straight through
      } else if (inbyte<0xE0) { //UTF-8 two byte sequence start byte
        u8cp=inbyte-0xC0; //Remember the last five bits of the start bytec
        u8bytesleft=1; //One more byte to read
        return 0; //For nonce
      } else if (inbyte<0xF0) { //UTF-8 three byte sequence start byte
        u8cp=inbyte-0xE0; //Remember the last four bits of the start bytec
        u8bytesleft=2;
        return 0;
      } else if (inbyte<0xF8) { //UTF-8 four byte sequence start byte
        u8cp=inbyte-0xF0; //Remember the last three bits of the start bytec
        u8bytesleft=3;
        return 0;
      } else if (inbyte<0xFC) { //UTF-8 five byte sequence start byte
        u8cp=inbyte-0xF8; //Remember the last two bits of the start bytec
        u8bytesleft=4;
        return 0;
      } else if (inbyte<0xFE) { //UTF-8 six byte sequence start byte
        u8cp=inbyte-0xFC; //Remember the last bit of the start byte
        u8bytesleft=5;
        return 0;
      } else { //Invalid UTF-8 code
        return 0x7F;
      }
    } else { //Currently processing UTF-8
      if (inbyte<0x80) { //Invalid UTF-8 continuation byte
        u8bytesleft=0; //Exit UTF-8 decoding
        return 0x7F;
      } else if (inbyte<0xC0) {//Valid continuation byte of UTF-8
        u8cp<<=6; //Shift codepoint value 6 bits left
        u8cp|=inbyte&0x3F; //Add last 6 bits of byte codepoint
        u8bytesleft--; //Decrement the bytes-left counter
        if (u8bytesleft==0) {
          //End of UTF-8 code
          return Unicodeto1252(u8cp); //u8cp will be reset at start of next UTF-8 code
        }
        return 0;
      } else { //Invalid, possibly non-UTF 8-bit value
        u8bytesleft=0; //Exit UTF-8 decoding
        return 0x7F;
      }
    }
  }
}

String Uto1252string(const String &instring) {
  //Returns CP1252 translation of UTF-8 or UTF-16 input string
  //See Uto1252 declaration
  String outstring="";
  int16_t charval;
  outstring.reserve(instring.length());
  for (unsigned int i=0; i<instring.length(); i++) {
    charval=Uto1252(instring[i]);
    if (charval!=0) outstring+=char(charval);
  }
  return outstring;
}

//Decodes escape sequences and returns cp1252 code. Returns 0x7F if sequence is not recognized.
char escape(String &escstr) {
  if (escstr[0]=='#') {
    uint32_t retval=0;
    if (escstr[1]=='x' || escstr[1]=='X') retval=strtol(escstr.substring(2).c_str(),NULL,16); //Hexadecimal code
    else retval=escstr.substring(1).toInt(); //Decimal code
    if (retval==0) return 0x7F;
    return Unicodeto1252(retval);
  } 
  if (escstr=="Tab") return 9;
  else if (escstr=="NewLine") return 10;  
  else if (escstr=="nbsp") return ' ';
  else if (escstr=="quot") return '\"';
  else if (escstr=="amp") return '&';
  else if (escstr=="ast") return '*';
  else if (escstr=="lt") return '<';
  else if (escstr=="gt") return '>';
  else if (escstr=="lsqb") return '[';
  else if (escstr=="rsqb") return ']';
  else if (escstr=="grave") return '`';
  else if (escstr=="lcub") return '{';
  else if (escstr=="verbar") return '|';
  else if (escstr=="rcub") return '}';
  else if (escstr=="euro") return 128;
  else if (escstr=="sbquo") return 130;
  else if (escstr=="fnof") return 131;
  else if (escstr=="bdquo") return 132;
  else if (escstr=="hellip") return 133;
  else if (escstr=="dagger") return 134;
  else if (escstr=="Dagger") return 135;
  else if (escstr=="circ") return 136;
  else if (escstr=="permil") return 137;
  else if (escstr=="Scaron") return 138;
  else if (escstr=="lsaquo") return 139;
  else if (escstr=="OElig") return 140;
  else if (escstr=="Zcaron") return 142;
  else if (escstr=="lsquo") return 145;
  else if (escstr=="rsquo") return 146;
  else if (escstr=="ldquo") return 147;
  else if (escstr=="rdquo") return 148;
  else if (escstr=="bull") return 149;
  else if (escstr=="ndash") return 150;
  else if (escstr=="mdash") return 151;
  else if (escstr=="tilde") return 152;
  else if (escstr=="trade") return 153;
  else if (escstr=="scaron") return 154;
  else if (escstr=="rsaquo") return 155;
  else if (escstr=="oelig") return 156;
  else if (escstr=="zcaron") return 158;
  else if (escstr=="Yuml") return 159;
  else if (escstr=="iexcl") return 161;
  else if (escstr=="cent") return 162;
  else if (escstr=="pound") return 163;
  else if (escstr=="curren") return 164;
  else if (escstr=="yen") return 165;
  else if (escstr=="brvbar") return 166;
  else if (escstr=="sect") return 167;
  else if (escstr=="uml") return 168;
  else if (escstr=="copy") return 169;
  else if (escstr=="ordf") return 170;
  else if (escstr=="laquo") return 171;
  else if (escstr=="not") return 172;
  else if (escstr=="shy") return 173;
  else if (escstr=="reg") return 174;
  else if (escstr=="macr") return 175;
  else if (escstr=="deg") return 176;
  else if (escstr=="plusmn") return 177;
  else if (escstr=="sup2") return 178;
  else if (escstr=="sup3") return 179;
  else if (escstr=="acute") return 180;
  else if (escstr=="micro") return 181;
  else if (escstr=="para") return 182;
  else if (escstr=="middot") return 183;
  else if (escstr=="cedil") return 184;
  else if (escstr=="sup1") return 185;
  else if (escstr=="ordm") return 186;
  else if (escstr=="raquo") return 187;
  else if (escstr=="frac14") return 188;
  else if (escstr=="frac12") return 189;
  else if (escstr=="frac34") return 190;
  else if (escstr=="iquest") return 191;
  else if (escstr=="Agrave") return 192;
  else if (escstr=="Aacute") return 193;
  else if (escstr=="Acirc") return 194;
  else if (escstr=="Atilde") return 195;
  else if (escstr=="Auml") return 196;
  else if (escstr=="Aring") return 197;
  else if (escstr=="AElig") return 198;
  else if (escstr=="Ccedil") return 199;
  else if (escstr=="Eacute") return 201;
  else if (escstr=="Ecirc") return 202;
  else if (escstr=="Euml") return 203;
  else if (escstr=="Igrave") return 204;
  else if (escstr=="Iacute") return 205;
  else if (escstr=="Icirc") return 206;
  else if (escstr=="Iuml") return 207;
  else if (escstr=="ETH") return 208;
  else if (escstr=="Ntilde") return 209;
  else if (escstr=="Ograve") return 210;
  else if (escstr=="Oacute") return 211;
  else if (escstr=="Ocirc") return 212;
  else if (escstr=="Otilde") return 213;
  else if (escstr=="Ouml") return 214;
  else if (escstr=="times") return 215;
  else if (escstr=="Oslash") return 216;
  else if (escstr=="Ugrave") return 217;
  else if (escstr=="Uacute") return 218;
  else if (escstr=="Ucirc") return 219;
  else if (escstr=="Uuml") return 220;
  else if (escstr=="Yacute") return 221;
  else if (escstr=="THORN") return 222;
  else if (escstr=="szlig") return 223;
  else if (escstr=="agrave") return 224;
  else if (escstr=="aacute") return 225;
  else if (escstr=="acirc") return 226;
  else if (escstr=="atilde") return 227;
  else if (escstr=="auml") return 228;
  else if (escstr=="aring") return 229;
  else if (escstr=="aelig") return 230;
  else if (escstr=="ccedil") return 231;
  else if (escstr=="egrave") return 232;
  else if (escstr=="eacute") return 233;
  else if (escstr=="ecirc") return 234;
  else if (escstr=="euml") return 235;
  else if (escstr=="igrave") return 236;
  else if (escstr=="iacute") return 237;
  else if (escstr=="icirc") return 238;
  else if (escstr=="iuml") return 239;
  else if (escstr=="eth") return 240;
  else if (escstr=="ntilde") return 241;
  else if (escstr=="ograve") return 242;
  else if (escstr=="oacute") return 243;
  else if (escstr=="ocirc") return 244;
  else if (escstr=="otilde") return 245;
  else if (escstr=="ouml") return 246;
  else if (escstr=="divide") return 247;
  else if (escstr=="oslash") return 248;
  else if (escstr=="ugrave") return 249;
  else if (escstr=="uacute") return 250;
  else if (escstr=="ucirc") return 251;
  else if (escstr=="uuml") return 252;
  else if (escstr=="yacute") return 253;
  else if (escstr=="thorn") return 254;
  else if (escstr=="yuml") return 255;
  else return 0x7F;
}


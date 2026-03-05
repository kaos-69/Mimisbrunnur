# As of v0.0

## Can do (with limitations):

**Parse text files.**  
**Parse HTML and XHTML files.**  
**Parse styles of HTML/XHTML,** whether external stylesheets, internal, inline or WEB 1.0 tags.  
**Do margins, indents, bold, italic, underline and strikethrough** in accordance with these.  
**Parse .ncx files** (used in ebooks).  
**Handle ASCII, UTF-8, UTF-16 and (X)HTML escape sequences.**  
**Read and browse any of the above.**  
**Follow links in documents.**  
**Set/remove and go to bookmarks.**  
**Browse for documents on the SD card.**  

## Can not do (why not, how you can work around it, and will it change):

**Parse ebook files.** Shocking for an e-reader, I know. .ebook files are simply zipped archives, but Mímisbrunnur, as of even date, does not have the ability to deal with this. But if you preprocess them by unzipping them (various tools exist, including built in support in Windows and favourite [7zip](https://www.7-zip.org/)) you can browse and read the contents.  
**Parse pdb, prc, pdf or other file formats.** I recommend [Calibre](https://calibre-ebook.com/download) to convert those to ebook format and then unzipping. There are deep arbbit holes associated with each one, so I don't foresee native support being adde3d to Mímisbrunnur, useful at it would be.  
**Images of any kind.** This **will** probably be the next major update.  
**Colours.** This is primarily a hardware limitation, as the e-paper used is black-and-white.  
**Different fonts.** For a relatively low resolution display like this you need to use bitmap fonts. And they need to cover the character set you wish to use, in this case the CP1252 8-bit set. I settled on a single sans-serif proportional font, in normal, bold, italic and bold-italic. See comment at start of /code/Mimisbrunnur.ino for details. It is limiting though, so in the future I might add a serifed monospace "typewriter" font.  
**Different font sizes.** See above. For different font sizes you would have to make / clean up a complete bitmap font set. You might get away with "pixel doubling" for larger fonts, i.e. making each pixel in the font occupy 4 (or 9 or 16) pixels on screen. That would lead to jagged fonts though, and a very coarse stepping in font sizes. For the time being, the best we can do is using boldface as a stand in for larger font sizes.  
**Character sets / codepages other than CP1252.** After this project I could do a treatise on character sets, codepages and encodings. Partially because of the limitations of the software libraries I'm using, and partially because I didn't have time to dive down the rabbit holes of all the worlds writing systems, and partially because scope creep is already getting out of hand here, I decided to use an 8-bit character set. CP1252 is a Windows codepage that includes codepoints 0x00 to 0xFF from Unicode, but replaces the extended control characters 0x80 to 0x9F with symbols and letters. This allows display of most western latin based languages, including my beloved native Icelandic.  
**For UTF-16, only Unicode codepoints 0x00 to 0xFF will be correctly handled.** Not even the full CP1252. This is because of the extremely simplistic approach to UTF-16 decoding. In short, null characters are ignored, meaning that as long as the codepoint is <=0xFF it will treat the low-order byte as the complete codepoint. As you may surmise UTF-16 decoding has not been a priority for me.  

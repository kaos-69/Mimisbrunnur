# As of v0.0

## Can do (with limitations):

Parse text files.  
Parse HTML and XHTML files.  
Parse styles of HTML/XHTML, whether external stylesheets, internal, inline or WEB 1.0 tags.  
Do margins, indents, bold, italic, underline and strikethrough in accordance with these.  
Parse .ncx files (used in ebooks).  
Handle ASCII, UTF-8 and UTF-16.  
Read and browse any of the above.  
Follow links in documents.  
Set/remove and go to bookmarks.  
Browse for documents on the SD card.  

## Can not do:

Parse ebook files. Shocking for an e-reader, I know, but if you preprocess them by unzipping them you can browse and read the contents.  
Parse pdb, prc, pdf or other file formats. I recommend Calibre to convert those to ebook (and then unzipping).  
Images of any kind.  
Different fonts. There is one font, a sans-serif proportional one, and that's it. It is available in normal, bold, italic and bold-italic though.  
Different font sizes. The best we can currently do is using boldface as a stand in for larger font sizes.  
Character sets other than CP1252. CP1252 is an 8-bit character set that includes codepoints 0x00 to 0xFF from Unicode, but replaces the extended control characters 0x80 to 0x9F with symbols and letters. This allows display of most western latin based languages.  
For UTF-16, only Unicode codepoints 0x00 to 0xFF will be correctly handled.  
 

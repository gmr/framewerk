<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output encoding="ISO-8859-15" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />
  <xsl:template match="/">
   <ul>
    <xsl:for-each select="/Options/Option">
     <li>
      <xsl:element name="a">
       <xsl:attribute name="href">
        <xsl:value-of select="@url" />
       </xsl:attribute>
       <xsl:value-of select="." />
      </xsl:element>   
     </li>
    </xsl:for-each>
   </ul>
 </xsl:template>
</xsl:stylesheet>

<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output omit-xml-declaration="yes" method="xml" version="1.0" encoding="UTF-8" indent="yes" />
  
  <xsl:template match="/menu">
   <ul>
    <xsl:apply-templates select="item[@title=//@select]/item" />
   </ul>
  </xsl:template>

  <xsl:template match="item">
   <xsl:element name="li">
    <xsl:if test="string-length(@class) &gt; 0">
     <xsl:attribute name="class"><xsl:value-of select="@class" /></xsl:attribute>
    </xsl:if>
    <xsl:choose>
     <xsl:when test="@url">
      <xsl:element name="a">
       <xsl:attribute name="href"><xsl:value-of select="@url" /></xsl:attribute>
       <xsl:value-of select="@title" />
      </xsl:element>
     </xsl:when>
     <xsl:otherwise>
      <xsl:value-of select="@title" />
     </xsl:otherwise>
    </xsl:choose>
   </xsl:element>
  </xsl:template>

</xsl:stylesheet>

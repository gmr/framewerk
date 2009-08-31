<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output encoding="UTF-8" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />

 <!-- Debug Messages -->
 <xsl:template match="debug">
  <div class="debugging">
   <h1><a href="#">+ Debugging Information</a></h1>
   <dl>
    <xsl:apply-templates select="debug-message" />
   </dl>
  </div>
 </xsl:template>

 <xsl:template match="debug-message">
  <dt><xsl:value-of select="object" /> (<xsl:value-of select="line" />)</dt>
  <dd><xsl:value-of select="value" /></dd>
 </xsl:template>

 <!-- Warning Messages -->
 <xsl:template match="warnings">
  <div class="warnings">
   <h1>Critical Engine Warnings</h1>
   <ol>
    <xsl:apply-templates select="warning" />
   </ol>
  </div>
 </xsl:template>

 <xsl:template match="warning">
  <li><xsl:value-of select="value" /></li>
 </xsl:template>

 <!-- Profile Messages -->
 <xsl:template match="profile">
  <div class="profiling">
   <h1><a href="#">+ Profiling Information in Seconds</a></h1>
   <dl>
    <xsl:apply-templates select="profile-message" />
   </dl>
  </div>
 </xsl:template>

 <xsl:template match="profile-message">
  <dt><xsl:value-of select="value" /></dt>
  <dd><xsl:value-of select="timestamp" /> sec</dd>
 </xsl:template>

</xsl:stylesheet>

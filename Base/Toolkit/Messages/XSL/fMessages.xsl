<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output encoding="UTF-8" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />
  <!-- Root node -->
  <xsl:template match="messages">
    <div id="messages">
      <xsl:apply-templates select="message" />
    </div>
  </xsl:template>
  <!-- Message node -->
  <xsl:template match="message">
    <div class="{style}"><xsl:text>{!i18n:</xsl:text><xsl:value-of select="body" /><xsl:text>}</xsl:text><xsl:if test="@age"> (<xsl:value-of select="@age" />)</xsl:if></div>
  </xsl:template>
</xsl:stylesheet>
<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id: tjQueryLiveGrid.xsl 920 2007-08-06 18:58:52Z dallas $ -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output omit-xml-declaration="yes" method="xml" version="1.0" encoding="UTF-8" indent="yes" />

  <xsl:template match="//liveGridData[@format='html']">
   <xsl:element name="table">
    <xsl:attribute name="id"><xsl:value-of select="@table" /></xsl:attribute>
    <thead>
     <xsl:for-each select="columns/column">
      <th>
       <xsl:value-of select="." />
      </th>
     </xsl:for-each>
    </thead>
    <tbody>
    <!--
     <xsl:for-each select="row">
      <tr>
       <xsl:for-each select="*">
        <td>
         <xsl:value-of select="." />
        </td>
       </xsl:for-each>
      </tr>
     </xsl:for-each>
     -->
    </tbody>
   </xsl:element>
  </xsl:template>
  <xsl:template match="//liveGridData[@format='xml']">
   <xsl:element name="rows">
    <page><xsl:value-of select="(@page + 1)" /></page>
    <total><xsl:value-of select="@pages" /></total>
    <xsl:for-each select="row">
     <xsl:element name="row">
      <xsl:attribute name="id"><xsl:value-of select="@rownum" /></xsl:attribute>
      <xsl:for-each select="*">
       <cell>
        <xsl:value-of select="." />
       </cell>
      </xsl:for-each>
     </xsl:element>
    </xsl:for-each>
   </xsl:element>
  </xsl:template>

</xsl:stylesheet>
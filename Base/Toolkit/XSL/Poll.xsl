<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output encoding="UTF-8" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />
<!--/* TPoll XML stylesheet
     * $Id: Poll.xsl 858 2007-05-27 17:44:18Z gmr $
     */-->  

 <xsl:template match="TPoll">
  <div class="TPoll">
   <xsl:if test="not(@hidetitle) or string-length(title) = 0"><h3><a name="{@id}"/><xsl:value-of select="title"/></h3></xsl:if>
   <table class="TPoll_results" align="center" border="0">
    <tr><td colspan="4" class="TPoll_question"><xsl:value-of select="question"/></td></tr>
    <xsl:for-each select="choices/choice">
     <xsl:sort select="@value" />
     <tr>
      <xsl:element name="td">
       <xsl:attribute name="class">TPoll_choice<xsl:if test="@myvote"> TPoll_mine</xsl:if></xsl:attribute>
       <xsl:attribute name="valign">top</xsl:attribute>
       <xsl:if test="@myvote">
        <xsl:attribute name="title">
         You voted for this choice on {@ftime:<xsl:value-of select="@myvote"/>}
        </xsl:attribute>
       </xsl:if>
       <xsl:value-of select="."/>
      </xsl:element>
      <td class="TPoll_bar" nowrap="1" valign="middle" width="{/TPoll/@barWidth}">
       <img align="middle" ALT="Poll" width="4" height="10" src="{{@themeGraphicsDirectory}}poll_leftCap.gif" /><xsl:if test="@width > 15"><xsl:element name="img">
         <xsl:attribute name="align">middle</xsl:attribute>
         <xsl:attribute name="alt">Poll</xsl:attribute>
         <xsl:attribute name="width"><xsl:value-of select="floor((@width - 15) div 2)" /></xsl:attribute>
         <xsl:attribute name="height">10</xsl:attribute>
         <xsl:attribute name="src">{@themeGraphicsDirectory}poll_left.gif</xsl:attribute>
        </xsl:element></xsl:if><img align="middle" ALT="Poll" width="7" height="10" src="{{@themeGraphicsDirectory}}poll_middle.gif" /><xsl:if test="@width > 15"><xsl:element name="img">
         <xsl:attribute name="align">middle</xsl:attribute>
         <xsl:attribute name="alt">Poll</xsl:attribute>
         <xsl:attribute name="width"><xsl:value-of select="floor((@width - 15) div 2)" /></xsl:attribute>
         <xsl:attribute name="height">10</xsl:attribute>
         <xsl:attribute name="src">{@themeGraphicsDirectory}poll_right.gif</xsl:attribute>
        </xsl:element></xsl:if><img align="middle" ALT="Poll" width="4" height="10" src="{{@themeGraphicsDirectory}}poll_rightCap.gif" />
      </td>
      <td class="TPoll_percent">
       <xsl:value-of select="round(10 * @percent) div 10"/>%
      </td>
      <td class="TPoll_voteCount">
       [ <xsl:value-of select="@votes"/> ]
      </td>
     </tr>
    </xsl:for-each>
    <tr><td colspan="3"/><td class="TPoll_totalVotes">[ <xsl:value-of select="@totalVotes"/> ]</td></tr>
   </table>
   
  </div>
 </xsl:template>

</xsl:stylesheet>

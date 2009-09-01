<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output encoding="UTF-8" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />
 <xsl:param name="SITE_URL" />
 <xsl:param name="PHOTO_URL" />
 <xsl:param name="IMAGE_URL" />

 <xsl:template match="tLadder">
   <div class="formLeft">
    <h1>
     <xsl:value-of select="@title" />&amp;nbsp;<xsl:value-of select="@name" />
    </h1>
    <xsl:value-of select="tForm" />
    <p>
     <xsl:if test="@title='Edit'">
      <a href="{@baseURI}">Add new <xsl:value-of select="@name" /></a>
     </xsl:if>
    </p>
   </div>
   <div class="formRight">
     <h1><xsl:value-of select="@name" /> List</h1>
     <table class="dataTable">
       <tr class="tableHeader">
         <td width="20"></td>
         <xsl:for-each select="table/headers/header">
           <td>
             <xsl:value-of select="." />
           </td>
         </xsl:for-each>
       </tr>
       <xsl:variable name="onClick">
         <xsl:text>return confirm('Delete this </xsl:text><xsl:value-of select="/tLadder/@noun" /><xsl:text>?');</xsl:text>
       </xsl:variable>
       <xsl:variable name="deleteBase">
         <xsl:value-of select="/tLadder/@baseURI" /><xsl:text>/delete/</xsl:text>
       </xsl:variable>
       <xsl:variable name="editBase">
         <xsl:value-of select="/tLadder/@baseURI" /><xsl:text>/edit/</xsl:text>
       </xsl:variable>
       <xsl:for-each select="table/rows/row">
         <tr>
           <td>
             <xsl:choose>
               <xsl:when test="/tLadder/@allowDelete = '1'">
                 <a>
                   <xsl:attribute name="href">
                     <xsl:value-of select="$deleteBase" /><xsl:value-of select="@key" />
                   </xsl:attribute>
                   <xsl:attribute name="onclick">
                     <xsl:value-of select="$onClick" />
                   </xsl:attribute>
                  <img>
                   <xsl:attribute name="src"><xsl:value-of select="$IMAGE_URL" />icons/delete.png</xsl:attribute>
                   <xsl:attribute name="alt">Delete</xsl:attribute>
                  </img>
                 </a>
               </xsl:when>
               <xsl:otherwise><xsl:text> </xsl:text></xsl:otherwise>
             </xsl:choose>
           </td>
           <xsl:for-each select="column">
             <td>
               <xsl:choose>
                 <xsl:when test="@displayOnly = '1' or /tLadder/@allowEdit != '1'">
                   <xsl:value-of select="." />
                 </xsl:when>
                 <xsl:otherwise>
                   <a>
                     <xsl:attribute name="href"><xsl:value-of select="$editBase" /><xsl:value-of select="../@key" /></xsl:attribute>
                     <xsl:value-of select="." />
                   </a>
                 </xsl:otherwise>
               </xsl:choose>
             </td>
           </xsl:for-each>
         </tr>
       </xsl:for-each>
     </table>
   </div>
 </xsl:template>

</xsl:stylesheet>

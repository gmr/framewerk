<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output encoding="UTF-8" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />
  <!--
    Debug Error
  -->
  <xsl:template match="exception">
    <h2><xsl:value-of select="error" /></h2>
    <!-- Description -->
    <div class="section description">
      <p>
        <strong>Description: </strong>
        <xsl:value-of select="description" />
      </p>
    </div>
    <!-- Exception Message -->
    <div class="section message">
      <p>
        <strong>Message: </strong>
        <xsl:value-of select="message" disable-output-escaping="yes" />
      </p>
    </div>
    <!-- Source Error -->
    <xsl:if test="count(code/backtrace) > 0">
      <h3>Backtrace</h3>
      <p>Functions called in order of most recent to least:</p>
       <ol class="code">
        <xsl:for-each select="code/backtrace">
         <li><span class="function">
           <xsl:value-of select="class" />
           <xsl:text>::</xsl:text>
           <xsl:value-of select="function" />
           <xsl:text>(</xsl:text>
           <xsl:value-of disable-output-escaping="yes" select="args" />
           <xsl:text>)</xsl:text>
          </span>
          <xsl:text> in </xsl:text>
          <i><xsl:value-of select="file" /></i>
          <xsl:text> on line </xsl:text>
          <xsl:value-of select="@number" />
          <xsl:if test="string-length(source) > 0">
           <div class="source">
            <xsl:value-of disable-output-escaping="yes" select="source" />
           </div>
          </xsl:if>
          <xsl:for-each select="argPopup/popup">
           <xsl:element name="div">
            <xsl:attribute name="class">popup</xsl:attribute>
            <xsl:attribute name="id"><xsl:value-of select="@id" /></xsl:attribute>
            <xsl:value-of disable-output-escaping="yes" select="." />
           </xsl:element>
          </xsl:for-each>
         </li>
        </xsl:for-each>
       </ol>
    </xsl:if>
    <!-- Source File -->
    <xsl:if test="string-length(file) > 0">
      <div class="section file">
        <strong>Source file: </strong>
        <xsl:value-of select="file" />
      </div>
    </xsl:if>
    <!-- Source Line -->
    <xsl:if test="string-length(file) > 0">
      <div class="section line">
        <strong>Source line: </strong>
        <xsl:value-of select="line" />
      </div>
    </xsl:if>
    <!--Stack Trace -->
    <xsl:if test="count(trace/item) > 0">
      <div class="section trace">
        <h3>Stack Trace</h3>
        <p>
          <div class="code">
            <xsl:apply-templates select="trace/item" />
          </div>
        </p>
      </div>
    </xsl:if>
    <!--
    <hr />
    <div class="section version">
      <p>
        <strong>Server Information: </strong>
        <?php print_r($_SERVER); ?>
        Framewerk SVN (Revision); PHP
      </p>
    </div>
    <div class="section client">
      <p>
        <strong>Client Information: </strong>
      </p>
    </div>
    -->
  </xsl:template>

  <xsl:template match="item">
    <xsl:value-of select="class" />
    <xsl:value-of select="type" />
    <xsl:value-of select="function" />
    <xsl:variable name="items" select="count(args/item)" />
    <xsl:text>(</xsl:text>
    <xsl:for-each select="args/item">
      <xsl:value-of select="." />
      <xsl:if test="position() &lt; $items">, </xsl:if>
    </xsl:for-each>
    <xsl:text>)</xsl:text>

    [<xsl:value-of select="file" />:<xsl:value-of select="line" />]
    <br />
  </xsl:template>

  <!--

    Simple Error

  -->

  <xsl:template match="simple">
    <div class="simple">
      <h1>404 Not Found</h1>
      <div class="description">
        <p>The server could not find the object or file you requested.</p>
      </div>
      <hr />
      <div class="message">
        <p>File not found: /234234</p>
      </div>
    </div>
  </xsl:template>

</xsl:stylesheet>

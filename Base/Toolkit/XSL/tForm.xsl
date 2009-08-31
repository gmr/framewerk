<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output encoding="UTF-8" omit-xml-declaration="yes" method="xml" version="1.0" indent="yes" />

 <!-- Top Level Form Match -->
 <xsl:template match="tForm">
  <xsl:element name="form">
   <!-- Add our attributes -->
   <xsl:attribute name="action"><xsl:value-of select="@action"/></xsl:attribute>
   <xsl:attribute name="enctype"><xsl:value-of select="@enctype"/></xsl:attribute>
   <xsl:attribute name="id">
    <xsl:choose>
     <xsl:when test="string-length(@id) &gt; 0">
      <xsl:value-of select="@id"/>
     </xsl:when>
     <xsl:otherwise>
      <xsl:value-of select="@name"/>
     </xsl:otherwise>
    </xsl:choose>
   </xsl:attribute>
   <xsl:attribute name="method"><xsl:value-of select="@method"/></xsl:attribute>
   
   <!-- Create our definition list -->
   <xsl:element name="div">
    <!-- Add our class if it exists -->
    <xsl:if test="string-length(@class) &gt; 0">
     <xsl:attribute name="class"><xsl:value-of select="@class" /></xsl:attribute>
    </xsl:if>
    <!-- Apply our field templates -->
    <ul>
    <xsl:apply-templates select="*" />
    </ul>
   </xsl:element>
  </xsl:element>
  <!-- Dirty Hack to make forms not muck up the rest of pages since the css has things floating -->
  <div style="clear: left;">
   <xsl:text> </xsl:text>
  </div>
 </xsl:template>

 <xsl:template match="fieldset">
   <fieldset>
     <xsl:if test="string-length(@class) &gt; 0"><xsl:attribute name="class"><xsl:value-of select="@class" /></xsl:attribute></xsl:if>
     <xsl:if test="string-length(@legend) &gt; 0"><legend><xsl:value-of select="@legend" /></legend></xsl:if>
     <ul>
       <xsl:apply-templates select="*" />
     </ul>
   </fieldset>
 </xsl:template>
 
 <xsl:template match="container">
  <xsl:choose>
   <xsl:when test="string-length(@type) &gt; 0">
    <xsl:element name="{@type}">
     <xsl:if test="string-length(@class) &gt; 0">
      <xsl:attribute name="class"><xsl:value-of select="@class" /></xsl:attribute>
     </xsl:if>
     <xsl:apply-templates select="*" />
    </xsl:element>
   </xsl:when>
   <xsl:otherwise>
    <xsl:element name="div">
     <xsl:if test="string-length(@class) &gt; 0">
      <xsl:attribute name="class"><xsl:value-of select="@class" /></xsl:attribute>
     </xsl:if>
     <xsl:apply-templates select="*" />
    </xsl:element>
   </xsl:otherwise>
  </xsl:choose>
 </xsl:template>

 <!-- Return a form label -->
 <xsl:template name="returnLabel">
  <xsl:param name="node"/>
  <xsl:param name="class" />
  <xsl:param name="value" />
  <xsl:param name="for" />
   <xsl:if test="//tForm/@showLabels = 1 and not($node/@hidelabel)">
    <xsl:element name="label">
     <xsl:if test="boolean($node/@required) or string-length($class) &gt; 0">
      <xsl:attribute name="class">
       <xsl:if test="boolean($node/@required)">required</xsl:if>
       <xsl:if test="string-length($class) &gt; 0">
        <xsl:value-of select="$class" />
       </xsl:if>
      </xsl:attribute>
     </xsl:if>
     <xsl:attribute name="for">
      <xsl:choose>
       <xsl:when test="not($for)">
        <xsl:value-of select="$node/@name" />
       </xsl:when>
       <xsl:otherwise>
        <xsl:value-of select="$for" />
       </xsl:otherwise>
      </xsl:choose>
     </xsl:attribute>
     <xsl:if test="string-length($node/@key) &gt; 0">
      <xsl:attribute name="accesskey"><xsl:value-of select="$node/@key" /></xsl:attribute>
     </xsl:if>
     <xsl:choose>
      <xsl:when test="string-length($value) &gt; 0">
       <xsl:text>{!i18n:</xsl:text><xsl:value-of disable-output-escaping="yes" select="$value" /><xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:otherwise>
       <xsl:text>{!i18n:</xsl:text><xsl:value-of disable-output-escaping="yes" select="$node/@label" /><xsl:text>}</xsl:text>
      </xsl:otherwise>
     </xsl:choose>
     <xsl:if test="boolean($node/@required)"><sup>*</sup></xsl:if>
    </xsl:element>
   </xsl:if>
 </xsl:template>

 <!-- Generic Text Field Handler -->
 <xsl:template name="textInput">
  <xsl:param name="class" />
  <xsl:param name="type">text</xsl:param>
  <xsl:param name="before" />
  <xsl:param name="after" />
  <xsl:element name="li">
   <xsl:call-template name="returnLabel">
    <xsl:with-param name="node" select="." />
   </xsl:call-template>
   <xsl:if test="@error">
    <xsl:attribute name="class">error</xsl:attribute>
   </xsl:if>
   <xsl:if test="string-length($before) &gt; 0">
    <xsl:value-of disable-output-escaping="yes" select="$before" />
   </xsl:if>
   <xsl:element name="input">
    <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
    <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
    <xsl:attribute name="type"><xsl:value-of select="$type" /></xsl:attribute>
    <xsl:attribute name="class">
     <xsl:call-template name="buildClass">
      <xsl:with-param name="node" select="." />
     </xsl:call-template>
    </xsl:attribute>
    <xsl:if test="string-length(@size) &gt; 0">
     <xsl:attribute name="size"><xsl:value-of select="@size" /></xsl:attribute>
    </xsl:if>
    <xsl:if test="string-length(@maxlength) &gt; 0">
     <xsl:attribute name="maxlength"><xsl:value-of select="@maxlength" /></xsl:attribute>
    </xsl:if>
    <xsl:if test="string-length(@disabled) &gt; 0">
     <xsl:attribute name="disabled"><xsl:value-of select="@disabled" /></xsl:attribute>
    </xsl:if>
    <xsl:if test="string-length(.) &gt; 0">
     <xsl:attribute name="value"><xsl:value-of disable-output-escaping="yes" select="." /></xsl:attribute>
    </xsl:if>
   </xsl:element>
   <xsl:if test="string-length($after) &gt; 0">
    <xsl:value-of disable-output-escaping="yes" select="$after" />
   </xsl:if>
  </xsl:element>
 </xsl:template>

 <!-- TTextField -->
 <xsl:template match="field[@type='tTextField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="class">tTextField</xsl:with-param>
  </xsl:call-template>
 </xsl:template>

 <!-- TEmailField -->
 <xsl:template match="field[@type='tEmailField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="class">tEmailField</xsl:with-param>
  </xsl:call-template>
 </xsl:template>

 <!-- TPasswordField -->
 <xsl:template match="field[@type='tPasswordField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="type">password</xsl:with-param>
   <xsl:with-param name="class">tPasswordField</xsl:with-param>
  </xsl:call-template>
  <xsl:if test="string-length(@create) &gt; 0">
   <li class="underLabel">Password Strength: <span id="strength">Unchecked</span></li>
  </xsl:if>
 </xsl:template>

 <!-- TFileUploadField -->
 <xsl:template match="field[@type='tFileUploadField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="type">file</xsl:with-param>
   <xsl:with-param name="class">tFileUploadField</xsl:with-param>
  </xsl:call-template>
 </xsl:template>

 <!-- TCaptchaField -->
 <xsl:template match="field[@type='tCaptchaField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="class">tCaptchaField</xsl:with-param>
   <xsl:with-param name="before"><![CDATA[<ul class="tCaptchaField"><li>{@captchaMarkup}</li><li>]]></xsl:with-param>
   <xsl:with-param name="after"><![CDATA[</li></ul>]]></xsl:with-param>
  </xsl:call-template>
 </xsl:template>

 <!-- tPhoneField -->
 <xsl:template match="field[@type='tPhoneField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="class">tPhoneFieldExt</xsl:with-param>
  </xsl:call-template>
 </xsl:template>
 
 
 <!-- TDateField -->
 <xsl:template match="field[@type='tDateField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="class">tDateField</xsl:with-param>
  </xsl:call-template>
  <xsl:if test="string-length(@format) &gt; 0">
   <li class="underLabel">Format: <xsl:value-of select="@format" /></li>
  </xsl:if>
 </xsl:template>
 
  <!-- TDateField -->
 <xsl:template match="field[@type='tTimeField']">
  <xsl:call-template name="textInput">
   <xsl:with-param name="class">tTimeField</xsl:with-param>
  </xsl:call-template>
  <xsl:if test="string-length(@format) &gt; 0">
   <li class="underLabel">Format: <xsl:value-of select="@format" /></li>
  </xsl:if>
 </xsl:template>

 <!-- TSelectField -->
 <xsl:template match="field[@type='tSelectField']">
  <xsl:element name="li">
   <xsl:call-template name="returnLabel">
    <xsl:with-param name="node" select="." />
   </xsl:call-template>
   <xsl:if test="@error">
    <xsl:attribute name="class">error</xsl:attribute>
   </xsl:if>
   <xsl:element name="select">
    <xsl:attribute name="name">
      <xsl:value-of select="@name" />
      <!--  if it's a multi-select, make the name an array[] -->
      <xsl:if test="@multiple = 'true'">
        <xsl:text>[]</xsl:text>
      </xsl:if>
    </xsl:attribute>
    <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
    <xsl:attribute name="size"><xsl:value-of select="@size" /></xsl:attribute>
    <xsl:attribute name="class">
     <xsl:call-template name="buildClass">
      <xsl:with-param name="node" select="." />
     </xsl:call-template>
    </xsl:attribute>
    <xsl:if test="@multiple = 'true'">
     <xsl:attribute name="multiple">multiple</xsl:attribute>
    </xsl:if>
    <xsl:if test="string-length(@init_label) &gt; 0">
     <xsl:element name="option">
      <xsl:attribute name="value" />
      <xsl:if test="count(option[@selected=1]) &gt; 0">
       <xsl:attribute name="selected">selected</xsl:attribute>
      </xsl:if>
      <xsl:value-of select="@init_label" />
     </xsl:element>
    </xsl:if>
    <xsl:for-each select="option">
     <xsl:element name="option">
      <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
      <xsl:if test="@selected = 1">
       <xsl:attribute name="selected">selected</xsl:attribute>
      </xsl:if>
      <xsl:value-of select="@label" />
     </xsl:element>
    </xsl:for-each>
   </xsl:element>
  </xsl:element>
 </xsl:template>

 <!-- THiddenField -->
 <xsl:template match="field[@type='tHiddenField']">
  <li style="display: none;"><input type="hidden" id="{@name}" name="{@name}" value="{.}" /></li>
 </xsl:template>

 <!-- THeadlineField -->
 <xsl:template match="field[@type='tHeadlineField']">
  <li>
  <xsl:text>{!i18n:</xsl:text>
  <xsl:element name="{@htype}">
   <xsl:value-of select="." />
  </xsl:element>
  <xsl:text>}</xsl:text>
  </li>
 </xsl:template>

 <!-- TInstructionalField -->
 <xsl:template match="field[@type='tInstructionalField']">
  <xsl:element name="p">
   <xsl:attribute name="class">
    <xsl:call-template name="buildClass">
     <xsl:with-param name="node" select="." />
    </xsl:call-template>
   </xsl:attribute>
   <xsl:text>{!i18n:</xsl:text><xsl:value-of select="." /><xsl:text>}</xsl:text>
  </xsl:element>
 </xsl:template>

 <!-- TCheckField -->
 <xsl:template match="field[@type='tCheckField']">
  <xsl:element name="li">
   <xsl:choose>
    <xsl:when test="@noLabel != '1'">
     <xsl:element name="dl">
     <xsl:element name="dt"><xsl:text> </xsl:text></xsl:element>
     <xsl:element name="dd">
      <xsl:element name="input">
       <xsl:attribute name="type">checkbox</xsl:attribute>
       <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
       <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
       <xsl:attribute name="class">
        <xsl:call-template name="buildClass">
         <xsl:with-param name="node" select="." />
        </xsl:call-template>
       </xsl:attribute>
       <xsl:if test="@checked = '1'">
        <xsl:attribute name="checked">checked</xsl:attribute>
       </xsl:if>
       <xsl:if test="string-length(.) &gt; 0">
        <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
       </xsl:if>
      </xsl:element>
      <xsl:element name="label">
       <xsl:attribute name="for"><xsl:value-of select="@id"/></xsl:attribute>
       <xsl:attribute name="class">
        <xsl:call-template name="buildClass">
         <xsl:with-param name="node" select="." />
         <xsl:with-param name="addClass">rightLabel</xsl:with-param>
        </xsl:call-template>
       </xsl:attribute>
       <xsl:text>{!i18n:</xsl:text><xsl:value-of select="@label"/><xsl:text>}</xsl:text>
      </xsl:element>
     </xsl:element>
    </xsl:element>
   </xsl:when>
   <xsl:otherwise>
    <xsl:attribute name="class">
     <xsl:call-template name="buildClass">
      <xsl:with-param name="node" select="." />
      <xsl:with-param name="addClass">fullWidth</xsl:with-param>
     </xsl:call-template>
    </xsl:attribute>
    <xsl:element name="input">
     <xsl:attribute name="type">checkbox</xsl:attribute>
     <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
     <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
     <xsl:attribute name="class">
      <xsl:call-template name="buildClass">
       <xsl:with-param name="node" select="." />
      </xsl:call-template>
     </xsl:attribute>
     <xsl:if test="@checked = '1'">
      <xsl:attribute name="checked">checked</xsl:attribute>
     </xsl:if>
     <xsl:if test="string-length(.) &gt; 0">
      <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
     </xsl:if>
     </xsl:element>
     <xsl:element name="label">
      <xsl:attribute name="for"><xsl:value-of select="@id"/></xsl:attribute>
      <xsl:attribute name="class">
       <xsl:text>rightLabel </xsl:text>
       <xsl:if test="boolean(@required)">required</xsl:if>
      </xsl:attribute>
      <xsl:text>{!i18n:</xsl:text><xsl:value-of select="@label"/><xsl:text>}</xsl:text>
     </xsl:element>
    </xsl:otherwise>
   </xsl:choose>
  </xsl:element>
 </xsl:template>

 <!-- tRadioField -->
 <xsl:template match="field[@type='tRadioField']">
  <xsl:element name="li">
   <xsl:choose>
    <xsl:when test="@noLabel != '1'">
     <xsl:element name="dl">
     <xsl:element name="dt"><xsl:text> </xsl:text></xsl:element>
     <xsl:element name="dd">
      <xsl:element name="input">
       <xsl:attribute name="type">radio</xsl:attribute>
       <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
       <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
       <xsl:attribute name="class">
        <xsl:call-template name="buildClass">
         <xsl:with-param name="node" select="." />
        </xsl:call-template>
       </xsl:attribute>
       <xsl:if test="@checked = '1'">
        <xsl:attribute name="checked">checked</xsl:attribute>
       </xsl:if>
       <xsl:if test="string-length(.) &gt; 0">
        <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
       </xsl:if>
      </xsl:element>
      <xsl:element name="label">
       <xsl:attribute name="for"><xsl:value-of select="@id"/></xsl:attribute>
       <xsl:attribute name="class">
        <xsl:call-template name="buildClass">
         <xsl:with-param name="node" select="." />
         <xsl:with-param name="addClass">rightLabel</xsl:with-param>
        </xsl:call-template>
       </xsl:attribute>
       <xsl:value-of select="@label"/>
      </xsl:element>
     </xsl:element>
    </xsl:element>
   </xsl:when>
   <xsl:otherwise>
    <xsl:attribute name="class">
     <xsl:call-template name="buildClass">
      <xsl:with-param name="node" select="." />
      <xsl:with-param name="addClass">fullWidth</xsl:with-param>
     </xsl:call-template>
    </xsl:attribute>
    <xsl:element name="input">
     <xsl:attribute name="type">radio</xsl:attribute>
     <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
     <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
     <xsl:attribute name="class">
      <xsl:call-template name="buildClass">
       <xsl:with-param name="node" select="." />
      </xsl:call-template>
     </xsl:attribute>
     <xsl:if test="@checked = '1'">
      <xsl:attribute name="checked">checked</xsl:attribute>
     </xsl:if>
     <xsl:if test="string-length(.) &gt; 0">
      <xsl:attribute name="value"><xsl:value-of select="." /></xsl:attribute>
     </xsl:if>
     </xsl:element>
     <xsl:element name="label">
      <xsl:attribute name="for"><xsl:value-of select="@id"/></xsl:attribute>
      <xsl:attribute name="class">
       <xsl:text>rightLabel </xsl:text>
       <xsl:if test="boolean(@required)">required</xsl:if>
      </xsl:attribute>
      <xsl:value-of select="@label"/>
     </xsl:element>
    </xsl:otherwise>
   </xsl:choose>
  </xsl:element>
 </xsl:template>

 <!-- TDisplayField -->
 <xsl:template match="field[@type='tDisplayField']">
  <xsl:element name="li">
   <xsl:call-template name="returnLabel">
    <xsl:with-param name="node" select="." />
   </xsl:call-template>
   <xsl:value-of disable-output-escaping="yes" select="." />
  </xsl:element>
 </xsl:template>

 <!-- TSubmitField -->
 <xsl:template match="field[@type='tSubmitField']">
  <li class="buttonli">
   <xsl:element name="input">
    <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
    <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
    <xsl:attribute name="type">submit</xsl:attribute>
    <xsl:attribute name="class">
     <xsl:call-template name="buildClass">
      <xsl:with-param name="node" select="." />
      <xsl:with-param name="addClass">TSubmitField</xsl:with-param>
     </xsl:call-template>
    </xsl:attribute>
    <xsl:attribute name="value">
     <xsl:choose>
      <xsl:when test="string-length(@label) &gt; 0">
       <xsl:value-of select="@label" />
      </xsl:when>
      <xsl:when test="string-length(.) &gt; 0">
       <xsl:value-of select="." />
      </xsl:when>
      <xsl:otherwise>
       <xsl:text> Submit </xsl:text>
      </xsl:otherwise>
     </xsl:choose>
    </xsl:attribute>
   </xsl:element>
  </li>
 </xsl:template>

 <!-- TResetField -->
 <xsl:template match="field[@type='tResetField']">
  <li class="buttonli">
   <xsl:element name="input">
    <xsl:attribute name="id"><xsl:value-of select="@name" /></xsl:attribute>
    <xsl:attribute name="name"><xsl:value-of select="@name" /></xsl:attribute>
    <xsl:attribute name="type">reset</xsl:attribute>
    <xsl:attribute name="class">
     <xsl:call-template name="buildClass">
      <xsl:with-param name="node" select="." />
      <xsl:with-param name="addClass">TResetField</xsl:with-param>
     </xsl:call-template>
    </xsl:attribute>
    <xsl:attribute name="value">
     <xsl:choose>
      <xsl:when test="string-length(@label) &gt; 0">
       <xsl:value-of select="@label" />
      </xsl:when>
      <xsl:when test="string-length(.) &gt; 0">
       <xsl:value-of select="." />
      </xsl:when>
      <xsl:otherwise>
       <xsl:text> Clear </xsl:text>
      </xsl:otherwise>
     </xsl:choose>
    </xsl:attribute>
   </xsl:element>
  </li>
 </xsl:template>

 <!-- TWysiwygField -->
 <xsl:template match="field[@type='tWysiwygField']">
  <xsl:call-template name="returnLabel">
   <xsl:with-param name="node" select="." />
  </xsl:call-template>
  <xsl:element name="li">
   <xsl:if test="@error">
    <xsl:attribute name="class">error</xsl:attribute>
    <xsl:attribute name="title"><xsl:value-of select="@error"/></xsl:attribute>
   </xsl:if>
   <xsl:choose>
    <xsl:when test="@editor = 'FCKeditor'">
     <xsl:value-of disable-output-escaping="yes" select="." />
    </xsl:when>
    <xsl:otherwise>
     <xsl:element name="textarea">
      <xsl:attribute name="id">
       <xsl:choose>
        <xsl:when test="boolean(number(@id))">
         <xsl:value-of select="@id"/>
        </xsl:when>
        <xsl:otherwise>
         <xsl:value-of select="@name"/>
        </xsl:otherwise>
       </xsl:choose>
      </xsl:attribute>
      <xsl:attribute name="class">
       <xsl:call-template name="buildClass">
        <xsl:with-param name="node" select="." />
       </xsl:call-template>
      </xsl:attribute>
      <xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
      <xsl:attribute name="style"><xsl:value-of select="@style"/></xsl:attribute>
      <xsl:if test="boolean(number(@rows))">
        <xsl:attribute name="rows"><xsl:value-of select="@rows"/></xsl:attribute>
      </xsl:if>
      <xsl:if test="boolean(number(@cols))">
        <xsl:attribute name="cols"><xsl:value-of select="@cols"/></xsl:attribute>
      </xsl:if>
      <xsl:if test="boolean(number(@enabled))">
        <xsl:attribute name="enabled"><xsl:value-of select="@enabled"/></xsl:attribute>
      </xsl:if>
      <xsl:value-of disable-output-escaping="yes" select="."/>
     </xsl:element>
    </xsl:otherwise>
   </xsl:choose>
  </xsl:element>
 </xsl:template>

 <!-- TTextareaField -->
 <xsl:template match="field[@type='tTextareaField']">
  <xsl:element name="li">
   <xsl:choose>
    <xsl:when test="@toplabel = '1'">
     <xsl:call-template name="returnLabel">
      <xsl:with-param name="node" select="." />
     </xsl:call-template>
     <br />
    </xsl:when>
    <xsl:otherwise>
     <xsl:call-template name="returnLabel">
      <xsl:with-param name="node" select="." />
     </xsl:call-template>
     <xsl:if test="@error">
      <xsl:attribute name="class">error</xsl:attribute>
      <xsl:attribute name="title"><xsl:value-of select="@error"/></xsl:attribute>
     </xsl:if>
     <xsl:element name="textarea">
      <xsl:attribute name="id">
       <xsl:choose>
        <xsl:when test="string-length(@id) &gt; 0">
         <xsl:value-of select="@id"/>
        </xsl:when>
        <xsl:otherwise>
         <xsl:value-of select="@name"/>
        </xsl:otherwise>
       </xsl:choose>
      </xsl:attribute>
      <xsl:attribute name="class">
       <xsl:choose>
        <xsl:when test="@toplabel = '1'">
         <xsl:call-template name="buildClass">
          <xsl:with-param name="node" select="." />
          <xsl:with-param name="addClass">fullWidth</xsl:with-param>
         </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
         <xsl:call-template name="buildClass">
          <xsl:with-param name="node" select="." />
         </xsl:call-template>
        </xsl:otherwise>
       </xsl:choose>
      </xsl:attribute>
      <xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
      <xsl:attribute name="style"><xsl:value-of select="@style"/></xsl:attribute>
      <xsl:if test="boolean(number(@rows))">
        <xsl:attribute name="rows"><xsl:value-of select="@rows"/></xsl:attribute>
      </xsl:if>
      <xsl:if test="boolean(number(@cols))">
        <xsl:attribute name="cols"><xsl:value-of select="@cols"/></xsl:attribute>
      </xsl:if>
      <xsl:if test="@readonly = '1'">
       <xsl:attribute name="readonly">readonly</xsl:attribute>
     </xsl:if>
     <xsl:if test="boolean(number(@enabled))">
       <xsl:attribute name="enabled"><xsl:value-of select="@enabled"/></xsl:attribute>
     </xsl:if>
     <xsl:value-of disable-output-escaping="yes" select="."/>
     </xsl:element>
    </xsl:otherwise>
   </xsl:choose>
  </xsl:element>
 </xsl:template>

 <xsl:template name="buildClass">
  <xsl:param name="node" />
  <xsl:param name="addClass" />
  <xsl:if test="@required = '1'">
   <xsl:text>required </xsl:text>
  </xsl:if>
  <xsl:if test="string-length(@class) &gt; 0">
   <xsl:value-of select="@class" />
   <xsl:text> </xsl:text>
  </xsl:if>
  <xsl:if test="string-length($addClass) &gt; 0">
   <xsl:value-of select="$addClass" />
   <xsl:text> </xsl:text>
  </xsl:if>
 </xsl:template>

</xsl:stylesheet>

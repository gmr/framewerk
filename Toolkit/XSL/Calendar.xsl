<?xml version="1.0" encoding="UTF-8"?>
<!-- $Id -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output omit-xml-declaration="yes" method="xml" version="1.0" encoding="UTF-8" indent="yes" />
  
  <!-- TCalendar stylesheet -->
  
  <xsl:template match="calendar">
    <xsl:element name="table">
      <xsl:attribute name="class">
        <xsl:text>calendar calendar-</xsl:text>
        <xsl:value-of select="@view" />
      </xsl:attribute>
      
      <xsl:if test='@title != ""'>
      <tr class="calendar-headers">
        <xsl:choose>
          <xsl:when test="string(/calendar/@view) = 'small'">
            <th class="calendar-day">S</th>
            <th class="calendar-day">M</th>
            <th class="calendar-day">T</th>
            <th class="calendar-day">W</th>
            <th class="calendar-day">T</th>
            <th class="calendar-day">F</th>
            <th class="calendar-day">S</th>
          </xsl:when>
          <xsl:otherwise>
            <th class="calendar-day">Sun</th>
            <th class="calendar-day">Mon</th>
            <th class="calendar-day">Tue</th>
            <th class="calendar-day">Wed</th>
            <th class="calendar-day">Thu</th>
            <th class="calendar-day">Fri</th>
            <th class="calendar-day">Sat</th>
          </xsl:otherwise>
        </xsl:choose>
      </tr>
      </xsl:if>
      <xsl:for-each select="week">
        <tr class="calendar-week">
          <xsl:for-each select="day">
            <xsl:element name="td">
              <xsl:attribute name="class">
                <xsl:text>calendar-day</xsl:text>
                <xsl:if test="@outside">
                  <xsl:text> calendar-outside</xsl:text>
                </xsl:if>
                <xsl:if test="@today">
                  <xsl:text> calendar-today</xsl:text>
                </xsl:if>
              </xsl:attribute>
              <xsl:choose>
                <xsl:when test="boolean(@hidden) and string(/calendar/@display) != 'week'">
                  <xsl:text disable-output-escaping="yes">&amp;nbsp;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="count(events) > 0">
                      <a href="#">
                        <xsl:value-of select="@mday"/>
                        <ul style="display: none;">
                          <xsl:for-each select="events/event">
                            <li title="{@type}"><xsl:value-of select="."/></li>
                          </xsl:for-each>
                        </ul>
                      </a>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="@mday"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:element>
          </xsl:for-each>
        </tr>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>
  
</xsl:stylesheet>

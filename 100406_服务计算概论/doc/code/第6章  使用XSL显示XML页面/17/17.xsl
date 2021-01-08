<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">
  <html>
     <body>
<h1>
        <xmp>
        <xsl:apply-templates select="books/book/*"/>
        </xmp>
</h1>
     </body>
  </html>
</xsl:template>
<xsl:template match="//title">
<xsl:copy><xsl:value-of /></xsl:copy> 
</xsl:template>
<xsl:template match="//author">
<xsl:copy /> 
</xsl:template>
</xsl:stylesheet>
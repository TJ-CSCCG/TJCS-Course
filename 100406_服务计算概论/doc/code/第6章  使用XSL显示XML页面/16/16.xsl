<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/TR/WD-xsl">
<xsl:template match="/">
  <html>
     <body>
        <center>
            <xsl:for-each select="books/book">
               <xsl:apply-templates />
            </xsl:for-each>
        </center>
     </body>
  </html>
</xsl:template>
<xsl:template match="//name">
<h1><xsl:value-of  /></h1>
</xsl:template>
<xsl:template match="//author">
<xsl:value-of  />  著
</xsl:template>
</xsl:stylesheet>